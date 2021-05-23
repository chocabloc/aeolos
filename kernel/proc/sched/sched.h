#pragma once

#include "../task.h"
#include "lib/time.h"

void sched_add(task_t* task);
void sched_init(void (*entry)(tid_t));
void sched_sleep(timeval_t nanos);
void sched_die();
task_t* sched_get_current();
