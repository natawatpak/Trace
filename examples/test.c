#include "video/extract.h"
#include "image/imageutil.h"

int main () {
    extract_frames("/home/grostaco/thingg.mkv", AV_PIX_FMT_YUV420P, "Frames/e_%i_%i.jpg");
}
