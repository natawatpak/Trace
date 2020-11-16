#ifndef __QUERY_UTIL_H
#define __QUERY_UTIL_H

#include "query/query.h"

struct any_list {
    /* disabled uint32_t refcount; */
    char* elem;
    struct any_list* next;
};

#define query_any_list_add_ref(any_list) ({ any_list->refcount++;   any_list; })
#define query_any_list_rem_ref(any_list) ({ any_list->refcount--;   any_list; })
#define query_any_list_cls_ref(any_list) ({ any_list->refcount = 0; any_list; })

char* query_any_list_concat(struct any_list* any_list, char* delim);
struct any_list* query_any_list_parser(char* r, char* delim);
void query_any_list_free(struct any_list* al);

#endif