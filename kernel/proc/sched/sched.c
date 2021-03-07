#include "sched.h"
#include "../task.h"
#include "kmalloc.h"
#include "lib/klog.h"
#include "lib/time.h"
#include "lock.h"
#include "mm/mm.h"
#include "sys/apic/apic.h"
#include "sys/apic/timer.h"
#include "sys/hpet.h"
#include "sys/smp/smp.h"
#include "tqueue.h"

#define TIMESLICE_DEFAULT MILLIS_TO_NANOS(1)

static spinlock_t sched_lock;

// an idle task for each cpu
static task_t* tasks_idle[CPU_MAX];

// currently running task
static task_t* tasks_running[CPU_MAX];

// tasks grouped by priority
static tqueue_t tasks_bg;
static tqueue_t tasks_min;
static tqueue_t tasks_mid;
static tqueue_t tasks_max;

// sleeping tasks arranged in descending order of their wakeup time
static tqueue_t tasks_asleep;

// temporary space to hold dead tasks, before janitor cleans them
static tqueue_t tasks_dead;

extern void init_context_switch(void* v);
extern void finish_context_switch(task_t* next);

static void idle(tid_t tid __attribute__((unused)))
{
    while (true)
        asm volatile("hlt");
}

// the janitor, runs every second to clean up dead tasks
static void sched_janitor(tid_t tid __attribute__((unused)))
{
    while (true) {
        spinlock_take(&sched_lock);
        task_t* t;
        while ((t = tq_pop_back(&tasks_dead))) {
            klog_info("Deleting task %d, memory left: %d kb\n", t->tid, (pmm_get_mem_info()->free_mem / 1024));
            kmfree(t->kstack_top, KSTACK_SIZE);
            kmfree(t, sizeof(task_t));
        }
        spinlock_release(&sched_lock);
        sched_sleep(SECONDS_TO_NANOS(1));
    }
}

static bool add_task(task_t* t)
{
    switch (t->priority) {
    case PRIORITY_BG:
        tq_push_front(&tasks_bg, t);
        return true;
    case PRIORITY_MIN:
        tq_push_front(&tasks_min, t);
        return true;
    case PRIORITY_MID:
        tq_push_front(&tasks_mid, t);
        return true;
    case PRIORITY_MAX:
        tq_push_front(&tasks_max, t);
        return true;
    default:
        return false;
    }
}

void _do_context_switch(task_state_t* state)
{
    static uint64_t ticks = 0;

    spinlock_take(&sched_lock);
    uint16_t cpu = smp_get_current_info()->cpu_id;

    // save state of current task, if there is one
    task_t* curr = tasks_running[cpu];
    if (curr) {
        curr->kstack_top = state;
        curr->last_tick = ticks;

        // insert it in respective list, if ready
        if (curr->status == TASK_READY)
            add_task(curr);
    }

    // wake up tasks which need to be woken up
    while (tasks_asleep.back && tasks_asleep.back->wakeuptime < hpet_get_nanos())
        add_task(tq_pop_back(&tasks_asleep));

    // next task to run
    task_t* next = NULL;

    // get tasks from all lists
    task_t *tmin = tasks_min.back, *tmid = tasks_mid.back, *tmax = tasks_max.back;

    // if nothing to execute, grab a background task
    if (!tmin && !tmid && !tmax) {
        task_t* tbg = tasks_bg.back;
        if (!tbg) {
            next = tasks_idle[cpu];
        } else {
            next = tq_pop_back(&tasks_bg);
        }
        goto chosen;
    }

    // calculate effective priorities
    uint64_t tminp = tmin ? (tmin->priority + (ticks - tmin->last_tick)) : 0;
    uint64_t tmidp = tmid ? (tmid->priority + (ticks - tmid->last_tick)) : 0;
    uint64_t tmaxp = tmax ? (tmax->priority + (ticks - tmax->last_tick)) : 0;

    // pop task with highest priority
    if (tminp > tmidp) {
        if (tminp > tmaxp)
            next = tq_pop_back(&tasks_min);
        else
            next = tq_pop_back(&tasks_max);
    } else {
        if (tmidp > tmaxp)
            next = tq_pop_back(&tasks_mid);
        else
            next = tq_pop_back(&tasks_max);
    }

chosen : {
    next->status = TASK_READY;
    tasks_running[cpu] = next;
    ticks++;
    apic_send_eoi();
    spinlock_release(&sched_lock);
    finish_context_switch(next);
}
}

void sched_sleep(timeval_t nanos)
{
    spinlock_take(&sched_lock);
    task_t* curr = tasks_running[smp_get_current_info()->cpu_id];
    curr->wakeuptime = hpet_get_nanos() + nanos;
    curr->status = TASK_SLEEPING;

    // add to sleeping tasks list
    if (!tasks_asleep.front) {
        tq_push_front(&tasks_asleep, curr);
    } else if (tasks_asleep.back->wakeuptime > curr->wakeuptime) {
        tq_insert_after(&tasks_asleep, tasks_asleep.back, curr);
    } else {
        for (task_t* i = tasks_asleep.front; i; i = i->next) {
            if (i->wakeuptime <= curr->wakeuptime) {
                tq_insert_after(&tasks_asleep, i->prev, curr);
                break;
            }
        }
    }
    spinlock_release(&sched_lock);

    // wait for scheduler
    asm volatile("hlt");
}

void sched_die()
{
    spinlock_take(&sched_lock);
    task_t* curr = tasks_running[smp_get_current_info()->cpu_id];
    curr->status = TASK_DEAD;
    tq_push_front(&tasks_dead, curr);
    spinlock_release(&sched_lock);

    // wait for scheduler
    asm volatile("hlt");
}

bool sched_add(task_t* t)
{
    spinlock_take(&sched_lock);
    bool ret = add_task(t);
    spinlock_release(&sched_lock);
    return ret;
}

void sched_init(void (*entry)(tid_t))
{
    tasks_idle[smp_get_current_info()->cpu_id] = task_make(idle, PRIORITY_IDLE);

    // scheduler has been started on the bsp
    if (entry) {
        task_add(entry, PRIORITY_MID);
        task_add(sched_janitor, PRIORITY_MIN);
    }

    apic_timer_set_period(TIMESLICE_DEFAULT);
    apic_timer_set_mode(APIC_TIMER_MODE_PERIODIC);
    apic_timer_set_handler(init_context_switch);
    apic_timer_start();
}