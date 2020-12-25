#include "task.h"
#include "kconio.h"
#include "kmalloc.h"
#include "sys/apic/apic.h"
#include "sys/apic/timer.h"
#include "sys/cpu/cpu.h"
#include "sys/panic.h"
#include <stdbool.h>
#include <stddef.h>

static task_t head_task;
static task_t* curr_task;
static task_t* tail_task;

static uint64_t curr_tid = 1;

extern void init_context_switch();
extern void finish_context_switch(task_t* nexttask);

// self modifying code to trigger an apic timer interrupt
static void reschedule()
{
    asm volatile("movq $interrupt, %%rax;"
                 "movb %0, %%bl;"
                 "movb %%bl, 1(%%rax);"
                 "interrupt: int $0"
                 :
                 : "g"(apic_timer_get_vector())
                 : "bl", "rax");
}

void _do_context_switch(task_state_t* currentstate)
{
    // save state of current task
    curr_task->kstack_top = currentstate;

    // get next task. if we have reached the end, rewind
    curr_task = curr_task->next;
    if (!curr_task)
        curr_task = &head_task;

    apic_send_eoi();

    // switch to next task
    finish_context_switch(curr_task);
}

int task_create(void* entrypoint)
{
    if (curr_tid > TID_MAX)
        return -1;

    // allocate memory for the new task and its stack
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
    ntask->tid = curr_tid;
    curr_tid++;
    ntask->next = NULL;

    // append the task
    tail_task->next = ntask;
    tail_task = ntask;

    return ntask->tid;
}

bool task_destroy(uint64_t tid)
{
    task_t* t = &head_task;
    while (t->next) {
        if (t->next->tid == tid) {
            task_t* todelete = t->next;
            if (t->next->next)
                t->next = t->next->next;
            else {
                t->next = NULL;
                tail_task = t;
            }
            kmfree(todelete->kstack_top - 1, 4096);
            kmfree(todelete, sizeof(task_t));
            return true;
        }
        t = t->next;
    }

    kdbg_warn("task_destroy(): Task with tid %d not found\n", tid);
    return false;
}

void task_yield()
{
    reschedule();
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
    head_task.tid = 0;

    curr_task = &head_task;
    tail_task = &head_task;

    apic_timer_set_frequency(200);
    apic_timer_set_mode(APIC_TIMER_MODE_PERIODIC);
    apic_timer_set_handler(init_context_switch);
    apic_timer_enable();
}
