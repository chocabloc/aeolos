#pragma once

#include <stdbool.h>
#include <stdint.h>

#define KMODE_CS 0x08
#define KMODE_SS 0x10
#define KMODE_RFLAGS 0x202

#define KSTACK_SIZE 4096
#define TID_MAX UINT16_MAX

typedef uint16_t tid_t;

typedef enum {
    PRIORITY_MIN,
    PRIORITY_MID,
    PRIORITY_MAX
} priority_t;

typedef struct {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} task_state_t;

typedef struct task_t {
    void* kstack_top; // top of kernel stack
    uint64_t cr3; // virtual address space
    tid_t tid; // task id
    int priority;
    struct task_t* next;
    struct task_t* prev;
} task_t;

// doubly linked list of tasks
typedef struct
{
    task_t* head;
    task_t* current;
    task_t* tail;
} tasklist_t;

void task_init();
tid_t task_create(void (*entrypoint)(tid_t), priority_t priority);
bool task_destroy(uint64_t tid);
void task_yield();
