#include "video/frames.h"

int main () {
    frameobject* fobj = frame_open("/home/grostaco/thingg.mkv");
    frame_extract(fobj, AV_PIX_FMT_YUV420P, FRAME_NSAVE, 1, NULL);
    (void)fobj;
}