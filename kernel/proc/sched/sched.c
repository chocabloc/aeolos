#include "sched.h"
#include "../task.h"
#include "lock.h"
#include "sys/apic/apic.h"
#include "sys/apic/timer.h"
#include "sys/smp/smp.h"
#include "tqueue.h"

static spinlock_t sched_lock;

static task_t* tasks_idle[CPU_MAX];
static task_t* tasks_running[CPU_MAX] __attribute__((unused));

static tqueue_t tasks_bg;
static tqueue_t tasks_min;
static tqueue_t tasks_mid;
static tqueue_t tasks_max;

extern void init_context_switch();
extern void finish_context_switch(task_t* next);

static void idle(tid_t tid __attribute__((unused)))
{
    while (true)
        asm volatile("hlt");
}

bool sched_add(task_t* t)
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
    // save state of current task, if there is one
    task_t* curr = tasks_running[cpu];
    if (curr) {
        curr->kstack_top = state;
        curr->last_tick = ticks;

        // insert it in respective list
        sched_add(curr);
    }

    // jump to next task
    tasks_running[cpu] = next;
    ticks++;
    apic_send_eoi();
    spinlock_release(&sched_lock);
    finish_context_switch(next);
}
}

void sched_init(void (*entry)(tid_t))
{
    tasks_idle[smp_get_current_info()->cpu_id] = task_make(idle, PRIORITY_IDLE);

    if (entry)
        task_add(entry, PRIORITY_MID);

    apic_timer_set_frequency(500);
    apic_timer_set_mode(APIC_TIMER_MODE_PERIODIC);
    apic_timer_set_handler(init_context_switch);
    apic_timer_enable();
}