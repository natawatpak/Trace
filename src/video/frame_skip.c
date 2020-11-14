#include "video/frames.h"

extern size_t               _frame_skip(__frame_frameobject*, size_t) attribute_hidden;

size_t _frame_skip(__frame_frameobject* self, size_t nframes) {
    _FRAME_EXTRACT(self, AV_PIX_FMT_RGB24, AV_CODEC_ID_MJPEG, "", FRAME_SKIP, FRAME_NSAVE, nframes, FRAME_ENDARG);  
    return 0;
}
weak_alias(_frame_skip, frame_skip);
