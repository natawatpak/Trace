#ifndef __EXTRACT_H
#define __EXTRACT_H

#include <libavcodec/avcodec.h>

AVFrame* extract_frames(const char*, enum AVCodecID);

#endif