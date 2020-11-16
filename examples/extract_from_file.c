#include "video/frames.h"

#include "query/query.h"
#include "query/query_util.h"

#include <sys/stat.h>
#include <getopt.h>

static int last_found(char* path, int c) {
    int index = -1;
    for (int i = 0 ; path[i] ; ++i) if (path[i] == c)index = i;
    return index;
}

int main (int argc, char** argv) { 

    int option_index;
    int c;

    char buffer[256];

    char* username unused = NULL;
    char* password unused = NULL;

    char* infile  = NULL;
    char* outfile = NULL;
    int   nframes = 0;

    char* aid                 = NULL;
    QueryObject* qobj         = NULL;
    anidb_response* ani_res   = NULL;
    struct any_list* cred_al  = NULL;

    struct option long_options[] = {
        {"help",    no_argument,       0, 'h'},
        {"in",      required_argument, 0, 'i'},
        {"out",     required_argument, 0, 'o'},
        {"nframes", required_argument, 0, 'n'},
        {"aid",     required_argument, 0, 'a'},
        {"creds",   required_argument, 0, 'c'},
    };
    
    while ((c=getopt_long(argc, argv, "hi:o:n:c:a:", long_options, &option_index)) != -1) {
        switch(c) {
            case 'h' :
                printf("""Usage: %s [-h]\
                          \n        [-a|--aid aid]\
                          \n        [-n frames]\
                          \n        [-o|--out outfmt]\
                          \n        [-i|--in] infile\
                          \n        [-c|--creds username:password]\
                          \nExtract frames from a media file\
                        """, argv[0]);
                exit(0);
            case 'i' :
                infile = optarg;
                break;
            case 'o' :
                outfile = optarg;
                break;
            case 'n' :
                nframes = strtol(optarg, NULL, 10);
                break;
            case 'a' :
                aid = optarg;
                break;
            case 'c' :
                cred_al = query_any_list_parser(optarg, ":");
                username = cred_al->elem;
                password = cred_al->next->elem;
                break;
            case '?' :
                printf("?");
                infile = optarg;
                break;
            default :
                abort();
        }
    }
    if (infile == NULL) {
        if (argv[optind] == NULL) {
            fprintf(stderr, "%s: fatal: infile not supplied\n", argv[0]);
            exit(1);
        }
        infile = argv[optind];
    }

    frameobject* fobject = frame_open(infile);
    
    if (nframes == 0) {
        nframes = 60;
    }

    char* btmp = infile + strlen(infile) - 1;
    if (outfile == NULL) {
        outfile = malloc(128);
        struct stat st;
        if (stat("Frames", &st) == -1) {
            mkdir("Frames", S_IRWXU);
        }
        while (btmp != infile && *btmp != '/')btmp--;
        snprintf(outfile, 128, "Frames%s@f%%d-%%d.png", btmp);
    }

    if (aid != NULL) {
        if (username == NULL || password == NULL) {
            fprintf(stderr, "%s: fatal: aid supplied without username or password\n", argv[0]);
            exit(1); 
        }
        qobj = query_new(username, password, "trace", "4");
        query_establish_connection(qobj);
        query_refresh_session(qobj);
        ani_res = query_by_id(qobj, strtol(aid, NULL, 10), query_int_to_amask(QUERY_FLAG_AID | QUERY_FLAG_YEAR | QUERY_FLAG_TYPE | QUERY_FLAG_RELATED_AID_LIST |\
                                                                              QUERY_FLAG_RELATED_AID_TYPE | QUERY_FLAG_ROMANJI_NAME | QUERY_FLAG_ENGLISH_NAME  |\
                                                                              QUERY_FLAG_EPISODES | QUERY_FLAG_RATINGS | QUERY_FLAG_TAG_NAME_LIST));

        int i = last_found(outfile, '/');
        if (i != -1)outfile[i]='\0';
        snprintf(buffer, 256, "%s/metadata.json", outfile, btmp);
        FILE* json = fopen(buffer, "w");
        if (i != -1)outfile[i]='/';
        fprintf(json, "{\
                       \n\t\"QUERY_FLAG_AID\":\"%d\",\
                       \n\t\"QUERY_FLAG_YEAR\":\"%d-%d\",\
                       \n\t\"QUERY_FLAG_TYPE\":\"%s\",\
                       \n\t\"QUERY_FLAG_RELATED_AID_LIST\":\"%s\",\
                       \n\t\"QUERY_FLAG_RELATED_AID_TYPE\":\"%s\",\
                       \n\t\"QUERY_FLAG_ROMANJI_NAME\":\"%s\",\
                       \n\t\"QUERY_FLAG_ENGLISH_NAME\":\"%s\",\
                       \n\t\"QUERY_FLAG_EPISODES\":\"%d\",\
                       \n\t\"QUERY_FLAG_RATINGS\":\"%d\",\
                       \n\t\"QUERY_FLAG_TAG_NAME_LIST\":\"%s\"\
                       }",
                       ani_res->aid, ani_res->year.from, ani_res->year.to,
                       ani_res->type, query_any_list_concat((struct any_list*)ani_res->related_aid_list, ","), query_any_list_concat((struct any_list*)ani_res->related_aid_type, ","),
                       ani_res->romanji_name, ani_res->english_name, 
                       ani_res->episodes, ani_res->ratings, query_any_list_concat((struct any_list*)ani_res->tags, ","));
        fclose(json);
    }

    if (qobj != NULL) {
        query_free(qobj);
        query_anidb_response_free(ani_res);
    }
    
    frame_extract(fobject, AV_PIX_FMT_YUV420P, AV_CODEC_ID_MJPEG, outfile, FRAME_SEPSAVE, FRAME_NSAVE, nframes, FRAME_ENDARG);
    query_any_list_free(cred_al);

    exit(0);
}

