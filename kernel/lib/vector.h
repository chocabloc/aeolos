#include "klog.h"
#include "memutils.h"
#include <kmalloc.h>
#include <stdint.h>

#define vec_struct(type)    \
    struct {                \
        size_t len;         \
        size_t alloc_size;  \
        type* data;         \
    }

#define vec_new(type, name) vec_struct(type) name = { 0 }

#define vec_new_static(type, name) static vec_new(type, name)

#define vec_push_back(vec, elem)                                     \
    {                                                                \
        (vec)->len++;                                                \
        if ((vec)->alloc_size < (vec)->len * sizeof(elem)) {         \
            (vec)->alloc_size = ((vec)->len * sizeof(elem)) * 2;     \
            (vec)->data = kmrealloc((vec)->data, (vec)->alloc_size); \
        }                                                            \
        (vec)->data[(vec)->len - 1] = elem;                          \
    }

#define vec_at(vec, index) (vec)->data[index]

#define vec_erase(vec, index)                                                 \
    {                                                                         \
        memcpy(&((vec)->data[index + 1]), &((vec)->data[index]),              \
               sizeof((vec)->data[0]) * (vec)->len - index - 1);              \
        (vec)->len--;                                                         \
    }

#define vec_erase_val(vec, val)                        \
    {                                                  \
        for(size_t __i = 0; __i < (vec)->len; __i++) { \
            if (vec_at(vec, __i) == (val)) {           \
                vec_erase(vec, __i);                   \
                break;                                 \
            }                                          \
        }                                              \
    }

#define vec_init(v)       \
    {                     \
        v.len = 0;        \
        v.alloc_size = 0; \
        v.data = NULL;    \
    }
