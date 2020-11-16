#define  _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "query/query.h"
#include "common.h"

int main (int argc, char** argv) {
    
    
    if (argc < 5) {
        fprintf(stderr, "Invalid usage: expected ./%s <username> <password> <client> <clientver>\n", argv[0]);
        return -1;
    }
    
    QueryObject* qobj = query_new(argv[1], argv[2], argv[3], argv[4]);
    query_enable_session_cache(qobj, "thing");
    
    query_establish_connection(qobj);
    anidb_response* ares = query_by_id(qobj, 2, query_int_to_amask(QUERY_FLAG_AID | QUERY_FLAG_YEAR | QUERY_FLAG_TYPE | QUERY_FLAG_RELATED_AID_LIST | QUERY_FLAG_RELATED_AID_TYPE | QUERY_FLAG_ROMANJI_NAME | QUERY_FLAG_ENGLISH_NAME | QUERY_FLAG_EPISODES | QUERY_FLAG_RATINGS | QUERY_FLAG_TAG_NAME_LIST));

    (void)ares;
    query_anidb_response_free(ares);
    query_free(qobj);
    
    
    return 0;
    
}