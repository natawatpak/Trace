#include "video/frames.h"

extern __frame_frameobject* _frame_open(char*) attribute_hidden;
extern int                  _frame_extract(__frame_frameobject*, enum AVPixelFormat, enum AVCodecID, char*, ...) attribute_hidden;
extern int                  _frame_error(__frame_frameobject*) attribute_hidden;
extern size_t               _frame_skip(__frame_frameobject*, size_t) attribute_hidden;
extern int                  _frame_close(__frame_frameobject*) attribute_hidden;


__frame_frameobject* _frame_open(char *file) {
    struct __frame_frameobject_plus *fop = calloc(1, sizeof *fop);
    fop->vtable = calloc(1, sizeof *fop->vtable);
    _FRAME_JUMPS(fop)->__extract    = frame_extract;
    _FRAME_JUMPS(fop)->__skip       = frame_skip;
    _FRAME_JUMPS(fop)->__error      = frame_error;
    _FRAME_JUMPS(fop)->__close      = frame_close;

    struct __frame_frameobject* self = (struct __frame_frameobject*)fop;
    self->_fCtx = avformat_alloc_context();
    self->_cCtx = avcodec_alloc_context3(NULL);
    self->_frame = av_frame_alloc();
    self->_frameRGB = av_frame_alloc();


    if ((self->_errnum = avformat_open_input(&self->_fCtx, file, NULL, NULL)) < 0) {
        av_strerror(self->_errnum, error_buf, error_buf_len);
        fprintf(stderr, "avformat_open_input: %s", error_buf);
        return NULL;
    }

    if ((self->_errnum = avformat_find_stream_info(self->_fCtx, NULL)) < 0) {
        av_strerror(self->_errnum, error_buf, error_buf_len);
        fprintf(stderr, "avformat_find_stream_info: %s", error_buf);
        return NULL;
    }

    for (self->_video_index = 0 ; (unsigned)self->_video_index < self->_fCtx->nb_streams && self->_fCtx->streams[self->_video_index]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO ; ++self->_video_index);
    if (self->_fCtx->streams[self->_video_index]->codecpar->codec_type != AVMEDIA_TYPE_VIDEO) {
        snprintf(error_buf, 128, "frame_extract: cannot find video index for media");
        return NULL;
    }

    if ((self->_errnum = avcodec_parameters_to_context(self->_cCtx, self->_fCtx->streams[self->_video_index]->codecpar)) < 0) {
        av_strerror(self->_errnum, error_buf, error_buf_len);
        fprintf(stderr, "avcodec_parameters_to_context: %s", error_buf);
        return NULL;
    }

    if ((self->_codec = avcodec_find_decoder(self->_cCtx->codec_id)) == NULL) {
        fprintf(stderr, "avcodec_find_decoder: cannot find decoder for codec_id %d\n", self->_cCtx->codec_id);
        return NULL;  
    }

    if ((self->_errnum = avcodec_open2(self->_cCtx, self->_codec, NULL)) < 0) {
        av_strerror(self->_errnum, error_buf, error_buf_len);
        fprintf(stderr, "avcodec_open2: %s", error_buf);
        return NULL; 
    }

    self->_frame_rate = av_guess_frame_rate(self->_fCtx, self->_fCtx->streams[self->_video_index], NULL);
    self->_nbytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, self->_cCtx->width, self->_cCtx->height, 16);
    self->_underlying_buf = av_malloc(self->_nbytes) ;

    av_image_fill_arrays(self->_frameRGB->data, self->_frameRGB->linesize, (uint8_t*)self->_underlying_buf, AV_PIX_FMT_YUV420P, self->_cCtx->width, self->_cCtx->height, 16);
    return self;
}
weak_alias(_frame_open, frame_open);
