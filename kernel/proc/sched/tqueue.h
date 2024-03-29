#pragma once

#include "../task.h"
#include <stddef.h>

typedef struct {
    task_t* front;
    task_t* back;
    uint64_t len;
} tqueue_t;

task_t* tq_pop_back(tqueue_t* q);
void tq_push_front(tqueue_t* q, task_t* t);
task_t* tq_find(tqueue_t* q, tid_t tid);
void tq_insert_after(tqueue_t* q, task_t* a, task_t* t);
void tq_remove(task_t* t, tqueue_t* q);
