#include "video/frames.h"

extern int                  _frame_close(__frame_frameobject*) attribute_hidden;

int _frame_close(__frame_frameobject* self) {
    free(_FRAME_REINTERPRET_ACCESS(self, struct __frame_frameobject_plus, vtable));

    av_free(self->_underlying_buf);
    av_frame_free(&self->_frame);
    av_frame_free(&self->_frameRGB);
    avformat_free_context(self->_fCtx);
    avcodec_free_context(&self->_cCtx);

    free(self);
    return 0;
}

weak_alias(_frame_close, frame_close);
