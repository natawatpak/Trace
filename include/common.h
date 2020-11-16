#ifndef __COMMON_H
#define __COMMON_H

// glibc's from libc-symbols.h
#define weak_alias(name, aliasname) _weak_alias(name, aliasname)
#define _weak_alias(name, aliasname)      \
    extern __typeof__(name) aliasname     \
    __attribute__((weak, alias(#name)))   \
    __attribute_copy__(name)                                      

#define DEFINE_TRIVIAL_LINKED_LIST(name, dt, dtname) \
    struct name {          \
        dt dtname;         \
        struct name* next; \
    };

#define attribute_hidden __attribute__((visibility("hidden")))
#define unused           __attribute__((unused))

#endif