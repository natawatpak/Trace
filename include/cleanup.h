#ifndef __CLEANUP_H
#define __CLEANUP_H

#define DEFINE_TRIVIAL_CLEANUP_FUNC(type, func) \
    static inline func##p (type* p) {           \
        if (*p)                                 \
            func(*p);                           \
}

#define defer(func) __attribute__((cleanup(func)))  

#endif