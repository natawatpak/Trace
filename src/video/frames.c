#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <assert.h>
#include <stdarg.h>

#include "cleanup.h"
#include "video/frames.h"
#include "image/imageutil.h"

/*
 The OOP nature of this code is heavily inspired by glibc FILE 
*/

extern int __frame_skip_internal(__frame_frameobject* self, int to_skip);
extern int __frame_deallocate_internal(__frame_frameobject* self);
extern int __frame_extract_internal(__frame_frameobject*, enum AVPixelFormat, struct __frame_extract_opt);

#ifndef _FRAME_NO_USE_GLOBAL_BUFFER
#define error_buf_len 128
static char error_buf[error_buf_len];
#else
#error "local error buffers not supported (yet)"
#endif

#define VIDEO_INDEX(fmtCtx, vindex) ({ \
    for (vindex = 0 ; vindex < fCtx->nb_streams && fCtx->streams[vindex]->codecpar->codec_type!=AVMEDIA_TYPE_VIDEO ; ++vindex); \
    vindex;                            \
})

#define FRAME_COND_NOT(cond) !(cond)
#define FRAME_COND_LE_ARBI(n) < n
#define FRAME_COND_GE_ARBI(n) > n
#define FRAME_COND_EQ_ARBI(n) == n
#define FRAME_COND_GT_OR_EQ_ARBI(n) >= n
#define FRAME_COND_LE_OR_EQ_ARBI(n) <= n
#define FRAME_COND_NEQ_ARBI(n)      != n
#define FRAME_COND_LE_ZERO FRAME_COND_LE_ARBI(0)
#define FRAME_COND_GE_ZERO FRAME_COND_GE_ARBI(0)
#define FRAME_COND_EQ_ZERO FRAME_COND_EQ_ARBI(0)
#define FRAME_COND_GT_OR_EQ_ZERO FRAME_COND_GT_OR_EQ_ARBI(0)
#define FRAME_COND_LE_OR_EQ_ZERO FRAME_COND_LE_OR_EQ_ARBI(0)
#define FRAME_COND_NEQ_ZERO      FRAME_COND_NEQ_ARBI(0)

#define FRAME_HASFLAG(n,f) ((n & f) == f)

#define FRAME_ENDARG NULL




#define FRAMEAV_EXPLAIN(func, rcond, buf, buflen, ...) ({ \
    int r;                                                \
    if ((r = func(__VA_ARGS__)) rcond) {                  \
        av_strerror(r, buf, buflen);                      \
        printf("%s: %s\n", #func, buf);                   \
    }                                                     \
    r rcond;                                              \
})


__frame_frameobject* frame_open(char* file) {
    struct __frame_frameobject_plus* fop = calloc(1, sizeof *fop);
    fop->vtable = calloc(1, sizeof *fop->vtable);
    _FRAME_JUMPS(fop)->__extract    = __frame_extract_internal;
    _FRAME_JUMPS(fop)->__skip       = (void*)0x69; (void)__frame_skip_internal;
    _FRAME_JUMPS(fop)->__deallocate = __frame_deallocate_internal;

    struct __frame_frameobject* self = (struct __frame_frameobject*)fop;
    self->fCtx = avformat_alloc_context();
    self->cCtx = avcodec_alloc_context3(NULL);

    if (FRAMEAV_EXPLAIN(avformat_open_input, FRAME_COND_LE_ZERO, error_buf, 128, &self->fCtx, file, NULL, NULL)) {
        return NULL;
    }
    if (FRAMEAV_EXPLAIN(avformat_find_stream_info, FRAME_COND_NEQ_ZERO, error_buf, error_buf_len, self->fCtx, NULL)) {
        return NULL;
    }

    return self;
}

int frame_extract(__frame_frameobject* self, enum AVPixelFormat pixfmt, ...) {
    static void* dispatch_table[] = {&&flag_endarg, &&flag_nsave, &&flag_bulksave, &&flag_nosave};
    #define DISPATCH() goto *dispatch_table[va_arg(ap, int)]
    struct __frame_extract_opt extopt = {0};
    va_list ap;
    
    va_start(ap, pixfmt);

    DISPATCH();
    flag_nsave :
        extopt.nframes_requested  = va_arg(ap, int);
        DISPATCH();
    flag_bulksave:
        extopt.bulksave=1;
        DISPATCH();
    flag_nosave :
        extopt.nosave=1;
        DISPATCH();
    flag_endarg :

    va_end(ap);

    _FRAME_EXTRACT(self, pixfmt, extopt);

    return -1;
}

int __frame_skip_internal(__frame_frameobject* self, int to_skip) {
    (void)self;
    (void)to_skip;
    return 0;
}

int __frame_deallocate_internal(__frame_frameobject* self) {
    (void)self;
    return 0;
}

int __frame_extract_internal(__frame_frameobject* self, enum AVPixelFormat pixfmt, struct __frame_extract_opt extopt) {
    
    return 0;
}

