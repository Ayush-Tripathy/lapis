#include <Python.h>

#include "functions.h"
#include "frameobj.h"

static PyMethodDef LapisMethods[] = {
    {"read_csv", (PyCFunction)read_csv, METH_VARARGS | METH_KEYWORDS, "Reads a CSV file and returns a DataFrame."},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef lapismodule = {
    PyModuleDef_HEAD_INIT,
    "lapis",
    NULL,
    -1,
    LapisMethods};

PyMODINIT_FUNC PyInit_lapis(void)
{
    PyObject *m;
    if (PyType_Ready(&FrameType) < 0)
    {
        return NULL;
    }

    m = PyModule_Create(&lapismodule);
    if (m == NULL)
    {
        return NULL;
    }

    Py_INCREF(&FrameType);
    if (PyModule_AddObject(m, "DataFrame", (PyObject *)&FrameType) < 0)
    {
        Py_DECREF(&FrameType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}