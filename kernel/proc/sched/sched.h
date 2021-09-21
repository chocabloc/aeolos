#pragma once

#include "../task.h"
#include "lib/time.h"

void sched_add(task_t* task);
void sched_init(void (*entry)(tid_t));
void sched_sleep(timeval_t nanos);
int64_t sched_kill(tid_t tid);
task_t* sched_get_current();
