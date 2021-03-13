// XORShift random number generator

#include "random.h"

static uint64_t rand_state;

void random_seed(uint64_t seed)
{
    rand_state = seed;
}

uint64_t random()
{
    rand_state ^= rand_state << 13;
    rand_state ^= rand_state >> 17;
    rand_state ^= rand_state << 5;
    return rand_state;
}
