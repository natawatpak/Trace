#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wchar.h>
#include <time.h>

#include "query/query.h"
#include "query/query_util.h"
#include "query/query_codes.h"
#include "config.h"

#define PARSER static inline
#define unused __attribute__((unused))

DEFINE_TRIVIAL_LINKED_LIST(fields, char*, field);

struct response {
    int code;
    char* retstring;
    struct fields* fields;
};

PARSER struct response query_response_parser(const char* r) {
    struct response response = {0};
    struct fields*  fields   = response.fields = malloc(sizeof *fields);
    char* endptr;
    char* pfield;

    response.code = strtol(r, &endptr, 10);
    if (endptr == r) {
        fprintf(stderr, "query_response_parser: fatal: malformed response \"%s\"\n", r);
        exit(EXIT_FAILURE); 
    }
    
    response.retstring = calloc(1, (pfield = strchr(endptr+1, '\n')) - endptr);
    memcpy(response.retstring, endptr+1, pfield-endptr);
    response.retstring[pfield - endptr - 1] = '\0';

    pfield++;
    while((endptr = strchr(pfield, '|'))) {
        fields->field = calloc(1, endptr - pfield);
        fields->next  = calloc(1, sizeof *fields->next);
        memcpy(fields->field, pfield, endptr - pfield); 
        fields = fields->next;
        pfield = endptr+1;
    }

    fields->field = calloc(1, strlen(pfield));
    memcpy(fields->field, pfield, strlen(pfield)); 
    fields->next = NULL;

    return response;
}

PARSER void query_response_free(struct response* r) {
    free(r->retstring);
    struct fields* fp, *fnext = r->fields;
    while (fnext) {
        fp = fnext;
        fnext = fnext->next;
        free(fp->field);
        free(fp);
    }
    
}



PARSER uint64_t query_year_parser(const char* r unused) {
    return 0;
}

QueryObject*    query_new(const char* username, const char* password, const char* client, const char* clientver) {
    QueryObject* qobj = calloc(1, sizeof *qobj);
    query_init(qobj, username, password, client, clientver);
    return qobj; 
}


QueryObject*    query_init(QueryObject* qobj, const char* username, const char* password, const char* client, const char* clientver) {
    qobj->_username  = username;
    qobj->_password  = password;
    qobj->_client    = client;
    qobj->_clientver = clientver;
    return qobj;
}
QueryObject*    query_establish_connection(QueryObject* qobj) {
    struct addrinfo hint = {0}, *results, *result;

    hint.ai_family   = AF_UNSPEC;
    hint.ai_socktype = SOCK_DGRAM;
    hint.ai_flags    = 0;
    hint.ai_protocol = 0;

    qobj->_r = getaddrinfo(ANIDB_ENDPOINT, ANIDB_PORT, &hint, &results);
    if (qobj->_r != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(qobj->_r));
        return NULL;
    }

    for (result = results ; result != NULL ; result = result->ai_next) {
        qobj->_sfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (qobj->_sfd == -1) {
            continue;
        }
        if (connect(qobj->_sfd, result->ai_addr, result->ai_addrlen) != -1) {
            break;
        }
        close(qobj->_sfd);
    }
    freeaddrinfo(results);

    if (result == NULL) {
        fprintf(stderr, "query_establish_connection: unable to connect\n");
        return NULL;
    }
    return qobj;
} 

static inline const char*    _query_refresh_session(QueryObject* qobj) {
    int n;
    n = snprintf(qobj->_buffer, ANIDB_NREAD, ANIDB_AUTHFMT, qobj->_username, qobj->_password, qobj->_clientver, qobj->_client);
    write(qobj->_sfd, qobj->_buffer, n);
    n = read(qobj->_sfd, qobj->_buffer, ANIDB_NREAD);
    qobj->_buffer[n] = '\0';
    
    struct response r = query_response_parser(qobj->_buffer);  
    switch (r.code) {
        case ANIDB_LOGIN_ACCEPTED :
            break;
        case ANIDB_LOGIN_ACCEPTED_NEW_VER_AVL :
            fprintf(stderr, "query_refresh_session: login accepted, new version available\n");
            break;
        case ANIDB_LOGIN_FAILED :
            fprintf(stderr, "query_refresh_session: fatal: login failed\n");
            exit(EXIT_FAILURE);
        case ANIDB_BANNED :
            fprintf(stderr, "query_refresh_session: fatal: banned\n");
            exit(EXIT_FAILURE);
        default :
            fprintf(stderr, "query_refresh_session: fatal: unexpected response code %d\n", r.code);
            exit(EXIT_FAILURE);
    }

    memcpy(qobj->_session, r.retstring, ANIDB_NSESSION);
    qobj->_session[ANIDB_NSESSION] = '\0';

    query_response_free(&r); 
    return qobj->_session;
}

