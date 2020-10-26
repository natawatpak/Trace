#ifndef __CLEANUP_H
#define __CLEANUP_H


#define defer(func) __attribute__((cleanup(func)))  

#endif