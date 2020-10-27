#ifndef __FRAMES_H
#define __FRAMES_H

/*
    This OOP approach is heavily inspired by libioP.h
*/

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <stddef.h>

#define FRAME_ENDARG   0x0
#define FRAME_NSAVE    0x1
#define FRAME_BULKSAVE 0x2
#define FRAME_NOSAVE   0x3
#define FRAME_NOBUFFER 0x4

typedef struct __frame_frameobject {
    AVFormatContext    *_fCtx;
    AVCodecContext     *_cCtx;
    AVCodec            *_codec;
    AVFrame            *_frame_buffer, *_frame, *_frameRGB;
    AVRational         _frame_rate;
    struct SwsContext  *_img_convert_ctx;
    int                _video_index;
    int                _errnum;
    int                _nbytes;
    char               *_underlying_buf;
#ifndef USE_RAME_GLOBAL_ERROR_BUFFER
#define error_buf_len 128
    char               _err_buf[error_buf_len];
#endif
}__frame_frameobject;


#define _FRAME_MEMBER_TYPE(type, member) __typeof__(((type){}).member)
#define _FRAME_REINTERPRET_ACCESS(self, type, member) (*(_FRAME_MEMBER_TYPE(type, member)*)((char*)self + offsetof(type, member)))

#define JUMP_FIELD(type, name) type name
#define _FRAME_JUMPS(self) _FRAME_REINTERPRET_ACCESS(self, struct __frame_frameobject_plus, vtable)

#define JUMP0(self, method)       ((_FRAME_JUMPS(self))->method(self))
#define JUMPN(self, method, ...)  ((_FRAME_JUMPS(self)->method)(self, __VA_ARGS__))

typedef int (*__frame_extract)(struct __frame_frameobject*, enum AVPixelFormat, ...);
#define _FRAME_EXTRACT(self, pixfmt, frameopt) JUMPN(self, __extract, pixfmt, frameopt)

typedef size_t (*__frame_skip)(struct __frame_frameobject*, size_t);
#define _FRAME_SKIP(self, n) JUMPN(self, __skip, n)

typedef int (*__frame_error)(struct __frame_frameobject*);
#define _FRAME_ERROR(self) JUMP0(self, __error) 

typedef int (*__frame_close)(struct __frame_frameobject*);
#define _FRAME_DEALLOC(self) JUMP0(self, __close)

typedef struct __internal_frame_jump_t {
    JUMP_FIELD(__frame_extract,     __extract);
    JUMP_FIELD(__frame_skip,        __skip);
    JUMP_FIELD(__frame_error,       __error);
    JUMP_FIELD(__frame_close,  __close);
}__internal_frame_jump_t;



struct __frame_frameobject_plus {
    __frame_frameobject      frameobject;
    __internal_frame_jump_t* vtable;
};

typedef struct __frame_frameobject frameobject;

extern frameobject*                frame_open(char* __filename) ;
extern int                         frame_extract(frameobject* __frameobject, enum AVPixelFormat __pixfmt, ...);
extern size_t                      frame_skip(frameobject* __frameobject, size_t __nframes);
extern int                         frame_error(frameobject* __frameobject);
extern int                         frame_close(frameobject* __frameobject);

#endif