const char*     query_refresh_session(QueryObject* qobj) {
    if (*qobj->session_loc) {
        struct stat st;

        time_t t = time(NULL);

        if ((stat(qobj->session_loc, &st) < 0) || (((st.st_ctime - t) / 60) > 35)) {
            unlink(qobj->session_loc);
            _query_refresh_session(qobj);
            query_save_session(qobj);
            return qobj->_session;
        }

        int fd = open(qobj->session_loc, O_RDONLY);
        read(fd, qobj->_session, ANIDB_NSESSION + 1);
        close(fd);
        return qobj->_session;
    }

    return _query_refresh_session(qobj);
}

void query_enable_session_cache(QueryObject* qobj, const char* location) {
    strncpy(qobj->session_loc, location, MAX_SESSION_LEN);  
}

void query_save_session(QueryObject* qobj) {
    int fd = open(qobj->session_loc, O_WRONLY | O_TRUNC | O_CREAT, 0777);
    write(fd, qobj->_session, ANIDB_NSESSION + 1);
    close(fd);
}



anidb_response  query_by_name(QueryObject* qobj, const char* aname, const char* amask) {
    int n;
retry :
    n = snprintf(qobj->_buffer, ANIDB_NREAD, ANIDB_ANAMEFMT, qobj->_session, aname, amask); 
    write(qobj->_sfd, qobj->_buffer, n);
    n = read(qobj->_sfd, qobj->_buffer, ANIDB_NREAD);
    qobj->_buffer[n] = '\0';
    struct response r = query_response_parser(qobj->_buffer);
    if (r.code == ANIDB_LOGIN_FIRST) {
        query_refresh_session(qobj);
        goto retry;
    }

    return (anidb_response){0};
}

