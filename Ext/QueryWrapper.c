#include <Python.h>
#include "structmember.h"
#include "query/query.h"
#include "common.h"

typedef struct py_queryobject {
    PyObject_HEAD;
    QueryObject* qobj;
}Py_QueryObject;

typedef struct py_anidb_response {
    PyObject_HEAD;
    anidb_response* anires; 
}AnidbResponseObject;

static int Query_Init(Py_QueryObject*, PyObject*, PyObject*);
static PyObject* Query_Establish_Connection(Py_QueryObject*, PyObject*);
static PyObject* Query_Refresh_Session(Py_QueryObject*, PyObject*);
static PyObject* Query_By_Id(Py_QueryObject*, PyObject*);
static PyObject* Query_Free(Py_QueryObject*, PyObject*);

static PyObject* Query_Anidb_Response_Free(PyObject*, PyObject*);
static PyObject* Query_Int_To_Amask(PyObject*, PyObject*);

static PyMethodDef Query_Methods[] = {
    {"establish_connection", (PyCFunction)Query_Establish_Connection, METH_NOARGS,  "Connect to API endpoint"},
    {"refresh_session",      (PyCFunction)Query_Refresh_Session,      METH_NOARGS,  "Create a new session"},
    {"by_id",                (PyCFunction)Query_By_Id,                METH_VARARGS, "Query API endpoint with aid"},
    {"free",                 (PyCFunction)Query_Free,                 METH_NOARGS,  "Free all allocated resources"},
    {NULL,                   NULL,                                    0,            NULL},
};

static PyMethodDef IQuery_Methods[] = {
    {"anidb_response_free",  (PyCFunction)Query_Anidb_Response_Free,  METH_VARARGS, NULL},
    {"int_to_amask",         (PyCFunction)Query_Int_To_Amask,         METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL},
};

static PyTypeObject QueryType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name="query.query",
    .tp_doc="anidb UDP API wrapper",
    .tp_basicsize=sizeof(Py_QueryObject),
    .tp_itemsize=0,
    .tp_flags=Py_TPFLAGS_DEFAULT,
    .tp_new=PyType_GenericNew,
    .tp_init=(initproc)Query_Init,
    .tp_methods=Query_Methods,
};

static PyModuleDef QueryModule = {
    PyModuleDef_HEAD_INIT,
    .m_name="query",
    .m_size=-1,
    IQuery_Methods,
};

static int Query_Init(Py_QueryObject* self, PyObject* args, PyObject* kwds) {
    char* username, *password, *client, *clientver;
    if (!PyArg_ParseTuple(args, "ssss", &username, &password, &client, &clientver)) {
        return -1;
    }
    self->qobj = query_new(username, password, client, clientver);
    return 0;
}

static PyObject* Query_Establish_Connection(Py_QueryObject* self, PyObject* args) {
    query_establish_connection(self->qobj); 
    return PyLong_FromLongLong(0);
}

static PyObject* Query_Refresh_Session(Py_QueryObject* self, PyObject* args) {
    query_refresh_session(self->qobj);
    return PyLong_FromLongLong(0);
}

static PyObject* Query_By_Id(Py_QueryObject* self, PyObject* args) {
    int id;
    char* amask;
    if (!PyArg_ParseTuple(args, "is", &id, &amask)) {
        return NULL;
    }

    AnidbResponseObject* ani_res = malloc(sizeof *ani_res);
    ani_res->anires = query_by_id(self->qobj, id, amask);
    return &ani_res;
}

static PyObject* Query_Free_Anidb_Response(AnidbResponseObject ani_res) {
    query_anidb_response_free(ani_res.anires);
    return PyLong_FromLongLong(0);
}

static PyObject* Query_Free(Py_QueryObject* self, PyObject* args) {
    query_free(self->qobj);
    return PyLong_FromLong(0);
}

static PyObject* Query_Int_To_Amask(PyObject* self, PyObject* args) {
    uint64_t i;
    if (!PyArg_ParseTuple(args, "i", &i) < 0) {
        return NULL;
    }

    return Py_BuildValue("s", query_int_to_amask(i));
}

static PyObject* Query_Anidb_Response_Free(PyObject* self, PyObject* args) {
    AnidbResponseObject ani_res;
    
    if (!PyArg_ParseTuple(args, "O", &ani_res)) {
        return NULL;
    }
    query_anidb_response_free(ani_res.anires);
    return PyLong_FromLongLong(0);
}

PyMODINIT_FUNC PyInit_Query(void) {
    PyObject* module;
    if (PyType_Ready(&QueryType) < 0) {
        return NULL;
    }
    module = PyModule_Create(&QueryType);
    if (module == 0) {
        return NULL;
    }
    Py_IncRef((PyObject*)&QueryType);
    if (PyModule_AddObject(module, "Query", (PyObject*)&QueryType) < 0) {
        Py_DECREF(&QueryType);
        Py_DECREF(module);
    }
    return module;
}