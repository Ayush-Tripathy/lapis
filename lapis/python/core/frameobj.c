#include "frameobj.h"

// __new__ method in Python
static PyObject *FrameNew(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Frame *self;

    self = (Frame *)type->tp_alloc(type, 0);

    if (self != NULL)
    {
        // TODO: Required initialization
    }

    return (PyObject *)self;
}

// __init__ method in Python
static int Frame_init(Frame *self, PyObject *args, PyObject *kwds)
{
    // TODO: Parse args passed to the constructor
    return 0;
}

static size_t Frame_get_len(PyObject *self)
{
    return ((Frame *)self)->_frame->rows - 1;
}

static PyObject *Frame_get_item(PyObject *self, Py_ssize_t i)
{
    // if (i < 0 || i >= Py_SIZE(self)) {
    //     PyErr_SetString(PyExc_IndexError, "index out of range");
    //     return NULL;
    // }

    size_t row = (size_t)i;

    frame *f = ((Frame *)self)->_frame;
    size_t cols = f->cols;
    size_t rows = f->rows;
    char *buffer;
    if (f->storage->type == MMAPPED)
    {
        buffer = f->storage->handle.mmapped->buffer;
    }

    if (((Frame *)self)->_frame == NULL || row >= rows)
    {
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return NULL;
    }

    PyObject *list = PyList_New(cols);
    if (list == NULL)
    {
        return NULL;
    }

    for (size_t i = 0; i < cols; i++)
    {
        ffield_t *field = frame_get(f, row, i);
        if (field == NULL)
        {
            PyErr_SetString(PyExc_RuntimeError, "Error getting field");
            return NULL;
        }

        if (field->quoted)
        {
            PyObject *field_obj = PyUnicode_FromStringAndSize(buffer + field->start + 1, field->end - field->start - 2);
            PyList_SetItem(list, i, field_obj);
            continue;
        }
        PyObject *field_obj = PyUnicode_FromStringAndSize(buffer + field->start, field->end - field->start);
        PyList_SetItem(list, i, field_obj);
    }

    // Py_INCREF(list);
    return list;
}

// Getter for num_rows
static PyObject *Frame_get_num_rows(Frame *self, void *closure)
{
    if (self->_frame == NULL)
    {
        Py_RETURN_NONE;
    }
    return PyLong_FromSize_t(self->_frame->rows - 1);
}

// Setter for num_rows
static int Frame_set_num_rows(Frame *self, PyObject *value, void *closure)
{
    if (self->_frame == NULL || !PyLong_Check(value))
    {
        PyErr_SetString(PyExc_TypeError, "The value must be an integer");
        return -1;
    }
    self->_frame->rows = PyLong_AsSize_t(value);
    return 0;
}

// Getter for mem_size
static PyObject *Frame_get_mem_size(Frame *self, void *closure)
{
    if (self->_frame == NULL)
    {
        Py_RETURN_NONE;
    }
    return PyLong_FromSize_t(frame_mem_used(self->_frame));
}

// TODO: Implement this properly
static PyObject *FrameStr(PyObject *self)
{
    // return PyUnicode_FromFormat("Frame object with %ld rows", ((Frame *)self)->_frame->rows);

    // Pretty print the table with first and last 3 rows
    frame *f = ((Frame *)self)->_frame;
    size_t cols = f->cols;
    size_t rows = f->rows;
    char *buffer;
    if (f->storage->type == MMAPPED)
    {
        buffer = f->storage->handle.mmapped->buffer;
    }

    // If empty return pretty string with headers
    if (rows == 0)
    {
        char *header = (char *)malloc(1);
        header[0] = '\0';
        for (size_t i = 0; i < cols; i++)
        {
            ffield_t *field = frame_get(f, 0, i);
            size_t len = field->end - field->start;
            header = (char *)realloc(header, strlen(header) + len + 2);
            strncat(header, buffer + field->start, len);
            strcat(header, ",");
        }
        header[strlen(header) - 1] = '\0';
        char *pretty = (char *)malloc(strlen(header) + 3);
        sprintf(pretty, "%s\n%s\n%s", header, "----", header);
        return PyUnicode_FromString(pretty);
    }

    // return pretty table with first and last 3 rows
    // example - |------|------|------|
    //           | col1 | col2 | col3 |
    //           |------|------|------|
    //           | val1 | val2 | val3 |
    //           | val4 | val5 | val6 |
    //           | val7 | val8 | val9 |
    //           |------|------|------|

    short max_chars_field = 30; // More field length will be truncated with 27 characters and ...
    size_t num_rows = 6;
    size_t num_cols = cols > 6 ? cols : cols;
    size_t num_fields = num_rows * num_cols;
    size_t num_chars_in_line = num_fields * max_chars_field + num_cols + 2;                            // 2 (1 for newline and 1 for last divider)
    size_t num_of_dividers = 3;                                                                        // Dividers mean |------|------|------| 1st row, 3rd row and last row
    size_t num_chars_total = num_chars_in_line + num_cols + (num_of_dividers * num_chars_in_line) + 4; // 4 for  divider of first 3 rows and last 3 rows

    char *pretty = (char *)malloc(num_chars_total);
    pretty[0] = '\0';

    // Add Divider
    for (size_t i = 0; i < num_chars_in_line % 100; i++)
    {
        strcat(pretty, "-");
    }
    strcat(pretty, "\n");

    // Add header like | col1 | col2 | col3 |, limit cols to 6
    for (size_t i = 0; i < num_cols; i++)
    {
        ffield_t *field = frame_get(f, 0, i);
        size_t len = field->end - field->start;
        char *header = (char *)malloc(len + 3);
        // sprintf(header, "| %.*s ", len, buffer + field->start);
        sprintf(header, "| %.*s", (int)len, buffer + field->start);
        strcat(pretty, header);
    }
    strcat(pretty, "|\n");

    // Add Divider
    for (size_t i = 0; i < num_chars_in_line % 100; i++)
    {
        strcat(pretty, "-");
    }
    strcat(pretty, "\n");

    // Add first 3 rows like | val1 | val2 | val3 |
    for (size_t i = 0; i < 3; i++)
    {
        for (size_t j = 0; j < num_cols; j++)
        {
            ffield_t *field = frame_get(f, i, j);
            size_t len = field->end - field->start;
            // Truncate if length is more than max_chars_field 27 + ...
            if (len > max_chars_field)
            {
                len = max_chars_field - 3;
            }

            char *field_str = (char *)malloc(len + 3);
            // sprintf(field_str, "| %.*s ", len, buffer + field->start);
            sprintf(field_str, "| %.*s", (int)len, buffer + field->start);
            strcat(pretty, field_str);
            free(field_str);
        }
        strcat(pretty, "|\n");
    }

    // Add rows divider (...)
    for (size_t i = 0; i < 3; i++)
    {
        strcat(pretty, ".");
    }
    strcat(pretty, "\n");

    // Add last 3 rows like | val7 | val8 | val9 |
    for (size_t i = rows - 3; i < rows; i++)
    {
        for (size_t j = 0; j < num_cols; j++)
        {
            ffield_t *field = frame_get(f, i, j);
            size_t len = field->end - field->start;
            // Truncate if length is more than max_chars_field 27 + ...
            if (len > max_chars_field)
            {
                len = max_chars_field - 3;
            }

            char *field_str = (char *)malloc(len + 3);
            // sprintf(field_str, "| %.*s ", len, buffer + field->start);
            sprintf(field_str, "| %.*s", (int)len, buffer + field->start);
            strcat(pretty, field_str);
            free(field_str);
        }
        strcat(pretty, "|\n");
    }

    // Add Divider
    for (size_t i = 0; i < num_chars_in_line % 100; i++)
    {
        strcat(pretty, "-");
    }
    strcat(pretty, "\n");

    PyObject *pretty_obj = PyUnicode_FromString(pretty);
    free(pretty);
    return pretty_obj;
}

