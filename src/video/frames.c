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

#ifdef  FRAME_SILENT_AVERROR
#define fprintf(...)
#endif

#ifdef USE_FRAME_GLOBAL_ERROR_BUFFER
#define error_buf_len 128
static char error_buf[error_buf_len];
#else
#define error_buf self->_err_buf
#endif

// glibc's from libc-symbols.h
#define weak_alias(name, aliasname) _weak_alias(name, aliasname)
#define _weak_alias(name, aliasname)      \
    extern __typeof__(name) aliasname     \
    __attribute__((weak, alias(#name)))   \
    __attribute_copy__(name)                                      

#define attribute_hidden __attribute__((visibility("hidden")))

extern __frame_frameobject* _frame_open(char*) attribute_hidden;
extern int                  _frame_extract(__frame_frameobject*, enum AVPixelFormat, ...) attribute_hidden;
extern int                  _frame_error(__frame_frameobject*) attribute_hidden;
extern size_t               _frame_skip(__frame_frameobject*, size_t) attribute_hidden;
extern int                  _frame_close(__frame_frameobject*) attribute_hidden;

__frame_frameobject* _frame_open(char *file) {
    struct __frame_frameobject_plus *fop = calloc(1, sizeof *fop);
    fop->vtable = calloc(1, sizeof *fop->vtable);
    _FRAME_JUMPS(fop)->__extract    = frame_extract;
    _FRAME_JUMPS(fop)->__skip       = frame_skip;
    _FRAME_JUMPS(fop)->__error      = frame_error;
    _FRAME_JUMPS(fop)->__close      = frame_close;

    struct __frame_frameobject* self = (struct __frame_frameobject*)fop;
    self->_fCtx = avformat_alloc_context();
    self->_cCtx = avcodec_alloc_context3(NULL);
    self->_frame = av_frame_alloc();
    self->_frameRGB = av_frame_alloc();


    if ((self->_errnum = avformat_open_input(&self->_fCtx, file, NULL, NULL)) < 0) {
        av_strerror(self->_errnum, error_buf, error_buf_len);
        fprintf(stderr, "avformat_open_input: %s", error_buf);
        return NULL;
    }

    if ((self->_errnum = avformat_find_stream_info(self->_fCtx, NULL)) < 0) {
        av_strerror(self->_errnum, error_buf, error_buf_len);
        fprintf(stderr, "avformat_find_stream_info: %s", error_buf);
        return NULL;
    }

    for (self->_video_index = 0 ; (unsigned)self->_video_index < self->_fCtx->nb_streams && self->_fCtx->streams[self->_video_index] == AVMEDIA_TYPE_VIDEO ; ++self->_video_index);
    if (self->_fCtx->streams[self->_video_index] != AVMEDIA_TYPE_VIDEO) {
        snprintf(error_buf, 128, "__frame_extract_internal: cannot find video index for media");
        return NULL;
    }

    if ((self->_errnum = avcodec_parameters_to_context(self->_cCtx, self->_fCtx->streams[self->_video_index]->codecpar)) < 0) {
        av_strerror(self->_errnum, error_buf, error_buf_len);
        fprintf(stderr, "avcodec_parameters_to_context: %s", error_buf);
        return NULL;
    }

    if ((self->_codec = avcodec_find_decoder(self->_cCtx->codec_id)) == NULL) {
        fprintf(stderr, "avcodec_find_decoder: cannot find decoder for codec_id %d\n", self->_cCtx->codec_id);
        return NULL;  
    }

    if ((self->_errnum = avcodec_open2(self->_cCtx, self->_codec, NULL)) < 0) {
        av_strerror(self->_errnum, error_buf, error_buf_len);
        fprintf(stderr, "avcodec_open2: %s", error_buf);
        return NULL; 
    }

    self->_frame_rate = av_guess_frame_rate(self->_fCtx, self->_fCtx->streams[self->_video_index], NULL);
    self->_nbytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, self->_cCtx->width, self->_cCtx->height, 16);
    self->_underlying_buf = av_malloc(self->_nbytes);

    av_image_fill_arrays(self->_frameRGB->data, self->_frameRGB->linesize, (uint8_t*)self->_underlying_buf, AV_PIX_FMT_YUV420P, self->_cCtx->width, self->_cCtx->height, 16);
    self->_img_convert_ctx = sws_getContext(self->_cCtx->width, self->_cCtx->height,
                                            self->_cCtx->pix_fmt,
                                            self->_cCtx->width, self->_cCtx->height,
                                            self->_cCtx->pix_fmt, 
                                            SWS_BICUBIC,
                                            NULL, NULL, NULL);

    return self;
}
weak_alias(_frame_open, frame_open);

struct __frame_extract_opt {
    uint64_t bulksave:1;
    uint64_t nosave:1; 
    char*    format;
    int      nframes_requested;
};

int _frame_extract(__frame_frameobject* self, enum AVPixelFormat pixfmt, ...) {
    static void* dispatch_table[] = {&&flag_endarg, &&flag_nsave, &&flag_bulksave, &&flag_nosave};
    #define DISPATCH() goto *dispatch_table[va_arg(ap, int)]
    struct __frame_extract_opt extopt = {0};
    extopt.nframes_requested=1; // at least 1 frame
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

    // if FRAME_NOSAVE were passed with other save options, it will be ignored
    if (extopt.nosave && extopt.bulksave) {
        extopt.nosave=0;
    }else if (extopt.nosave && extopt.nframes_requested) {
        extopt.nosave=0;
    }

    

    return 0;
}
weak_alias(_frame_extract, frame_extract);

int _frame_error(__frame_frameobject* self) {
    return self->_errnum;
}

weak_alias(_frame_error, frame_error);

size_t _frame_skip(__frame_frameobject* self, size_t nframes) {
    (void)self;
    (void)nframes;
    return 0;
}
weak_alias(_frame_skip, frame_skip);

int _frame_close(__frame_frameobject* self) {
    (void)self;
    return 0;
}
weak_alias(_frame_close, frame_close);

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