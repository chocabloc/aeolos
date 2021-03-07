#include "task.h"
#include "klog.h"
#include "kmalloc.h"
#include "sched/sched.h"
#include "sys/cpu/cpu.h"
#include <stdbool.h>
#include <stddef.h>

// highest used tid
static tid_t curr_tid = 0;

task_t* task_make(void (*entrypoint)(tid_t), priority_t priority)
{
    // could not allocate a tid
    if (curr_tid == TID_MAX)
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
    ntask->last_tick = 0;
    ntask->status = TASK_READY;
    ntask->wakeuptime = 0;

    curr_tid++;
    return ntask;
}

int task_add(void (*entry)(tid_t), priority_t priority)
{
    task_t* t = task_make(entry, priority);
    if (t) {
        sched_add(t);
        return t->tid;
    }
    return -1;
}