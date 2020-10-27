#include <Python.h>
#include <sys/stat.h>
#include "video/frames.h"

static PyObject* method_frame_open(PyObject* self, PyObject* args) {
    if (PyArg_ParseTuple(self, )) 
}

static PyObject* method_frame_skip(PyObject* self, PyObject* args) {

}

static PyObject* method_frame_extract(PyObject* self, PyObject* args) {

}

static PyObject* method_frame_close(PyObject* self, PyObject* args) {

}

static PyObject* method_frame_extract(PyObject* self, PyObject* args) {
    static char buffer[128];
    char *url, *dst;
    int  r;

    if (PyArg_ParseTuple(args, "ss|i", &url, &dst) == 0) {
        return NULL;
    }

    struct stat stb;

    if (stat(url, &stb) == -1) {
        return PyErr_Format(PyExc_FileNotFoundError, "source file '%s' does not exist", url);
    }

    char* eod = dst;
    while (*eod!='/' && *eod)eod++;
    bcopy(dst, buffer, eod-dst);
    buffer[eod-dst]='\0'; 


    if (stat(buffer, &stb) == -1) {
        return PyErr_Format(PyExc_FileNotFoundError, "destination folder '%s' does not exist", buffer);
    }
    if (!S_ISDIR(stb.st_mode)) {
        return PyErr_Format(PyExc_NotADirectoryError, "destination directory '%s' is not a directory", buffer);  
    }

    

    if ((r=extract_frames(url, AV_PIX_FMT_YUV420P, dst, NULL)) < 0) {
        av_strerror(r, buffer, 128);
        return PyErr_Format(PyExc_RuntimeError, "%s", buffer);
    }

    return PyLong_FromLong(0);
}

static PyMethodDef TraceMethods[] = {
    {"extract_frame", method_frame_extract, METH_VARARGS, 
     "extract_frame(source, dest_fmt) -> integer\n\nExtract frames from the source argument into destination in the given format\nThe first %d will be the seconds into the video, the second %d will be the current frame in said second"},
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