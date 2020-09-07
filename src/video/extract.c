#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#include "video/extract.h"

#ifndef DUMP_FORMAT 
#define DUMP_FORMAT 0
#endif

AVFrame* extract_frames(const char* url) {
    AVFormatContext* fCtx = NULL;
    AVCodecContext*  cCtx = NULL;
    AVCodec*         codec;
    AVFrame          *frame, *frameRGB, *frames;
    AVPacket         pkt;
    char             errorBuffer[256], *buffer;
    int              r;
    unsigned int     vindex, nbytes;

    if ((r = avformat_open_input(&fCtx, url, NULL, NULL)) < 0) {
        av_strerror(r, errorBuffer, 256);
        fprintf(stderr, "avformat_open_input: %s", errorBuffer);
        return NULL; 
    }

    if ((r = avformat_find_stream_info(fCtx, NULL)) != 0) {
        av_strerror(r, errorBuffer, 256);
        fprintf(stderr, "avformat_find_stream_info: %s", errorBuffer);
        return NULL; 
    }

#if DUMP_FORMAT
    av_dump_format(fCtx, 0, url, 0);
#endif

    AVRational avfps = av_guess_frame_rate(fCtx, *fCtx->streams, NULL);
    int        fps   = ceil((double)avfps.num/avfps.den);

    frames = malloc(fps * (fCtx->duration / AV_TIME_BASE));

    vindex = -1;
    for (vindex = 0 ; vindex < fCtx->nb_streams && fCtx->streams[vindex]->codecpar->codec_type!=AVMEDIA_TYPE_VIDEO ; ++vindex);
    if (vindex > fCtx->nb_streams || fCtx->streams[vindex]->codecpar->codec_type!=AVMEDIA_TYPE_VIDEO) {
        fprintf(stderr, "Cannot find video stream\n");
        return NULL;
    }

    cCtx = avcodec_alloc_context3(NULL);
    avcodec_parameters_to_context(cCtx, fCtx->streams[vindex]->codecpar);

    if ((codec = avcodec_find_decoder(cCtx->codec_id)) == NULL) {
        fprintf(stderr, "Cannot find decoder for codec id: %d\n", cCtx->codec_id);
        return NULL;
    }

    if ((r = avcodec_open2(cCtx, codec, NULL)) != 0) {
        av_strerror(r, errorBuffer, 256);
        fprintf(stderr, "avcodec_open2: %s\n", errorBuffer);
        return NULL; 
    }

    frame = av_frame_alloc();
    frameRGB = av_frame_alloc();
    if (!frame || !frameRGB) {
        av_strerror(r, errorBuffer, 256);
        fprintf(stderr, "av_frame_alloc: %s\n", errorBuffer);
        return NULL; 
    }

    nbytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, cCtx->width, cCtx->height, 16);
    buffer = av_malloc(nbytes);

    av_image_fill_arrays(frameRGB->data, frameRGB->linesize, (uint8_t*)buffer, AV_PIX_FMT_YUV420P, cCtx->width, cCtx->height, 16);
    
    AVFrame* frame_base = frames;
    static struct SwsContext* img_convert_ctx;
    if (img_convert_ctx == NULL) {
        img_convert_ctx =sws_getContext(cCtx->width, cCtx->height,
                       cCtx->pix_fmt,
                       cCtx->width, cCtx->height,
                       AV_PIX_FMT_YUV420P,
                       SWS_BICUBIC,
                       NULL, NULL, NULL);
    }

    while (av_read_frame(fCtx, &pkt) >= 0) {
        if ((unsigned)pkt.stream_index == vindex) {
            avcodec_send_packet(cCtx, &pkt);
            r = avcodec_receive_frame(cCtx, frame);
            if (r == 0) {
                r = sws_scale(img_convert_ctx, (const unsigned char* const*)frame->data, frame->linesize, 0, cCtx->height, frameRGB->data, frameRGB->linesize);
                *frames++ = *frameRGB;
            }
        }
    }

    avcodec_free_context(&cCtx);

    return frame_base;
}