anidb_response* query_by_id(QueryObject* qobj, int aid, const char* amask) {
    int n;
retry:
    n = snprintf(qobj->_buffer, ANIDB_NREAD, ANIDB_ANIIDFMT, qobj->_session, aid, amask); 
    printf("%s\n", qobj->_buffer);
    write(qobj->_sfd, qobj->_buffer, n);
    n = read(qobj->_sfd, qobj->_buffer, ANIDB_NREAD);
    qobj->_buffer[n] = '\0';
    struct response r = query_response_parser(qobj->_buffer);
    if (r.code == ANIDB_LOGIN_FIRST) {
        query_response_free(&r);
        _query_refresh_session(qobj);
        query_save_session(qobj);
        goto retry;
    }

    if (r.code == ANIDB_INVALID_SESSION) {
        query_refresh_session(qobj);
        goto retry;
    }

    if (r.code == ANIDB_NO_SUCH_ANIME) {
        fprintf(stderr, "No such anime found\n");
        query_response_free(&r);
    }

    if (r.code == ANIDB_BANNED) {
        fprintf(stderr, "query_by_id: fatal: banned \"%s\"\n", r.retstring);
        exit(EXIT_FAILURE);
    }

    if (r.code != ANIDB_ANIME) {
        fprintf(stderr, "query_by_id: fatal: unknown return code, received %d\n", r.code);
        exit(EXIT_FAILURE);
    }

#define ISSET(mask,bit) (((mask) & (bit)) == (bit))
#define NEXT(l) ((l) = (l)->next)
    printf("%s\n", qobj->_buffer);    
    anidb_response* ani_res = calloc(sizeof *ani_res, 1);
    struct fields*  fields  = r.fields;

    memcpy(qobj->_buffer, "0x", 2);
    strcpy(qobj->_buffer+2, amask); 
    uint64_t iamask = strtol(qobj->_buffer, NULL, 16);

    if (ISSET(iamask, QUERY_FLAG_AID)) {
        ani_res->aid = strtol(fields->field, NULL, 10); 
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_DATEFLAGS)) {
        ani_res->dateflags = strtol(fields->field, NULL, 10);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_YEAR)) {
        struct any_list* al = query_any_list_parser(fields->field, "-");
        ani_res->year = (struct ryear){.from = strtol(al->elem, NULL, 10), .to=strtol(al->next->elem, NULL, 10)};
        query_any_list_free(al);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_TYPE)) {
        ani_res->type = calloc(strlen(fields->field)+1, 1);
        strcpy(ani_res->type, fields->field);
        NEXT(fields);  
    }
    if (ISSET(iamask, QUERY_FLAG_RELATED_AID_LIST)) {
        ani_res->related_aid_list = (struct id_list*)query_any_list_parser(fields->field, "\'");
        NEXT(fields);
    } 
    if (ISSET(iamask, QUERY_FLAG_RELATED_AID_TYPE)) {
        ani_res->related_aid_type = (struct id_list*)query_any_list_parser(fields->field, "\'");
        NEXT(fields);
    }

    if (ISSET(iamask, QUERY_FLAG_ROMANJI_NAME)) {
        ani_res->romanji_name = calloc(strlen(fields->field) + 1, 1);
        strcpy(ani_res->romanji_name, fields->field);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_KANJI_NAME)) {
        ani_res->kanji_name = calloc(wcslen((wchar_t*)fields->field) + 1, sizeof (wchar_t));
        wcscpy(ani_res->kanji_name, (wchar_t*)fields->field);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_ENGLISH_NAME)) {
        ani_res->english_name = calloc(strlen(fields->field) + 1, 1);
        strcpy(ani_res->english_name, fields->field);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_SHORT_NAME_LIST)) {
        ani_res->short_name_list = (struct name_list*)query_any_list_parser(fields->field, "\'");
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_SYN_NAME_LIST)) {
        ani_res->syn_name_list = (struct name_list*)query_any_list_parser(fields->field, "\'");
        NEXT(fields);
    }

    if (ISSET(iamask, QUERY_FLAG_EPISODES)) {
        ani_res->episodes = strtol(fields->field, NULL, 10);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_HIGHEST_EP_NUM)) {
        ani_res->highest_episode_number = strtol(fields->field, NULL, 10);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_SPECIAL_EP_CNT)) {
        ani_res->special_ep_count = strtol(fields->field, NULL, 10);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_AIRDATE)) {
        ani_res->air_date = strtol(fields->field, NULL, 10);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_ENDDATE)) {
        ani_res->end_date = strtol(fields->field, NULL, 10);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_URL)) {
        ani_res->url = calloc(strlen(fields->field) + 1, 1);
        strcpy(ani_res->url, fields->field);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_PICNAME)) {
        ani_res->pic_name = calloc(strlen(fields->field) + 1, 1);
        strcpy(ani_res->pic_name, fields->field);
        NEXT(fields);
    }

    if (ISSET(iamask, QUERY_FLAG_RATINGS)) {
        ani_res->ratings = strtol(fields->field, NULL, 10);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_VOTE_CNT)) {
        ani_res->vote_count = strtol(fields->field, NULL, 10);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_TMP_RATING)) {
        ani_res->temp_rating = strtol(fields->field, NULL, 10);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_AVG_REVIEW_RATING)) {
        ani_res->average_review_rating = strtol(fields->field, NULL, 10);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_REVIEW_CNT)) {
        ani_res->review_count = strtol(fields->field, NULL, 10);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_AWARD_LIST)) {
        ani_res->awards = (struct award_list*)query_any_list_parser(fields->field, "\'");
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_IS_NSFW)) {
        ani_res->is_nsfw = strtol(fields->field, NULL, 10);
        NEXT(fields);
    }

    if (ISSET(iamask, QUERY_FLAG_ANN_ID)) {
        ani_res->ANNid = strtol(fields->field, NULL, 10);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_ALL_CINEMA_ID)) {
        (void)ani_res->allcinema_id;
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_ANIME_NFO)) {
        
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_TAG_NAME_LIST)) {
        ani_res->tags = (struct tags*)query_any_list_parser(fields->field, ",");
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_TAG_ID_LIST)) {
        ani_res->tag_ids = (struct id_list*)query_any_list_parser(fields->field, ",");
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_TAG_WEIGHT_LIST)) {
        ani_res->tag_weight_list = (struct weight_list*)query_any_list_parser(fields->field, ",");
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_DATE_RECORD_UPDATED)) {
        ani_res->date_record_updated = strtol(fields->field, NULL, 10);
        NEXT(fields);
    }

    if (ISSET(iamask, QUERY_FLAG_CHARACTER_ID_LIST)) {
        ani_res->character_id_list = (struct id_list*)query_any_list_parser(fields->field, ",");
        NEXT(fields);
    }

    if (ISSET(iamask, QUERY_FLAG_SPECIALS_CNT)) {
        ani_res->specials_count = strtol(fields->field, NULL, 10);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_CREDITS_CNT)) {
        ani_res->credits_count = strtol(fields->field, NULL, 10);;
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_OTHER_CNT)) {
        ani_res->others_count = strtol(fields->field, NULL, 10);
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_TRAILER_CNT)) {
        ani_res->trailers_count = strtol(fields->field, NULL, 10);;
        NEXT(fields);
    }
    if (ISSET(iamask, QUERY_FLAG_PARODY_CNT)) {
        ani_res->parodies_count = strtol(fields->field, NULL, 10);
        NEXT(fields);
    }
    

    query_response_free(&r);
    return ani_res;
}

void query_anidb_response_free(anidb_response* ani_res) {
    query_any_list_free((struct any_list*)ani_res->related_aid_list);
    query_any_list_free((struct any_list*)ani_res->related_aid_type);
    free(ani_res->romanji_name);
    free(ani_res->english_name);
    free(ani_res->other_name);
    free(ani_res->kanji_name);
    query_any_list_free((struct any_list*)ani_res->short_name_list);
    query_any_list_free((struct any_list*)ani_res->syn_name_list);
    free(ani_res->url);
    free(ani_res->pic_name);
    query_any_list_free((struct any_list*)ani_res->awards);
    query_any_list_free((struct any_list*)ani_res->tags);
    query_any_list_free((struct any_list*)ani_res->tag_ids);
    query_any_list_free((struct any_list*)ani_res->tag_weight_list);
    query_any_list_free((struct any_list*)ani_res->character_id_list);
    free(ani_res);
}

void            query_free(QueryObject* qobj) {
    close(qobj->_sfd);
    free(qobj);
}

const char*     query_int_to_amask(uint64_t iamask) {
    static char amask[64];
    snprintf(amask, 64, "%014lx", iamask);
    return amask;
}