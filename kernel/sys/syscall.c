#include "syscall.h"
#include "sys/panic.h"

void syscall_dispatcher(int index, uint64_t param1, uint64_t param2)
{
    kernel_panic("syscall %d not implemented\n", index);
}
