#include "video/extract.h"


int main () {
    AVFrame* frames = extract_frames("/home/grostaco/thingg.mkv");
    free(frames);
}
