#include "task.h"
#include "klog.h"
#include "kmalloc.h"
#include "lock.h"
#include "sys/apic/apic.h"
#include "sys/apic/timer.h"
#include "sys/cpu/cpu.h"
#include "sys/panic.h"
#include <stdbool.h>
#include <stddef.h>

spinlock_t sched_lock = 0;

// currently running task
static task_t* curr_task = NULL;

// currently highest used tid, 0 is reserved
static tid_t curr_tid = 1;

extern void init_context_switch();
extern void finish_context_switch(task_t* nexttask);

// trigger an apic timer interrupt
static void reschedule()
{
    asm volatile("movq 1f, %%rax;"
                 "movb %0, %%bl;"
                 "movb %%bl, 1(%%rax);"
                 "1: int $0"
                 :
                 : "g"(apic_timer_get_vector())
                 : "bl", "rax");
}

static task_t* alloc_task(void (*entrypoint)(tid_t), priority_t priority)
{
    // could not allocate a tid or invalid priority
    if (!curr_tid || priority > PRIORITY_MAX)
        return NULL;

    // allocate memory for the new task and its stack
    task_t* ntask = kmalloc(sizeof(task_t));
    void* nstack = kmalloc(KSTACK_SIZE) + KSTACK_SIZE;

    // create the stack frame and update the state to defaults for kernel mode
    task_state_t* ntask_state = nstack - sizeof(task_state_t);
    ntask_state->cs = KMODE_CS;
    ntask_state->ss = KMODE_SS;
    ntask_state->rip = (uint64_t)entrypoint;
    ntask_state->rsp = (uint64_t)nstack;
    ntask_state->rflags = KMODE_RFLAGS;
    ntask_state->rdi = curr_tid; // pass the tid to the task

    // initialize the task
    ntask->kstack_top = ntask_state;
    read_cr("cr3", &(ntask->cr3));
    ntask->tid = curr_tid;
    ntask->priority = priority;

    curr_tid++;
    return ntask;
}

void _do_context_switch(task_state_t* currentstate)
{
    static bool firstrun = true;

    if (!spinlock_try_take(&sched_lock))
        return;

    if (!curr_task) {
        klog_warn("No runnable task. Halting...\n");
        while (true)
            ;
    }

    if (!firstrun)
        curr_task->kstack_top = currentstate;
    else
        firstrun = false;

    // get next task
    curr_task = curr_task->next;

    // send an EOI, so that we continue to receive timer interrupts
    apic_send_eoi();
    spinlock_release(&sched_lock);

    // switch to next task
    finish_context_switch(curr_task);
}

tid_t task_create(void (*entrypoint)(tid_t), priority_t priority)
{
    spinlock_take(&sched_lock);

    task_t* ntask = alloc_task(entrypoint, priority);
    if (!ntask) {
        spinlock_release(&sched_lock);
        return -1;
    }

    // append to task list
    ntask->next = curr_task->next;
    curr_task->next->prev = ntask;
    ntask->prev = curr_task;
    curr_task->next = ntask;

    spinlock_release(&sched_lock);
    return ntask->tid;
}

bool task_destroy(uint64_t tid)
{
    spinlock_take(&sched_lock);

    int ret = -1; // have we returned to the current task while looping
    for (task_t* i = curr_task; i; i = i->next) {
        ret += (i == curr_task);
        if (ret)
            break;

        if (i->tid == tid) {
            // cant delete current task
            if (i == curr_task) {
                klog_warn("task_destroy(): cannot destroy current task\n");
                spinlock_release(&sched_lock);
                return false;
            }

            // remove from list
            i->prev->next = i->next;
            i->next->prev = i->prev;

            // free memory occupied by task
            kmfree(i->kstack_top - 1, KSTACK_SIZE);
            kmfree(i, sizeof(task_t));

            spinlock_release(&sched_lock);
            return true;
        }
    }

    spinlock_release(&sched_lock);
    klog_warn("task_destroy(): invalid tid %d\n", tid);
    return false;
}

void task_yield()
{
    reschedule();
}

// initialize multitasking and set entry point of first task
void task_init(void (*entrypoint)(tid_t))
{
    // create the initial task
    curr_task = alloc_task(entrypoint, PRIORITY_MID);
    curr_task->next = curr_task;
    curr_task->prev = curr_task;

    apic_timer_set_frequency(500);
    apic_timer_set_mode(APIC_TIMER_MODE_PERIODIC);
    apic_timer_set_handler(init_context_switch);
    apic_timer_enable();
}
