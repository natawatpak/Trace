#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <assert.h>
#include <stdarg.h>

#include "cleanup.h"
#include "video/frames.h"

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
extern int                  _frame_extract(__frame_frameobject*, enum AVPixelFormat, enum AVCodecID, char*, ...) attribute_hidden;
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

    for (self->_video_index = 0 ; (unsigned)self->_video_index < self->_fCtx->nb_streams && self->_fCtx->streams[self->_video_index]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO ; ++self->_video_index);
    if (self->_fCtx->streams[self->_video_index]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO) {
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
    self->_underlying_buf = av_malloc(self->_nbytes) ;

    av_image_fill_arrays(self->_frameRGB->data, self->_frameRGB->linesize, (uint8_t*)self->_underlying_buf, AV_PIX_FMT_YUV420P, self->_cCtx->width, self->_cCtx->height, 16);
    return self;
}
weak_alias(_frame_open, frame_open);

struct __frame_extract_opt {
    uint64_t bulksave:1;
    uint64_t sepsave:1; 
    uint64_t skip:1;
    char*    format;
    int      nframes_requested;
};

int _frame_extract(__frame_frameobject* self, enum AVPixelFormat pixfmt, enum AVCodecID codecID, char* format, ...) {
    static void* dispatch_table[] = {&&flag_endarg, &&flag_nsave, &&flag_bulksave, &&flag_sepsave, &&flag_skip};
    #define DISPATCH() goto *dispatch_table[va_arg(ap, int)]
    // default sep save
    struct __frame_extract_opt extopt = {.sepsave=1, .nframes_requested=1};

    va_list ap;
    
    va_start(ap, format);

    DISPATCH();
    flag_nsave :
        extopt.nframes_requested  = va_arg(ap, int);
        DISPATCH();
    flag_bulksave:
        extopt.bulksave=1;
        extopt.sepsave=0;
        DISPATCH();
    flag_sepsave :
        extopt.sepsave=1;
        extopt.bulksave=0;
        DISPATCH();
    flag_skip :
        extopt.skip=1;
        DISPATCH();
    flag_endarg :
        va_end(ap);

    if (self->_img_convert_ctx == NULL && !extopt.skip) {
        self->_img_convert_ctx = sws_getContext(self->_cCtx->width, self->_cCtx->height,
                                            self->_cCtx->pix_fmt,
                                            self->_cCtx->width, self->_cCtx->height,
                                            pixfmt, 
                                            SWS_BICUBIC,
                                            NULL, NULL, NULL);
    }

    AVCodecContext* c = avcodec_alloc_context3(NULL);
    AVCodec* codec = avcodec_find_encoder(codecID);    
    AVPacket pkt;
    FILE*    dst;
    char     filename_buffer[256];

    if (extopt.bulksave) {
        dst = fopen(format, "w");
    }

    c->bit_rate  = self->_cCtx->bit_rate;
    c->width     = self->_cCtx->width;
    c->height    = self->_cCtx->height;
    c->time_base = (AVRational){1,25};
    c->pix_fmt   = AV_PIX_FMT_YUVJ420P;

    self->_frameRGB->width  = self->_cCtx->width;
    self->_frameRGB->height = self->_cCtx->height;
    self->_frameRGB->format = c->pix_fmt;

    avcodec_open2(c, codec, NULL);

    while(extopt.nframes_requested && av_read_frame(self->_fCtx, &pkt) >= 0) {
        if (pkt.stream_index != self->_video_index) {
            continue;
        }
        if ((self->_errnum = avcodec_send_packet(self->_cCtx, &pkt)) < 0) {
            av_strerror(self->_errnum, error_buf, error_buf_len);
            fprintf(stderr, "avcodec_send_packet: %s\n", error_buf);
            return self->_errnum;  
        }
        if ((self->_errnum = avcodec_receive_frame(self->_cCtx, self->_frame)) != 0) {
            continue;
        }

        if (extopt.skip) {
            extopt.nframes_requested--;
            self->_cframe++;
            continue;
        }        

        sws_scale(self->_img_convert_ctx, (const unsigned char* const*)self->_frame->data, self->_frame->linesize, 0, self->_cCtx->height, self->_frameRGB->data, self->_frameRGB->linesize);
        
        if ((self->_errnum = avcodec_send_frame(c, self->_frameRGB)) < 0) {
            av_strerror(self->_errnum, error_buf, error_buf_len);
            fprintf(stderr, "avcodec_send_frame: %s", error_buf);       
            return self->_errnum;
        }
        if ((self->_errnum = avcodec_receive_packet(c, &pkt)) < 0) {
            av_strerror(self->_errnum, error_buf, error_buf_len);
            fprintf(stderr, "avcodec_receieve_packet: %s", error_buf);
            return self->_errnum;
        }
        
        if (extopt.bulksave) {
            fwrite(&pkt.size, sizeof(pkt.size), 1, dst);
            fwrite(pkt.data, pkt.size, 1, dst);
        }else {
            snprintf(filename_buffer, 256, format, self->_cframe/60, self->_cframe%60);
            dst = fopen(filename_buffer, "w");
            if (dst == NULL) {
                perror("fopen");
                return -errno;
            }

            fwrite(pkt.data, pkt.size, 1, dst);
            fclose(dst);
        }

        // repeat until finished if nframes_requested is -1
        if (extopt.nframes_requested != -1) {
            extopt.nframes_requested--;
        }
        self->_cframe++;
    }    

    if (extopt.bulksave) {
        fclose(dst);
    }

    avcodec_close(c);
    avcodec_free_context(&c);

    return 0;
}
weak_alias(_frame_extract, frame_extract);

int _frame_error(__frame_frameobject* self) {
    return self->_errnum;
}

weak_alias(_frame_error, frame_error);

size_t _frame_skip(__frame_frameobject* self, size_t nframes) {
    _FRAME_EXTRACT(self, AV_PIX_FMT_RGB24, AV_CODEC_ID_MJPEG, "", FRAME_SKIP, FRAME_NSAVE, nframes, FRAME_ENDARG);  
    return 0;
}
weak_alias(_frame_skip, frame_skip);

int _frame_close(__frame_frameobject* self) {
    free(_FRAME_REINTERPRET_ACCESS(self, struct __frame_frameobject_plus, vtable));

    av_free(self->_underlying_buf);
    av_frame_free(&self->_frame);
    av_frame_free(&self->_frameRGB);
    avformat_free_context(self->_fCtx);
    avcodec_free_context(&self->_cCtx);

    free(self);
    return 0;
}
weak_alias(_frame_close, frame_close);
