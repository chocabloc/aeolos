#include "klog.h"
#include <kmalloc.h>
#include <stdint.h>

#define vector_struct(type) \
    struct {                \
        size_t len;         \
        size_t alloc_size;  \
        type* data;         \
    }

#define vector_new_static(type, name) \
    static vector_struct(type) name = { 0 }

#define vector_push_back(vec, elem)                                  \
    {                                                                \
        (vec)->len++;                                                \
        if ((vec)->alloc_size < (vec)->len * sizeof(elem)) {         \
            (vec)->alloc_size = ((vec)->len * sizeof(elem)) * 2;     \
            (vec)->data = kmrealloc((vec)->data, (vec)->alloc_size); \
        }                                                            \
        (vec)->data[(vec)->len - 1] = elem;                          \
    }

#define vector_init(v)    \
    {                     \
        v.len = 0;        \
        v.alloc_size = 0; \
        v.data = NULL;    \
    }
