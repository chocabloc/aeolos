#include "tqueue.h"

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

void tq_insert_after(tqueue_t* q, task_t* a, task_t* t)
{
    t->prev = NULL;
    t->next = NULL;
    if (!a) {
        tq_push_front(q, t);
    } else if (a == q->back) {
        a->next = t;
        t->prev = a;
        q->back = t;
    } else {
        task_t* b = a->next;
        b->prev = t;
        a->next = t;
        t->prev = a;
        t->next = b;
    }
}

void tq_remove(task_t* t, tqueue_t* q)
{
    if (t->prev && t->next) {
        t->prev->next = t->next;
        t->next->prev = t->prev;
    } else if (t->prev) {
        t->prev->next = NULL;
        q->back = t->prev;
    } else if (t->next) {
        t->next->prev = NULL;
        q->front = t->next;
    }
    t->next = NULL;
    t->prev = NULL;
}

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

task_t* tq_find(tqueue_t* q, tid_t tid)
{
    for (task_t* t = q->front; t; t = t->next)
        if (t->tid == tid)
            return t;
    return NULL;
}
