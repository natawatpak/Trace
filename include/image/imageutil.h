#ifndef __IMAGEUTIL_H
#define __IMAGEUTIL_H

#include <libavcodec/codec.h>
#include <libavformat/avformat.h>

AVFrame* openImage(const char*, AVCodecContext*);

#endif