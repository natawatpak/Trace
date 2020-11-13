#include <Python.h>
#include <stdio.h>
#include <sys/stat.h>
#include "video/frames.h"
#include "structmember.h"

#define unused __attribute__((unused))


typedef struct py_frameobject{
    PyObject_HEAD;
    struct __frame_frameobject*     frameobject;
    struct __internal_frame_jump_t* vtable;
}FrameObject;


static int Frame_Init(FrameObject*, PyObject*, PyObject*);
static PyObject* Frame_Open(FrameObject*, PyObject*);
static PyObject* Frame_Close(FrameObject*, PyObject*);
static PyObject* Frame_Skip(FrameObject*, PyObject*);
static PyObject* Frame_Extract(FrameObject*, PyObject*, PyObject*);
static PyObject* Frame_Error(FrameObject*, PyObject*);


static PyMethodDef Frame_methods[] = {
    {"open",    (PyCFunction)Frame_Open,    METH_VARARGS, "Opens a file for the current frameobject to handle"},
    {"close",   (PyCFunction)Frame_Close,   METH_NOARGS,  "Free all resources allocated"},
    {"skip",    (PyCFunction)Frame_Skip,    METH_VARARGS, "Skip up to N frames"},
    {"extract", (PyCFunction)Frame_Extract, METH_VARARGS | METH_KEYWORDS, "Extract frames"},
    {"error",   (PyCFunction)Frame_Error,   METH_NOARGS,  "Return latest error code"},
    {NULL}
};

static PyTypeObject FrameType  = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name="frame.Frame",
    .tp_doc="A frame C object wrapper",
    .tp_basicsize=sizeof(FrameObject),
    .tp_itemsize=0,
    .tp_flags= Py_TPFLAGS_DEFAULT,
    .tp_new= PyType_GenericNew,
    .tp_init=(initproc)Frame_Init,
    .tp_methods=Frame_methods,
};

static PyModuleDef FrameModule = {
    PyModuleDef_HEAD_INIT,
    .m_name="frame",
    .m_size=-1,
};

static int Frame_Init(FrameObject* self, PyObject* args, PyObject* unused kwds) {
    char* url = NULL;
    if (!PyArg_ParseTuple(args, "|s", &url)) {
        return -1;
    }
    if (url != NULL) {
        self->frameobject = frame_open(url);
    }
    return 0;
}

static PyObject* Frame_Open(FrameObject* self, PyObject* args) {
    char *url;

    if (!PyArg_ParseTuple(args, "s", &url)) {
        return NULL;
    }

    struct stat stb;

    if (stat(url, &stb) == -1) {
        return PyErr_Format(PyExc_FileNotFoundError, "source file '%s' does not exist", url);
    }

    self->frameobject = frame_open(url); 

    return PyLong_FromLongLong(0);
}

static PyObject* Frame_Extract(FrameObject* self, PyObject* args, PyObject* kwds) {
    static char buffer[128];

    int nframes = 60;
    static char *kwlist[] = {"dstfmt", "nframes", "pixfmt", "dstid", "sepsave", "bulksave", "skip", NULL};
    char* dstfmt = NULL;
    int pixfmt = AV_PIX_FMT_YUV420P, dstid = AV_CODEC_ID_MJPEG, sepsave = 1, bulksave = 0, skip = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|iiiiii", kwlist, &dstfmt, &nframes, &pixfmt, &dstid, &sepsave, &bulksave, &skip)) {
        return NULL;
    }

    if (dstfmt == NULL) {
        return PyErr_Format(PyExc_TypeError, "Frame_Extract expects 1 positional argument 'dstfmt' (0 given)");
    }

    char* dstfmt_ptr = dstfmt;
    while (*dstfmt_ptr && *dstfmt_ptr != '/')++dstfmt_ptr;
    if (!bulksave && !*dstfmt_ptr) {
        PyErr_Format(PyExc_NameError, "Format for frame and second not specified, perhaps bulksave was intended");
        return NULL;
    }

    struct stat stb;
    memcpy(buffer, dstfmt, dstfmt_ptr-dstfmt);
    buffer[dstfmt_ptr-dstfmt] = '\0';
    if (stat(buffer, &stb) < 0) {
        perror("stat");
    }

    struct __frame_extract_opt extopt = {.bulksave=bulksave, .sepsave=sepsave, .nframes_requested=nframes, .skip=skip};

    return PyLong_FromLongLong(frame_optextract(self->frameobject, pixfmt, dstid, dstfmt, extopt));
}

static PyObject* Frame_Skip(FrameObject* self, PyObject* args) {
    return PyErr_Format(PyExc_NotImplementedError, "Frame_Skip currently not implemented\n");
}

static PyObject* Frame_Close(FrameObject* self, PyObject* unused dummy) {
    return PyLong_FromLong(frame_close(self->frameobject));
}

static PyObject* Frame_Error(FrameObject* self, PyObject* unused dummy) {
    return PyLong_FromLong(frame_error(self->frameobject));
}

PyMODINIT_FUNC PyInit_Frame(void) {
    PyObject* m;
    if (PyType_Ready(&FrameType) < 0) {
        return NULL;
    }
    m = PyModule_Create(&FrameModule);
    if (m == 0) {
        return NULL;
    }
    Py_IncRef((PyObject*)&FrameType); 
    if (PyModule_AddObject(m, "Frame", (PyObject*)&FrameType) < 0) {
        Py_DECREF(&FrameType);
        Py_DECREF(m);
    } 
    return m;
}



int main () {
    ;
}