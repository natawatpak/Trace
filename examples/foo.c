#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct any_list {
    struct any_list* next;
    char* elem;
};


static inline struct any_list* query_any_list_parser(char* r, char* delim) {
    struct any_list* head = calloc(1, sizeof *head);
    struct any_list* plist = head;

    char* endptr;
    char* token;
    for ( ; ; r = NULL) {
        token = strtok_r(r, delim, &endptr);
        if (token == NULL) {
            free(plist->next);
            plist->next = NULL;
            break;
        }
        plist->elem = calloc(1, strlen(token)+1);
        strcpy(plist->elem, token);
        plist = plist->next = calloc(1, sizeof *plist->next);
    }

    return head;
}

int main () {
    char thing[] = "1999-1999";
    struct any_list* al = query_any_list_parser(thing, "-");
    (void)al;
    return 2;
}