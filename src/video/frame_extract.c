#include "video/frames.h"

extern int                  _frame_extract(__frame_frameobject*, enum AVPixelFormat, enum AVCodecID, char*, ...) attribute_hidden;

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
