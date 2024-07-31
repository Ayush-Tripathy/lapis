#include <Python.h>

#include "functions.h"
#include "frameobj.h"

static PyMethodDef PyFrameMethods[] = {
    {"read_csv", read_csv, METH_VARARGS, "Read the entire CSV file"},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef pyframemodule = {
    PyModuleDef_HEAD_INIT,
    "pyf",
    NULL,
    -1,
    PyFrameMethods};

PyMODINIT_FUNC PyInit_pyf(void)
{
    PyObject *m;
    if (PyType_Ready(&FrameType) < 0)
    {
        return NULL;
    }

    m = PyModule_Create(&pyframemodule);
    if (m == NULL)
    {
        return NULL;
    }

    Py_INCREF(&FrameType);
    if (PyModule_AddObject(m, "Frame", (PyObject *)&FrameType) < 0)
    {
        Py_DECREF(&FrameType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}