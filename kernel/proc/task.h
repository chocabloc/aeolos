#pragma once

#include "fs/vfs.h"
#include "time.h"
#include "vector.h"
#include <stdbool.h>
#include <stdint.h>

#define KMODE_CS 0x08
#define KMODE_SS 0x10
#define UMODE_CS 0x1b
#define UMODE_SS 0x23
#define RFLAGS_DEFAULT 0x0202

#define KSTACK_SIZE 4096

typedef uint16_t tid_t;
#define TID_MAX UINT16_MAX

typedef uint8_t priority_t;
#define PRIORITY_IDLE 0
#define PRIORITY_BG 5
#define PRIORITY_MIN 10
#define PRIORITY_MID 35
#define PRIORITY_MAX 70

typedef enum {
    TASK_KERNEL_MODE,
    TASK_USER_MODE
} tmode_t;

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_SLEEPING,
    TASK_DEAD
} tstatus_t;

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
    void* kstack_top; // kernel stack top
    uint64_t cr3; // virtual address space

    tid_t tid; // task id
    priority_t priority; // task priority
    uint64_t last_tick; // last tick at which task ran
    tstatus_t status; // current status of task
    timeval_t wakeuptime; // time at which task should wake up
    tmode_t mode; // kernel mode or usermode
    void* kstack_limit; // kernel stack limit
    vector_struct(vfs_node_desc_t*) openfiles; //  stores open files

    struct task_t* next;
    struct task_t* prev;
} task_t;

task_t* task_make(void (*entrypoint)(tid_t), priority_t priority, tmode_t mode, void* rsp, uint64_t pagemap);
int task_add(void (*entry)(tid_t), priority_t priority, tmode_t mode, void* rsp, uint64_t pagemap);