static PyObject *Frame_get_row(Frame *self, PyObject *args)
{
    size_t row;
    if (!PyArg_ParseTuple(args, "n", &row))
    {
        return NULL;
    }

    frame *f = self->_frame;
    size_t cols = f->cols;
    size_t rows = f->rows;
    char *buffer;
    if (f->storage->type == MMAPPED)
    {
        buffer = f->storage->handle.mmapped->buffer;
    }

    if (self->_frame == NULL || row >= rows)
    {
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return NULL;
    }

    PyObject *list = PyList_New(cols);
    if (list == NULL)
    {
        return NULL;
    }

    for (size_t i = 0; i < cols; i++)
    {
        ffield_t *field = frame_get(f, row, i);
        if (field == NULL)
        {
            PyErr_SetString(PyExc_RuntimeError, "Error getting field");
            return NULL;
        }

        if (field->quoted)
        {
            PyObject *field_obj = PyUnicode_FromStringAndSize(buffer + field->start + 1, field->end - field->start - 2);
            PyList_SetItem(list, i, field_obj);
            continue;
        }
        PyObject *field_obj = PyUnicode_FromStringAndSize(buffer + field->start, field->end - field->start);
        PyList_SetItem(list, i, field_obj);
    }

    return list;
}

static PyObject *Frame_free(Frame *self)
{
    frame_free(self->_frame);
    Py_TYPE(self)->tp_free((PyObject *)self);
    Py_RETURN_NONE;
}

static PySequenceMethods Frame_sequence_methods = {
    .sq_concat = NULL,                       /* concat */
    .sq_repeat = NULL,                       /* repeat */
    .was_sq_slice = NULL,                    /* slice */
    .sq_ass_item = NULL,                     /* assign item */
    .was_sq_ass_slice = NULL,                /* assign slice */
    .sq_contains = NULL,                     /* contains */
    .sq_inplace_concat = NULL,               /* inplace concat */
    .sq_inplace_repeat = NULL,               /* inplace repeat */
    .sq_length = (lenfunc)Frame_get_len,     /* length */
    .sq_item = (ssizeargfunc)Frame_get_item, /* item */
};

static PyGetSetDef FrameGetSet[] = {
    {"num_rows", (getter)Frame_get_num_rows, (setter)Frame_set_num_rows, "Number of rows", NULL},
    {"mem_size", (getter)Frame_get_mem_size, NULL, "Memory used", NULL},
    {NULL} // Sentinel
};

static PyMethodDef FrameMethods[] = {
    {"get", (PyCFunction)Frame_get_row, METH_VARARGS, "Get a row"},
    {NULL, NULL, 0, NULL}};

// Frame type definition
PyTypeObject FrameType = {
    PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "lapis.Frame",
    .tp_basicsize = sizeof(Frame),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "Frame object for storing tabular data.",
    .tp_getset = FrameGetSet,
    .tp_new = FrameNew,
    .tp_init = (initproc)Frame_init,
    .tp_str = FrameStr,
    .tp_dealloc = (destructor)Frame_free,
    .tp_methods = FrameMethods,
    .tp_as_sequence = &Frame_sequence_methods,
};
