#pragma once

#include "../task.h"
#include "lib/time.h"

#define TIMESLICE_DEFAULT \
    (timeval_t) { .s = 0, .ms = 1, .us = 0, .ns = 0 }

bool sched_add(task_t* task);
void sched_init();
