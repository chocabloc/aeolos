#pragma once

#include "../task.h"
#include "lib/time.h"

bool sched_add(task_t* task);
void sched_init();
void sched_sleep(timeval_t nanos);
void sched_die();
