#ifndef __EXTRACT_H
#define __EXTRACT_H

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

AVFrame* extract_frames(AVFormatContext*, enum AVPixelFormat);
int      saveFrame(AVCodecContext*, AVFrame*, enum AVCodecID, const char*);

#endif