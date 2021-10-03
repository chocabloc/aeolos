#include "sched.h"
#include "../task.h"
#include "atomic.h"
#include "lib/time.h"
#include "sys/apic/apic.h"
#include "sys/apic/timer.h"
#include "sys/hpet.h"
#include "sys/smp/smp.h"
#include "tqueue.h"
#include "vector.h"

#define TIMESLICE_DEFAULT MILLIS_TO_NANOS(2)
#define IS_TID_VALID(tid) ((tid) < tasks_all.len && tasks_all.data[(tid)] != NULL)

static lock_t sched_lock;

// an idle task for each cpu
static task_t* tasks_idle[CPU_MAX];

// currently running tasks on each cpu
static task_t* tasks_running[CPU_MAX];

// list of all tasks
vec_new_static(task_t*, tasks_all);

// asleep and dead tasks respectively
static tqueue_t tasks_asleep, tasks_dead;

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
            tasks_all.data[t->tid] = NULL;
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

// perform a context switch
void _do_context_switch(task_state_t* state)
{
    lock_wait(&sched_lock);
    uint16_t cpu = smp_get_current_info()->cpu_id;

    // save state of current task
    task_t* curr = tasks_running[cpu];
    curr->kstack_top = state;
    if (curr->status == TASK_RUNNING)
        curr->status = TASK_READY;

    // wake up tasks which need to be woken up
    while (tasks_asleep.back && tasks_asleep.back->wakeuptime < hpet_get_nanos()) {
        task_t* t = tq_pop_back(&tasks_asleep);
        t->is_sleeping = false;
    }

    // choose next task
    task_t* next = NULL;
    for (tid_t i = curr->tid + 1; i < tasks_all.len; i++) {
        task_t* t = tasks_all.data[i];
        if (t != NULL && t->status == TASK_READY && !(t->is_sleeping)) {
            next = t;
            goto task_chosen;
        }
    }
    for (tid_t i = 0; i <= curr->tid; i++) {
        task_t* t = tasks_all.data[i];
        if (t != NULL && t->status == TASK_READY && !(t->is_sleeping)) {
            next = t;
            goto task_chosen;
        }
    }

task_chosen:
    // could not find a runnable task, so idle
    if (!next)
        next = tasks_idle[cpu];

    // we've chosen next task
    next->status = TASK_RUNNING;
    tasks_running[cpu] = next;

    // set rsp0 in the tss
    smp_get_current_info()->tss.rsp0 = (uint64_t)(next->kstack_limit + KSTACK_SIZE);
    lock_release(&sched_lock);
    apic_send_eoi();
    apic_timer_set_period(TIMESLICE_DEFAULT);
    finish_context_switch(next);
}

// sleep current task for specified number of nanoseconds
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
    curr->is_sleeping = true;
    add_sleeping_sorted(curr);

    lock_release(&sched_lock);
    asm volatile("hlt");
}

// kill task with specified tid
int64_t sched_kill(tid_t tid)
{
    lock_wait(&sched_lock);

    // check if tid is valid
    if (!IS_TID_VALID(tid)) {
        klog_err("there's no task with tid %d\n", tid);
        lock_release(&sched_lock);
        return -1;
    }

    // mark the task as dead, it will be cleaned up later
    task_t* t = tasks_all.data[tid];
    t->status = TASK_DEAD;
    tasks_all.data[tid] = NULL;
    tq_push_front(&tasks_dead, t);

    // if it was sleeping, remove it from the sleeping list
    if (t->is_sleeping) {
        tq_remove(t, &tasks_asleep);
        t->is_sleeping = false;
    }

    // was it the currently running task?
    bool is_current = (t == tasks_running[smp_get_current_info()->cpu_id]);

    lock_release(&sched_lock);

    // we can't return if it was the current one
    if (is_current)
        asm volatile("hlt");
    return 0;
}

// block specified task indefinitely
int64_t sched_block(tid_t tid)
{
    lock_wait(&sched_lock);

    // check if tid is valid
    if (!IS_TID_VALID(tid)) {
        klog_err("there's no task with tid %d\n", tid);
        lock_release(&sched_lock);
        return -1;
    }

    // mark the task as blocked
    task_t* t = tasks_all.data[tid];
    if (t->status == TASK_BLOCKED) {
        klog_err("task %d is already blocked\n", tid);
        lock_release(&sched_lock);
        return -1;
    }
    t->status = TASK_BLOCKED;

    // was it the currently running task?
    bool is_current = (t == tasks_running[smp_get_current_info()->cpu_id]);

    lock_release(&sched_lock);

    // we can't return right now if it was the current one
    if (is_current)
        asm volatile("hlt");
    return 0;
}

// unblock specified task
int64_t sched_unblock(tid_t tid)
{
    lock_wait(&sched_lock);

    // check if tid is valid
    if (!IS_TID_VALID(tid)) {
        klog_err("there's no task with tid %d\n", tid);
        lock_release(&sched_lock);
        return -1;
    }

    // mark the task as ready
    task_t* t = tasks_all.data[tid];
    if (t->status != TASK_BLOCKED) {
        klog_err("task %d is already unblocked\n", tid);
        lock_release(&sched_lock);
        return -1;
    }
    t->status = TASK_READY;

    return 0;
}

// adds a task to the scheduler
void sched_add(task_t* t)
{
    lock_wait(&sched_lock);
    vec_resize(&tasks_all, t->tid + 1);
    tasks_all.data[t->tid] = t;
    lock_release(&sched_lock);
}

// get currently running task data
task_t* sched_get_current()
{
    return tasks_running[smp_get_current_info()->cpu_id];
}

// initialize the scheduler
void sched_init(void (*first_task)(tid_t))
{
    uint16_t cpu_id = smp_get_current_info()->cpu_id;

    /*
        create idle task for current core and set it to running
        idle tasks are removed from task list to prevent the
        scheduler from attempting to schedule them
    */
    tid_t tid_idle = task_add(idle, 0, TASK_KERNEL_MODE, NULL, 0);
    tasks_idle[cpu_id] = tasks_all.data[tid_idle];
    tasks_running[cpu_id] = tasks_all.data[tid_idle];
    tasks_all.data[tid_idle] = NULL;

    // scheduler has been started on the bsp, do some initialization
    if (first_task) {
        task_add(first_task, PRIORITY_MID, TASK_KERNEL_MODE, NULL, 0);
        task_add(sched_janitor, PRIORITY_MIN, TASK_KERNEL_MODE, NULL, 0);
        klog_ok("started on bsp\n");
    }

    // configure and start the lapic timer
    apic_timer_set_period(TIMESLICE_DEFAULT);
    apic_timer_set_mode(APIC_TIMER_MODE_ONESHOT);
    apic_timer_set_handler(init_context_switch);
    apic_timer_start();
}
