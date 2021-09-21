#include "sched.h"
#include "atomic.h"
#include "kmalloc.h"
#include "lib/klog.h"
#include "lib/time.h"
#include "sys/apic/apic.h"
#include "sys/apic/timer.h"
#include "sys/hpet.h"
#include "sys/smp/smp.h"
#include "tqueue.h"

#define TIMESLICE_DEFAULT MILLIS_TO_NANOS(1)

static lock_t sched_lock;

// tasks arranged by tid for fast access
vec_new_static(task_t*, tasks_by_tid);

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

// idle task run when there's nothing to do
_Noreturn static void idle(tid_t tid)
{
    (void)tid;
    while (true)
        asm volatile("hlt");
}

// the janitor, runs every second to clean up dead tasks
// TODO: run it on demand
_Noreturn static void sched_janitor(tid_t tid)
{
    (void)tid;
    while (true) {
        lock_wait(&sched_lock);
        task_t* t;
        while ((t = tq_pop_back(&tasks_dead))) {
            kmfree(t->kstack_limit);
            kmfree(t);
        }
        lock_release(&sched_lock);
        sched_sleep(SECONDS_TO_NANOS(1));
    }
}

// adds to the sleeping tasks list
static void add_sleeping_sorted(task_t* t)
{
    if (!tasks_asleep.front)
        tq_push_front(&tasks_asleep, t);
    else if (tasks_asleep.back->wakeuptime > t->wakeuptime)
        tq_insert_after(&tasks_asleep, tasks_asleep.back, t);
    else {
        for (task_t* i = tasks_asleep.front; i; i = i->next) {
            if (i->wakeuptime <= t->wakeuptime) {
                tq_insert_after(&tasks_asleep, i->prev, t);
                break;
            }
        }
    }
}

// adds task to respective list
static void add_task(task_t* t)
{
    if (t->status == TASK_SLEEPING) {
        add_sleeping_sorted(t);
        return;
    } else if (t->status == TASK_DEAD) {
        tq_push_front(&tasks_dead, t);
        return;
    }

    switch (t->priority) {
    case PRIORITY_BG:
        tq_push_front(&tasks_bg, t);
        return;
    case PRIORITY_MIN:
        tq_push_front(&tasks_min, t);
        return;
    case PRIORITY_MID:
        tq_push_front(&tasks_mid, t);
        return;
    case PRIORITY_MAX:
        tq_push_front(&tasks_max, t);
        return;
    }
}

// perform a context switch
void _do_context_switch(task_state_t* state)
{
    static uint64_t ticks = 0;

    lock_wait(&sched_lock);
    uint16_t cpu = smp_get_current_info()->cpu_id;

    // save state of current task, if there is one
    task_t* curr = tasks_running[cpu];
    if (curr) {
        curr->kstack_top = state;
        curr->last_tick = ticks;

        // if the task was running, set it to ready
        if (curr->status == TASK_RUNNING)
            curr->status = TASK_READY;
        add_task(curr);
    }

    // wake up tasks which need to be woken up
    while (tasks_asleep.back && tasks_asleep.back->wakeuptime < hpet_get_nanos()) {
        task_t* t = tq_pop_back(&tasks_asleep);
        t->status = TASK_READY;
        add_task(t);
    }

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
    next->status = TASK_RUNNING;
    tasks_running[cpu] = next;

    // set the rsp0 in tss
    smp_get_current_info()->tss.rsp0 = (uint64_t)(next->kstack_limit + KSTACK_SIZE);
    ticks++;
    lock_release(&sched_lock);
    apic_send_eoi();
    apic_timer_set_period(TIMESLICE_DEFAULT);
    finish_context_switch(next);
}
}

void sched_sleep(timeval_t nanos)
{
    // if sleep time is too little, busy sleep
    if (nanos <= TIMESLICE_DEFAULT) {
        hpet_nanosleep(nanos);
        return;
    }

    lock_wait(&sched_lock);
    task_t* curr = tasks_running[smp_get_current_info()->cpu_id];
    curr->wakeuptime = hpet_get_nanos() + nanos;
    curr->status = TASK_SLEEPING;
    lock_release(&sched_lock);
    asm volatile("hlt");
}

// kill task with specified tid
int64_t sched_kill(tid_t tid)
{
    lock_wait(&sched_lock);

    // check if tid is valid
    if (tid >= tasks_by_tid.len || tasks_by_tid.data[tid] == NULL) {
        klog_err("there's no task with tid %d\n", tid);
        lock_release(&sched_lock);
        return -1;
    }

    // mark the task as dead, it will be cleaned up later
    task_t* t = tasks_by_tid.data[tid];
    t->status = TASK_DEAD;
    tasks_by_tid.data[tid] = NULL;

    // was it the currently running task?
    bool is_current = (t == tasks_running[smp_get_current_info()->cpu_id]);

    lock_release(&sched_lock);

    // we can't return if it was the current one
    if (is_current)
        asm volatile("hlt");
    return 0;
}

// adds a task to the scheduler
void sched_add(task_t* t)
{
    lock_wait(&sched_lock);
    add_task(t);
    vec_resize(&tasks_by_tid, t->tid + 1);
    tasks_by_tid.data[t->tid] = t;
    lock_release(&sched_lock);
}

// get pointer to currently running task on the current cpu
task_t* sched_get_current()
{
    return tasks_running[smp_get_current_info()->cpu_id];
}

// initialize the scheduler
void sched_init(void (*entry)(tid_t))
{
    uint16_t cpu_id = smp_get_current_info()->cpu_id;
    tasks_idle[cpu_id] = task_make(idle, PRIORITY_IDLE, TASK_KERNEL_MODE, NULL, 0);

    // scheduler has been started on the bsp
    if (entry) {
        task_add(entry, PRIORITY_MID, TASK_KERNEL_MODE, NULL, 0);
        task_add(sched_janitor, PRIORITY_MIN, TASK_KERNEL_MODE, NULL, 0);
        klog_ok("started on bsp\n");
    }

    apic_timer_set_period(TIMESLICE_DEFAULT);
    apic_timer_set_mode(APIC_TIMER_MODE_ONESHOT);
    apic_timer_set_handler(init_context_switch);
    apic_timer_start();
}
