#include "image/imageutil.h"

#include <libavformat/avformat.h>
#include <libavcodec/codec.h>
#include <libavutil/imgutils.h>

#include <stdbool.h>

AVFrame* openImage (const char* url, AVCodecContext* c) {
    AVFormatContext  *fCtx = avformat_alloc_context();
    AVCodecContext   *cCtx = avcodec_alloc_context3(NULL);
    AVFrame          *frameRGB = av_frame_alloc();
    AVPacket         pkt;

    int r;

    if ((r = avformat_open_input(&fCtx, url, NULL, NULL)) < 0) {
        return NULL;
    }

    avcodec_parameters_to_context(cCtx, fCtx->streams[0]->codecpar);
    cCtx->width = c->width;
    cCtx->height = c->height;
    cCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    AVCodec* codec = avcodec_find_decoder(cCtx->codec_id);
    if (codec == NULL) {
        return NULL;
    }

    if (avcodec_open2(cCtx, codec, NULL) < 0) {
        return NULL;
    }

#ifdef DUMP_FORMAT
    av_dump_format(fCtx, 0, url, false);
#endif

    int nbytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, cCtx->width, cCtx->height, 16);
    uint8_t* buffer = av_malloc(nbytes);

    av_image_fill_arrays(frameRGB->data, frameRGB->linesize, buffer, AV_PIX_FMT_YUV420P, cCtx->width, cCtx->height, 16);

    while (av_read_frame(fCtx, &pkt) >= 0) {
        if (pkt.stream_index != 0) {
            continue;
        }
        avcodec_send_packet(cCtx, &pkt);
        r = avcodec_receive_frame(cCtx, frameRGB);
        if (r == 0) {
            break;
        }

    }

    avformat_free_context(fCtx);
    avcodec_free_context(&cCtx);
    av_free(buffer);
    return frameRGB;
}