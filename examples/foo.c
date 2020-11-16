#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "query/query_util.h"

int main () {
    char thing[] = "adsadsasafaf'adsadsasafaf'adsadsasafaf'adsadsasafaf'adsadsasafaf";
    struct any_list* al = query_any_list_parser(thing, "'");
    printf("%s\n", query_any_list_concat(al, ","));
    return 2;
}