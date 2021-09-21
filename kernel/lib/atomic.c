#include "atomic.h"
#include <stdbool.h>

// take an r/w lock for reading
void rwlock_take_read(rwlock_t* l)
{
    while (!rwlock_try_read(l)) {
    }
}

// try to take an r/w lock for reading
bool rwlock_try_read(rwlock_t* l)
{
    lock_wait(&(l->nrlock));
    l->num_readers++;
    if (l->num_readers == 1) {
        if (!lock_try(&(l->wlock))) {
            l->num_readers--;
            lock_release(&(l->nrlock));
            return false;
        };
    }
    lock_release(&(l->nrlock));
    return true;
}

// end read on an r/w lock
void rwlock_end_read(rwlock_t* l)
{
    lock_wait(&(l->nrlock));
    l->num_readers--;
    if (l->num_readers == 0)
        lock_release(&(l->wlock));
    lock_release(&(l->nrlock));
}

// try to take an r/w lock for writing
bool rwlock_try_write(rwlock_t* l)
{
    return lock_try(&(l->wlock));
}

// end writing on an r/w lock
void rwlock_end_write(rwlock_t* l)
{
    lock_release(&(l->wlock));
}
