#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "query/query.h"

int main (int argc, char** argv) {
    
    if (argc < 5) {
        fprintf(stderr, "Invalid usage: expected ./%s <username> <password> <client> <clientver>\n", argv[0]);
        return -1;
    }
    
    QueryObject* qobj = query_new(argv[1], argv[2], argv[3], argv[4]);
    query_establish_connection(qobj);
    fprintf(stdout, "Session ID: %s\n", query_refresh_session(qobj));
    query_by_id(qobj, 1, query_int_to_amask(QUERY_FLAG_AID | QUERY_FLAG_RELATED_AID_LIST | QUERY_FLAG_RELATED_AID_TYPE));
    query_free(qobj);
    return 0;
    
}