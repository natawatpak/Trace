#ifndef __FRAMES_H
#define __FRAMES_H

/*
    This OOP approach is heavily inspired by libioP.h
*/

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <stddef.h>

#define FRAME_NSAVE 0x1
#define FRAME_BULKSAVE 0x2
#define FRAME_NOSAVE 0x3



struct __frame_extract_opt {
    uint64_t bulksave:1;
    uint64_t nosave:1;
    char*    format;
    int      nframes_requested;
};

typedef struct __frame_frameobject {
    AVFormatContext* fCtx;
    AVCodecContext*  cCtx;
    AVCodec*         codec;
    AVFrame*         frame_buffer;
#ifdef _FRAME_NO_USE_GLOBAL_BUFFER
#define err_buf_len 128
    char             err_buf[err_buf_len];
#endif
}__frame_frameobject;


#define _FRAME_MEMBER_TYPE(type, member) __typeof__(((type){}).member)
#define _FRAME_REINTERPRET_ACCESS(self, type, member) (*(_FRAME_MEMBER_TYPE(type, member)*)((char*)self + offsetof(type, member)))

#define JUMP_FIELD(type, name) type name
#define _FRAME_JUMPS(self) _FRAME_REINTERPRET_ACCESS(self, struct __frame_frameobject_plus, vtable)

#define JUMP0(self, method)       ((_FRAME_JUMPS(self))->method(self))
#define JUMPN(self, method, ...)  ((_FRAME_JUMPS(self)->method)(self, __VA_ARGS__))

typedef int (*__frame_extract)(struct __frame_frameobject*, enum AVPixelFormat, struct __frame_extract_opt);
#define _FRAME_EXTRACT(self, pixfmt, frameopt) JUMPN(self, __extract, pixfmt, frameopt)

typedef int (*__frame_skip)(struct __frame_frameobject*, int);
#define _FRAME_SKIP(self, n) JUMPN(self, __skip, n)

typedef int (*__deallocate)(struct __frame_frameobject*);
#define _FRAME_DEALLOC(self) JUMP0(self, __deallocate)


typedef struct __internal_frame_jump_t {
    JUMP_FIELD(__frame_extract,   __extract);
    JUMP_FIELD(__frame_skip,      __skip);
    JUMP_FIELD(__deallocate,      __deallocate);
}__internal_frame_jump_t;



struct __frame_frameobject_plus {
    __frame_frameobject      frameobject;
    __internal_frame_jump_t* vtable;
};

typedef __frame_frameobject frameobject;

struct __frame_frameobject*                                frame_open(char* file);
int                                frame_extract(struct __frame_frameobject* fFile, enum AVPixelFormat, ...);

#endif