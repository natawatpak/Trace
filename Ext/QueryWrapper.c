#include <Python.h>
#include "query/query.h"
#include "common.h"

typedef struct py_queryobject {
    PyObject_HEAD;
    QueryObject qobj;
}QueryObject;

typedef struct py_anidb_response {
    PyObject_HEAD;
    anidb_response anires; 
}AnidbResponseObject;

static int Query_Init(QueryObject*, PyObject*, PyObject*);
static PyObject* Query_Establish_Connection(QueryObject*, PyObject*);
static PyObject* Query_Refresh_Session(QueryObject*, PyObject*);
static AnidbResponseObject Query_By_Id(QueryObject*, PyObject*);
static void Query_Free(QueryObject*, PyObject*);

static PyMethodDef Query_Methods[] = {
    {"establish_connection", (PyCFunction)Query_Establish_Connection, METH_NOARGS,  "Connect to API endpoint"},
    {"refresh_session",      (PyCFunction)Query_Refresh_Session,      METH_NOARGS,  "Create a new session"},
    {"by_id",                (PyCFunction)Query_By_Id,                METH_VARARGS, "Query API endpoint with aid"},
    {"free",                 (PyCFunction)Query_Free,                 METH_NOARGS,  "Free all allocated resources"},
};

static PyTypeObject QueryType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name="query.query",
    .tp_doc="anidb UDP API wrapper",
    .tp_basicsize=sizeof(QueryObject),
    .tp_itemsize=0,
    .tp_flags=Py_TPFLAGS_DEFAULT,
    .tp_new=PyType_GenericNew,
    .tp_init=(initproc)Query_Init,
    .tp_methods=Query_Methods,
};

