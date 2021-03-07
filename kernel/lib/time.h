#pragma once

#include <stdint.h>

typedef uint64_t timeval_t;

#define SECONDS_TO_NANOS(x) (x * 1000000000)
#define MILLIS_TO_NANOS(x) (x * 1000000)
#define MICROS_TO_NANOS(x) (x * 1000)
#define NANOS_TO_SECONDS(x) (x / 1000000000)
#define NANOS_TO_MILLIS(x) (x / 1000000)
#define NANOS_TO_MICROS(x) (x / 1000)