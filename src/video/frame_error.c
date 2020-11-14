#include "video/frames.h"

extern int                  _frame_error(__frame_frameobject*) attribute_hidden;

int _frame_error(__frame_frameobject* self) {
    return self->_errnum;
}
weak_alias(_frame_error, frame_error); 