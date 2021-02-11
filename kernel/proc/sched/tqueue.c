#include "tqueue.h"

task_t* tq_pop_back(tqueue_t* q)
{
    if (!q->back)
        return NULL;
    task_t* ret = q->back;
    q->back = ret->prev;
    if (!q->back)
        q->front = NULL;
    else
        q->back->next = NULL;
    return ret;
}

void tq_push_front(tqueue_t* q, task_t* t)
{
    t->prev = NULL;
    t->next = q->front;
    if (q->front)
        q->front->prev = t;
    else
        q->back = t;
    q->front = t;
}

task_t* tq_find(tqueue_t* q, tid_t tid)
{
    for (task_t* t = q->front; t; t = t->next)
        if (t->tid == tid)
            return t;
    return NULL;
}