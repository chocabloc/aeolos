#include "task.h"
#include "kconio.h"
#include "kmalloc.h"
#include "sys/apic/apic.h"
#include "sys/apic/timer.h"
#include "sys/cpu/cpu.h"
#include "sys/panic.h"
#include <stdbool.h>
#include <stddef.h>

// task lists for high, medium, and low priority tasks
static tasklist_t tlists[3] = { { .head = NULL, .current = NULL, .tail = NULL } };

// currently running task
static task_t* curr_task;

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

void _do_context_switch(task_state_t* currentstate)
{
    static uint64_t nticks = 4;

    tasklist_t* l; // list from which to take next task

    /* a very simple scheduling algorithm
     * will go into an infinite loop if there are no tasks to execute
     */
    while (true) {
        if (nticks % 6 == 0)
            l = &tlists[PRIORITY_MIN];
        else if (nticks % 2 == 0)
            l = &tlists[PRIORITY_MAX];
        else
            l = &tlists[PRIORITY_MID];

        nticks++;
        if (l->current)
            break;
    }

    // save state of current task, if there is one
    if (curr_task)
        curr_task->kstack_top = currentstate;

    // get next task. if we have reached the end, rewind
    l->current = l->current->next;
    if (!l->current)
        l->current = l->head;
    curr_task = l->current;

    // send an EOI, so that we continue to receive timer interrupts
    apic_send_eoi();

    // switch to next task
    finish_context_switch(curr_task);
}

tid_t task_create(void (*entrypoint)(tid_t), priority_t priority)
{
    // could not allocate a tid or invalid priority
    if (!curr_tid || priority > PRIORITY_MAX)
        return -1;

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
    ntask->next = NULL;
    ntask->prev = NULL;

    // append to respective task list
    if (tlists[priority].tail) {
        tlists[priority].tail->next = ntask;
        ntask->prev = tlists[priority].tail;
    } else {
        // list is empty, initialize it
        tlists[priority].head = ntask;
        tlists[priority].current = ntask;
    }
    tlists[priority].tail = ntask;

    curr_tid++;
    return ntask->tid;
}

bool task_destroy(uint64_t tid)
{
    // since tid 0 is reserved
    if (!tid)
        goto failed;

    // is the task to be deleted the currently running task
    bool isrunning = (curr_task->tid == tid);

    // loop through all 3 lists
    for (priority_t prt = PRIORITY_MIN; prt <= PRIORITY_MAX; prt++) {
        task_t* t = tlists[prt].head;
        while (t) {
            // found the task, now remove it from the list
            if (t->tid == tid) {
                // is the task to be deleted the current task in the list
                bool iscurrentinlist = (t->tid == tlists[prt].current->tid);

                if (t->next && t->prev) {
                    // task is in the middle of the list
                    t->prev->next = t->next;
                } else if (t->prev) {
                    // task is at the end of the list
                    t->prev->next = NULL;
                    tlists[prt].tail = t->prev;
                } else if (t->next) {
                    // task is at the beginning of the list
                    t->next->prev = NULL;
                    tlists[prt].head = t->next;
                } else {
                    // task is the only element in the list
                    tlists[prt].head = NULL;
                    tlists[prt].tail = NULL;
                }

                // free memory used by it
                kmfree(t->kstack_top - 1, KSTACK_SIZE);
                kmfree(t, sizeof(task_t));

                // if it is the current task in the list, set current task to null
                if (iscurrentinlist) {
                    tlists[prt].current = NULL;

                    /* if the task to be deleted is the currently running task, we can't return.
                     * so go into an infinite loop
                     */
                    if (isrunning) {
                        curr_task = NULL;
                        while (true)
                            ;
                    }
                }

                return true;
            }
            t = t->next;
        };
    }

failed:
    kdbg_warn("task_destroy(): Invalid tid %d\n", tid);
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
    task_create(entrypoint, PRIORITY_MIN);

    apic_timer_set_frequency(500);
    apic_timer_set_mode(APIC_TIMER_MODE_PERIODIC);
    apic_timer_set_handler(init_context_switch);
    apic_timer_enable();
}
