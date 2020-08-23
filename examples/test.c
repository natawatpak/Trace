#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <stdint.h>
#include <stdio.h>
#include <math.h>


static inline void safeFrame(AVCodecContext* cCtx, AVFrame* frame, int nframe, int framerate) {
    AVCodec*            enc = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    AVCodecContext*     c   = avcodec_alloc_context3(NULL);
    AVPacket*           pkt = av_packet_alloc();
    static char         buffer[64];

    c->bit_rate = cCtx->bit_rate;
    c->width = cCtx->width;
    c->height = cCtx->height;
    c->time_base= (AVRational){1,25};
    c->pix_fmt = AV_PIX_FMT_YUVJ420P;

    frame->width = cCtx->width;
    frame->height = cCtx->height;
    frame->format=c->pix_fmt;

    avcodec_open2(c, enc, NULL);
    av_init_packet(pkt);
    avcodec_send_frame(c, frame);
    avcodec_receive_packet(c, pkt);
    sprintf(buffer, "Frames/f%d_%d.png", nframe/framerate, nframe%framerate);
    FILE* fp = fopen(buffer, "w");
    fwrite(pkt->data, 1, pkt->size, fp);
    fclose(fp);
    

    avcodec_close(c);
}


int main () {
    AVFormatContext* fCtx = NULL;
    char             error_buffer[256];
    unsigned int     i, vindex, r;
    AVCodecContext*  cCtx;
    AVCodec*         codec;
    AVFrame*         frame, *frameRGB;
    AVPacket         packet;
    int              nbytes;
    uint8_t*         buffer;

    frame = av_frame_alloc();
    frameRGB = av_frame_alloc();

    if ((r=avformat_open_input(&fCtx, "/home/grostaco/thing.webm", NULL, NULL)) != 0) {
        av_strerror(r, error_buffer, 256);
        fprintf(stderr, "avformat_open_input: %s\n", error_buffer);
        return 1; 
    }
    if ((r=avformat_find_stream_info(fCtx, NULL)) != 0) {
        av_strerror(r, error_buffer, 256);
        fprintf(stderr, "avformat_find_stream_info: %s\n", error_buffer);
        return 1; 
    }
    av_dump_format(fCtx, 0, "~/thing.webm", 0); 

    AVRational avfps = av_guess_frame_rate(fCtx, *fCtx->streams, NULL);
    int        fps   = ceil((double)avfps.num/avfps.den);
    
    vindex = -1;
    for (vindex = 0 ; vindex < fCtx->nb_streams && fCtx->streams[vindex]->codecpar->codec_type!=AVMEDIA_TYPE_VIDEO ; ++vindex);
    // vindex > fCtx->nbstreams should suffice, but it'd never hurt to be extra careful
    if (vindex > fCtx->nb_streams || fCtx->streams[vindex]->codecpar->codec_type!=AVMEDIA_TYPE_VIDEO) {    
        fprintf(stderr, "Cannot find video stream\n");
        return 1;
    }
    cCtx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(cCtx, fCtx->streams[vindex]->codecpar);

    if ((codec = avcodec_find_decoder(cCtx->codec_id)) == NULL) {
        fprintf(stderr, "Cannot find decoder for codec id: %d\n", cCtx->codec_id); 
        return 1;
    }
    

    if ((r = avcodec_open2(cCtx, codec, NULL)) != 0) {
        av_strerror(r, error_buffer, 256);
        fprintf(stderr, "avcodec_open2: %s\n", error_buffer);
        return 1; 
    }

    frame = av_frame_alloc();
    frameRGB = av_frame_alloc();
    if (!frame || !frameRGB) {
        av_strerror(r, error_buffer, 256);
        fprintf(stderr, "av_frame_alloc: %s\n", error_buffer);
        return 1; 
    }

    nbytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, cCtx->width, cCtx->height, 1);
    buffer = malloc(nbytes);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    avpicture_fill((AVPicture*) frameRGB, buffer, AV_PIX_FMT_YUV420P, cCtx->width, cCtx->height);
#pragma GCC diagnostic pop
    i = 0;
    while (av_read_frame(fCtx, &packet) >= 0) {
        if ((unsigned)packet.stream_index == vindex) {
            avcodec_send_packet(cCtx, &packet);
            r = avcodec_receive_frame(cCtx, frame);
            if (r == 0 && i % 15 == 0) {
                
                static struct SwsContext* img_convert_ctx;
                if (img_convert_ctx==NULL) {
                    img_convert_ctx = sws_getContext(cCtx->width, cCtx->height,
                                                     cCtx->pix_fmt, 
                                                     cCtx->width, cCtx->height, 
                                                     AV_PIX_FMT_YUV420P,
                                                     SWS_BICUBIC,
                                                     NULL, NULL, NULL);
                    if (img_convert_ctx == NULL) {
                        fprintf(stderr, "Unable to initialize the conversion context\n");
                        return 1;
                    }
                }
                
                r = sws_scale(img_convert_ctx, (const unsigned char* const*)frame->data, frame->linesize, 0, cCtx->height, frameRGB->data, frameRGB->linesize);
                
                safeFrame(cCtx, frameRGB, i, fps);
            }
            i++;
        }
    }
}
