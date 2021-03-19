#include "lock.h"
#include <stdbool.h>

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

void rwlock_end_read(rwlock_t* l)
{
    lock_wait(&(l->nrlock));
    l->num_readers--;
    if (l->num_readers == 0)
        lock_release(&(l->wlock));
    lock_release(&(l->nrlock));
}

bool rwlock_try_write(rwlock_t* l)
{
    return lock_try(&(l->wlock));
}

void rwlock_end_write(rwlock_t* l)
{
    lock_release(&(l->wlock));
}
