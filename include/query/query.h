#ifndef __QUERY_H
#define __QUERY_H

#include <stdint.h>
#include <stddef.h>
#include "common.h"
#include "config.h"
#include "query_flags.h"

/*
    byte 1 refers to the MSB(yte)
*/


enum Relations {
    SEQUEL              = 1,
    PREQUEL             = 2 ,
    SAME_SETTINGS       = 11,
    ALTERNATIVE_SETTING = 12,
    ALTERNATIVE_VERSION = 32,
    MUSIC_VIDEO         = 41,
    CHARACTER           = 42,
    SIDE_STORY          = 51,
    PARENT_STORY        = 52,
    SUMMARY             = 61,
    FULL_STORY          = 62,
    OTHER               = 100
};

DEFINE_TRIVIAL_LINKED_LIST(id_list, char*, id);
DEFINE_TRIVIAL_LINKED_LIST(tags, char*, tag);
DEFINE_TRIVIAL_LINKED_LIST(name_list, char*, name);
DEFINE_TRIVIAL_LINKED_LIST(award_list, char*, award);
DEFINE_TRIVIAL_LINKED_LIST(weight_list, char*, weight);

struct ryear {
    uint16_t from; 
    uint16_t to;
}; 

typedef struct __internal_anidb_response {
    uint16_t       aid;
    uint16_t       dateflags;
    struct ryear   year;
    char*          type;
    struct id_list* related_aid_list;
    struct id_list* related_aid_type;

    char*      romanji_name;
    wchar_t*    kanji_name;
    char*       english_name;
    wchar_t*    other_name;
    struct name_list* short_name_list;
    struct name_list* syn_name_list;

    uint16_t    episodes;
    uint16_t    highest_episode_number;
    uint16_t    special_ep_count;
    uint64_t    air_date;
    uint64_t    end_date;
    char* url;
    char* pic_name;

    uint16_t    ratings;
    uint16_t    vote_count;
    uint16_t    temp_rating;
    uint16_t    temp_vote_count;
    uint16_t    average_review_rating;
    uint16_t    review_count;
    struct award_list* awards;
    uint16_t    is_nsfw:1;

    uint16_t    ANNid;
    uint16_t    allcinema_id;
    uint16_t    animeNFOid;
    struct tags* tags;
    struct id_list* tag_ids;
    struct weight_list* tag_weight_list;
    uint16_t    date_record_updated;

    struct id_list* character_id_list;

    uint16_t    specials_count;
    uint16_t    credits_count;
    uint16_t    others_count;
    uint16_t    trailers_count;
    uint16_t    parodies_count;
}anidb_response;

typedef struct __internal_req_field {
    const char     *_username, *_password, *_client, *_clientver;
    char           _session[ANIDB_NSESSION + 1];
    char           _buffer[ANIDB_NREAD];
    int            _sfd, _r;
}QueryObject;

QueryObject*    query_new(const char* username, const char* password, const char* client, const char* clientver); /* avoid calling query_init directly */
QueryObject*    query_init(QueryObject* qobj, const char* username, const char* password, const char* client, const char* clientver);
QueryObject*    query_establish_connection(QueryObject* qobj);
const char*     query_refresh_session(QueryObject* qobj);
anidb_response  query_by_name(QueryObject* qobj, const char* aname, const char* amask);
anidb_response  query_by_id(QueryObject* qobj, int aid, const char* amask);
void            query_free(QueryObject* qobj);
void            query_anidb_response_free(anidb_response* ani_res);

const char*     query_int_to_amask(uint64_t iamask);

#endif