/*
static int __frame_extract_frames(const char* url, enum AVPixelFormat pixfmt, const char* dst, AVCodecContext* c) {
    AVFormatContext* fCtx;
    AVCodecContext*  cCtx = NULL;
    AVCodec*         codec;
    AVFrame          *frame, *frameRGB;
    AVPacket         pkt;
    char             errorBuffer[256], *buffer, nBuffer[64];
    int              r;
    unsigned int     vindex, nbytes;

    fCtx = avformat_alloc_context();
    if ((r = avformat_open_input(&fCtx, url, NULL, NULL)) < 0) {
        av_strerror(r, errorBuffer, 256);
        fprintf(stderr, "avformat_open_input: %s\n", errorBuffer);
        return r; 
    }

    if ((r = avformat_find_stream_info(fCtx, NULL)) != 0) {
        av_strerror(r, errorBuffer, 256);
        fprintf(stderr, "avformat_find_stream_info: %s\n", errorBuffer);
        return r; 
    }



#if DUMP_FORMAT
    av_dump_format(fCtx, 0, url, 0);
#endif

    AVRational avfps = av_guess_frame_rate(fCtx, *fCtx->streams, NULL);
    int        fps   = ceil((double)avfps.num/avfps.den);

    VIDEO_INDEX(fCtx, vindex);

    if (vindex > fCtx->nb_streams || fCtx->streams[vindex]->codecpar->codec_type!=AVMEDIA_TYPE_VIDEO) {
        fprintf(stderr, "Cannot find video stream\n");
        return r;
    }

    if (c != NULL) {
        cCtx = c;
    } else {
        cCtx = avcodec_alloc_context3(NULL);
    }

    avcodec_parameters_to_context(cCtx, fCtx->streams[vindex]->codecpar);

    if ((codec = avcodec_find_decoder(cCtx->codec_id)) == NULL) {
        fprintf(stderr, "Cannot find decoder for codec id: %d\n", cCtx->codec_id);
        return r;
    }

    if ((r = avcodec_open2(cCtx, codec, NULL)) != 0) {
        av_strerror(r, errorBuffer, 256);
        fprintf(stderr, "avcodec_open2: %s\n", errorBuffer);
        return r; 
    }

    frame = av_frame_alloc();
    frameRGB = av_frame_alloc();
    if (!frame || !frameRGB) {
        av_strerror(r, errorBuffer, 256);
        fprintf(stderr, "av_frame_alloc: %s\n", errorBuffer);
        return r; 
    }

    nbytes = av_image_get_buffer_size(pixfmt, cCtx->width, cCtx->height, 16);
    buffer = av_malloc(nbytes);

    av_image_fill_arrays(frameRGB->data, frameRGB->linesize, (uint8_t*)buffer, pixfmt, cCtx->width, cCtx->height, 16);
    
    static struct SwsContext* img_convert_ctx;
    if (img_convert_ctx == NULL) {
        img_convert_ctx =sws_getContext(cCtx->width, cCtx->height,
                       cCtx->pix_fmt,
                       cCtx->width, cCtx->height,
                       pixfmt,
                       SWS_BICUBIC,
                       NULL, NULL, NULL);
    }

    int i = 0;
    while (av_read_frame(fCtx, &pkt) >= 0) {
        if ((unsigned)pkt.stream_index == vindex) {
            avcodec_send_packet(cCtx, &pkt);
            r = avcodec_receive_frame(cCtx, frame);
            if (r == 0) {
                r = sws_scale(img_convert_ctx, (const unsigned char* const*)frame->data, frame->linesize, 0, cCtx->height, frameRGB->data, frameRGB->linesize);

                snprintf(nBuffer, 64, dst, i / fps, i % fps);
                saveFrame(cCtx, frameRGB, AV_CODEC_ID_MJPEG, nBuffer);
                i++;
#ifdef FIRST_FRAME_ONLY
                break;
#endif
            }
        }
    }

    avformat_free_context(fCtx);
    if (c == NULL) {
        avcodec_free_context(&cCtx);
    }
    av_frame_free(&frame);
    av_frame_free(&frameRGB);
    av_free(buffer);
    

    return 0;
}

static int saveFrame(AVCodecContext* cCtx, AVFrame* frame, enum AVCodecID codecID, const char* dst) {
    AVCodec*            enc = avcodec_find_encoder(codecID);
    AVCodecContext*     c   = avcodec_alloc_context3(NULL);
    AVPacket*           pkt;
    int                 r;

    pkt = av_packet_alloc();
    c->bit_rate = cCtx->bit_rate;
    c->width = cCtx->width;
    c->height = cCtx->height;
    c->time_base= (AVRational){1,25};
    c->pix_fmt = AV_PIX_FMT_YUVJ420P;

    frame->width = cCtx->width;
    frame->height = cCtx->height;
    frame->format=c->pix_fmt;

    avcodec_open2(c, enc, NULL);

    if ((r = avcodec_send_frame(c, frame)) < 0) {
        return r;
    }
    if ((r = avcodec_receive_packet(c, pkt)) < 0) {
        return r;
    }
    

    FILE* fp = fopen(dst, "w");
    if (fp == NULL) {
        perror("fopen");
        
        return errno;
    }

    fwrite(pkt->data, 1, pkt->size, fp);
    fclose(fp);

    av_packet_free(&pkt);
    avcodec_close(c);
    avcodec_free_context(&c);
    return 0;
}

*/