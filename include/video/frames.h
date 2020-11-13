#ifndef __FRAMES_H
#define __FRAMES_H

/*
    This OOP approach is heavily inspired by libioP.h
*/

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <stddef.h>

#include "common.h"

#define FRAME_ENDARG   0x0
#define FRAME_NSAVE    0x1
#define FRAME_BULKSAVE 0x2
#define FRAME_SEPSAVE  0x3
#define FRAME_SKIP     0x4 

typedef struct __frame_frameobject {
    AVFormatContext    *_fCtx;
    AVCodecContext     *_cCtx;
    AVCodec            *_codec;
    AVFrame            *_frame_buffer, *_frame, *_frameRGB;
    AVRational         _frame_rate;
    struct SwsContext  *_img_convert_ctx;
    uint8_t**          *_oldData;
    int                _video_index;
    int                _errnum;
    int                _nbytes;
    char               *_underlying_buf;
    int                _cframe;
#ifndef USE_FRAME_GLOBAL_ERROR_BUFFER
#define error_buf_len 128
    char               _err_buf[error_buf_len];
#endif
}__frame_frameobject;

struct __frame_extract_opt {
    uint64_t bulksave:1;
    uint64_t sepsave:1; 
    uint64_t skip:1;
    char*    format;
    int      nframes_requested;
};

#ifdef  FRAME_SILENT_AVERROR
#define FRAME_fprintf(...)
#endif

#ifdef USE_FRAME_GLOBAL_ERROR_BUFFER
#define error_buf_len 128
static char error_buf[error_buf_len];
#else
#define error_buf self->_err_buf
#endif

#define _FRAME_MEMBER_TYPE(type, member) __typeof__(((type){}).member)
#define _FRAME_REINTERPRET_ACCESS(self, type, member) (*(_FRAME_MEMBER_TYPE(type, member)*)((char*)self + offsetof(type, member)))

#define JUMP_FIELD(type, name) type name
#define _FRAME_JUMPS(self) _FRAME_REINTERPRET_ACCESS(self, struct __frame_frameobject_plus, vtable)

#define JUMP0(self, method)       ((_FRAME_JUMPS(self))->method(self))
#define JUMPN(self, method, ...)  ((_FRAME_JUMPS(self)->method)(self, __VA_ARGS__))

typedef int (*__frame_extract)(struct __frame_frameobject*, enum AVPixelFormat, enum AVCodecID, char*, ...);
#define _FRAME_EXTRACT(self, pixfmt, dstid, format, ...) JUMPN(self, __extract, pixfmt, dstid, format, __VA_ARGS__)

typedef int (*__frame_optextract)(struct __frame_frameobject*, enum AVPixelFormat, enum AVCodecID, char*, struct __frame_extract_opt);
#define _FRAME_OPTEXTRACT(self, pixfmt, dstid, format, ...) JUMPN(self, __optextract, pixfmt, dstid, format, __VA_ARGS__)

typedef int (*__frame_vextract)(struct __frame_frameobject*, enum AVPixelFormat, enum AVCodecID, char*, va_list);
#define _FRAME_VEXTRACT(self, pixfmt, dstid, format, ...) JUMPN(self, __vextract, pixfmt, dstid, format, __VA_ARGS__)

typedef size_t (*__frame_skip)(struct __frame_frameobject*, size_t);
#define _FRAME_SKIP(self, n) JUMPN(self, __skip, n)

typedef int (*__frame_error)(struct __frame_frameobject*);
#define _FRAME_ERROR(self) JUMP0(self, __error) 

typedef int (*__frame_close)(struct __frame_frameobject*);
#define _FRAME_DEALLOC(self) JUMP0(self, __close)

typedef struct __internal_frame_jump_t {
    JUMP_FIELD(__frame_extract,     __extract);
    JUMP_FIELD(__frame_vextract,    __vextract);
    JUMP_FIELD(__frame_optextract,  __optextract);
    JUMP_FIELD(__frame_skip,        __skip);
    JUMP_FIELD(__frame_error,       __error);
    JUMP_FIELD(__frame_close,       __close);
}__internal_frame_jump_t;



struct __frame_frameobject_plus {
    __frame_frameobject      frameobject;
    __internal_frame_jump_t* vtable;
};

typedef struct __frame_frameobject frameobject;

extern frameobject*                frame_open(char* __filename) ;
extern int                         frame_save(AVCodecContext*, AVFrame*, enum AVCodecID, const char*);
extern int                         frame_extract(frameobject* __frameobject, enum AVPixelFormat __pixfmt, enum AVCodecID __dstfmtid, char* __dstfmt, ...);
extern int                         frame_optextract(frameobject* __frameobject, enum AVPixelFormat __pixfmt, enum AVCodecID __dstfmtid, char* __dstfmt, struct __frame_extract_opt __opt);
extern int                         frame_vextract(frameobject* __frameobject, enum AVPixelFormat __pixfmt, enum AVCodecID __dstfmtid, char* __dstfmt, va_list);
extern size_t                      frame_skip(frameobject* __frameobject, size_t __nframes);
extern int                         frame_error(frameobject* __frameobject);
extern int                         frame_close(frameobject* __frameobject);


#endif