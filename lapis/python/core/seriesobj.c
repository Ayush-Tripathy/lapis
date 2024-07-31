#include "seriesobj.h"

// __new__ method in Python
static PyObject *Series_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Series *self = (Series *)type->tp_alloc(type, 0);
    if (self != NULL)
    {
        // TODO: Required initialization
    }
    return (PyObject *)self;
}

// __init__ method in Python
static int Series_Init(Series *self, PyObject *args, PyObject *kwds)
{
    // TODO: Parse args passed to the constructor
    return 0;
}

// destructor
static void Series_Free(Series *self)
{
    field_series_free(self->_series);
    Py_TYPE(self)->tp_free((PyObject *)self);
    Py_RETURN_NONE;
}

// __str__ method in Python
static PyObject *Series_Str(Series *self)
{
    return PyUnicode_FromFormat("<Series object at %p>: size=%ld", self, ((field_series_t *)self)->size);
}

// __len__ method in Python
static Py_ssize_t Series_Get_Len(Series *self)
{
    return ((field_series_t *)self)->size;
}

// __getitem__ method in Python
static PyObject *Series_Get_Item(Series *self, Py_ssize_t i)
{
    if (i < 0 || i >= ((field_series_t *)self)->size)
    {
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return NULL;
    }

    ffield_t field = field_series_get(self->_series, i);
    PyObject *value = NULL;
    switch (field.dtype)
    {
    case INT:
        value = PyLong_FromString(
            PyUnicode_AsUTF8(
                PyUnicode_FromStringAndSize(field.buffer + field.start, field.end - field.start)),
            NULL, 10);
        break;
    case FLOAT:
        value = PyFloat_FromString(
            PyUnicode_FromStringAndSize(field.buffer + field.start, field.end - field.start));
        break;
    case STRING:
        value = PyUnicode_FromStringAndSize(field.buffer + field.start, field.end - field.start);
        break;
    default:
        PyErr_SetString(PyExc_TypeError, "Invalid data type");
        return NULL;
    }

    return value;
}

static PySequenceMethods Series_Sequence_Methods = {
    .sq_concat = NULL,                        /* concat */
    .sq_repeat = NULL,                        /* repeat */
    .was_sq_slice = NULL,                     /* slice */
    .sq_ass_item = NULL,                      /* assign item */
    .was_sq_ass_slice = NULL,                 /* assign slice */
    .sq_contains = NULL,                      /* contains */
    .sq_inplace_concat = NULL,                /* inplace concat */
    .sq_inplace_repeat = NULL,                /* inplace repeat */
    .sq_length = (lenfunc)Series_Get_Len,     /* length */
    .sq_item = (ssizeargfunc)Series_Get_Item, /* item */
};

// static PyGetSetDef Series_GetSet[] = {
//     {"num_fields", (getter)Series_Get_NumFields, NULL, "Number of fields", NULL},
//     {"mem_size", (getter)Series_Get_MemSize, NULL, "Memory used", NULL},
//     {NULL} // Sentinel
// };

// static PyMethodDef SeriesMethods[] = {
//     {"get", (PyCFunction)Series_get_row, METH_VARARGS, "Get a field"},
//     {NULL, NULL, 0, NULL}};

// Series type definition
PyTypeObject SeriesType = {
    PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "lapis.Series",
    .tp_basicsize = sizeof(Series),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "Series object for storing linear data.",
    // .tp_getset = Series_GetSet,
    .tp_new = Series_New,
    .tp_init = (initproc)Series_Init,
    .tp_str = Series_Str,
    .tp_dealloc = (destructor)Series_Free,
    // .tp_methods = SeriesMethods,
    .tp_as_sequence = &Series_Sequence_Methods,
};