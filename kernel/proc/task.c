#include "task.h"
#include "kmalloc.h"
#include "sched/sched.h"
#include "sys/cpu/cpu.h"
#include <stddef.h>

// highest used tid
static tid_t curr_tid = 0;

task_t* task_make(void (*entry)(tid_t), priority_t priority, tmode_t mode, void* rsp, uint64_t pagemap)
{
    // could not allocate a tid
    if (curr_tid == TID_MAX) {
        klog_warn("could not allocate tid\n");
        return NULL;
    }

    // allocate memory for the new task and its stack
    task_t* ntask = kmalloc(sizeof(task_t));
    ntask->kstack_limit = kmalloc(KSTACK_SIZE);
    ntask->kstack_top = ntask->kstack_limit + KSTACK_SIZE;

    // create the stack frame and update the state to defaults
    task_state_t* ntask_state = ntask->kstack_top - sizeof(task_state_t);
    if (mode == TASK_KERNEL_MODE) {
        ntask_state->cs = KMODE_CS;
        ntask_state->ss = KMODE_SS;
    } else {
        ntask_state->cs = UMODE_CS;
        ntask_state->ss = UMODE_SS;
    }
    ntask_state->rflags = RFLAGS_DEFAULT;
    ntask_state->rip = (uint64_t)entry;
    ntask_state->rsp = rsp ? (uint64_t)rsp : (uint64_t)ntask->kstack_top;
    ntask_state->rdi = curr_tid; // pass the tid to the task

    // initialize the task
    if (pagemap)
        ntask->cr3 = pagemap;
    else
        read_cr("cr3", &(ntask->cr3));

    ntask->kstack_top = ntask_state;
    ntask->tid = curr_tid;
    ntask->priority = priority;
    ntask->last_tick = 0;
    ntask->status = TASK_READY;
    ntask->wakeuptime = 0;
    vector_init(ntask->openfiles);

    curr_tid++;
    return ntask;
}

int task_add(void (*entry)(tid_t), priority_t priority, tmode_t mode, void* rsp, uint64_t pagemap)
{
    task_t* t = task_make(entry, priority, mode, rsp, pagemap);
    if (t) {
        sched_add(t);
        return t->tid;
    }
    return -1;
}
