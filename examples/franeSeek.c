#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#include "video/extract.h"

int main () {
    AVFormatContext* fCtx = avformat_alloc_context();
    AVCodecContext*  cCtx = avcodec_alloc_context3(NULL);
    AVFrame*         frame, *frameRGB;
    AVCodec*         codec;
    AVPacket         pkt;
    char*            buffer;
    int              r, nbytes;

    avformat_open_input(&fCtx, "/home/grostaco/thing2.mp4", NULL, NULL);
    avformat_find_stream_info(fCtx, NULL);
    av_dump_format(fCtx, 0, "/home/grostaco/thing2.mp4", 0);

    avcodec_parameters_to_context(cCtx, fCtx->streams[0]->codecpar);
    codec = avcodec_find_decoder(cCtx->codec_id);
    avcodec_open2(cCtx, codec, NULL);

    frame = av_frame_alloc();
    frameRGB = av_frame_alloc();

    nbytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, cCtx->width, cCtx->height, 16);
    buffer = av_malloc(nbytes);

    av_image_fill_arrays(frameRGB->data, frameRGB->linesize, (uint8_t*)buffer, AV_PIX_FMT_YUV420P, cCtx->width, cCtx->height, 16);
    
    struct SwsContext* img_convert_ctx = sws_getContext(cCtx->width, cCtx->height,
                                                        cCtx->pix_fmt,
                                                        cCtx->width, cCtx->height,
                                                        AV_PIX_FMT_YUV420P,
                                                        SWS_BICUBIC,
                                                        NULL, NULL, NULL);
    
    //AVRational timeBase = fCtx->streams[0]->time_base;
    //int seek_pos = (int)(((float)50000)/1000 * (float)AV_TIME_BASE);
    //int seek_target = av_rescale_q(seek_pos, AV_TIME_BASE_Q, timeBase);


    
    
    while (av_read_frame(fCtx, &pkt) >= 0) {
        if (pkt.stream_index == 0) {
            avcodec_send_packet(cCtx, &pkt);
            r = avcodec_receive_frame(cCtx, frame);
            if (r == 0) {
                sws_scale(img_convert_ctx, (const unsigned char* const*)frame->data, frame->linesize, 0, cCtx->height, frameRGB->data, frameRGB->linesize);
                saveFrame(cCtx, frameRGB, AV_CODEC_ID_MJPEG, "Frames/foo.jpeg"); 
                break;
            }
        }
    }
}