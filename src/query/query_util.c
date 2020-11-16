#include <stdlib.h>
#include <string.h>

#include "query/query.h"
#include "query/query_util.h"

struct any_list* query_any_list_parser(char* r, char* delim) {
    struct any_list* head = calloc(1, sizeof *head);
    struct any_list* plist = head->next = head;

    char* endptr;
    char* token;
    for ( ; ; r = NULL) {
        token = strtok_r(r, delim, &endptr);
        if (token == NULL) {
            free(plist->next);
            plist->next = NULL;
            break;
        }
        plist = plist->next;
        plist->elem = calloc(1, strlen(token)+1);
        strcpy(plist->elem, token);
        plist->next = calloc(1, sizeof *plist->next);
    }

    return head;
}

char* query_any_list_concat(struct any_list* any_list, char* delim) {
    unsigned n = 32;
    unsigned c = 0;
    unsigned t = 0;
    unsigned d = delim?strlen(delim):0;

    char* buffer = calloc(n, sizeof *buffer);
    while (any_list) {
        while ((t=strlen(any_list->elem)) > n-c-d) {
            buffer = realloc(buffer, n *= 2);
        } 
        c += (t + d);
        strcat(buffer, any_list->elem);
        strcat(buffer, delim); 
        any_list = any_list->next;
    }

    buffer[c-1] = '\0';

    return buffer;
}

void query_any_list_free(struct any_list* any_list) {
    struct any_list* _any_list;
    while (any_list) {
        _any_list = any_list;
        any_list = any_list->next;
        
        free(_any_list->elem);
        free(_any_list);
    }
}