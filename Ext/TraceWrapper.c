#include <Python.h>
#include "video/extract.h"

static PyObject* method_frame_extract(PyObject* self, PyObject* args) {
    (void)self;
    char *url, *dst;

    if (PyArg_ParseTuple(args, "ss", &url, &dst) == 0) {
        return NULL;
    }

    (void)extract_frames(url, AV_PIX_FMT_YUV420P, dst, NULL);

    return PyLong_FromLong(0);
}

static PyMethodDef TraceMethods[] = {
    {"extract_frame", method_frame_extract, METH_VARARGS, "Trace backend for frame extraction"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef TraceModule  = {
    PyModuleDef_HEAD_INIT,
    "Trace",
    "Python interface for the Trace backend",
    -1,
    TraceMethods,
    0,
    0,
    0,
    0
};

PyMODINIT_FUNC PyInit_Trace(void) {
    return PyModule_Create(&TraceModule);
}

int main () {
    
}