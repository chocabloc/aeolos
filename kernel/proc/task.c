#include "task.h"
#include "kconio.h"
#include "kmalloc.h"
#include "sys/apic/apic.h"
#include "sys/apic/timer.h"
#include "sys/cpu/cpu.h"
#include "sys/panic.h"
#include <stddef.h>

static task_t head_task;
static task_t* curr_task;
static task_t* tail_task;

extern void init_context_switch();
extern void finish_context_switch(task_t* nexttask);

void do_context_switch(task_state_t* currentstate)
{
    curr_task->kstack_top = currentstate;
    curr_task = curr_task->next;
    if (!curr_task)
        curr_task = &head_task;

    apic_send_eoi();
    finish_context_switch(curr_task);
}

void task_create(void* entrypoint)
{
    task_t* ntask = kmalloc(sizeof(task_t));
    void* nstack = kmalloc(4096) + 4096;

    // create the stack frame and update the state to defaults for kernel mode
    task_state_t* ntask_state = nstack - sizeof(task_state_t);
    ntask_state->cs = 0x08;
    ntask_state->ss = 0x10;
    ntask_state->rip = (uint64_t)entrypoint;
    ntask_state->rsp = (uint64_t)nstack;
    ntask_state->rflags = 0x202;

    ntask->kstack_top = ntask_state;
    read_cr("cr3", &(ntask->cr3));
    ntask->next = NULL;

    // append the task
    tail_task->next = ntask;
    tail_task = ntask;
}

void task_init()
{
    // initialize the head task with current state
    asm volatile("mov %%rsp, %0"
                 : "=g"(head_task.kstack_top)
                 :
                 : "memory");
    read_cr("cr3", &head_task.cr3);
    head_task.next = NULL;
    curr_task = &head_task;
    tail_task = &head_task;

    apic_timer_set_frequency(200);
    apic_timer_set_mode(APIC_TIMER_MODE_PERIODIC);
    apic_timer_set_handler(init_context_switch);
    apic_timer_enable();
}