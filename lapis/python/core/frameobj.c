#include "frameobj.h"

// __new__ method in Python
static PyObject *Frame_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Frame *self;

    self = (Frame *)type->tp_alloc(type, 0);

    if (self != NULL)
    {
        // TODO: Required initialization
        self->_frame = NULL;
        self->_row_indexes = NULL;
        self->_col_indexes = NULL;
        self->_num_rows = 0;
        self->_num_cols = 0;
        self->_mem_size = 0;
        self->_is_transposed = 0;
    }

    return (PyObject *)self;
}

// __init__ method in Python
static int Frame_Init(Frame *self, PyObject *args, PyObject *kwds)
{
    // TODO: Parse args passed to the constructor
    return 0;
}

static PyObject *Frame_Create(PyObject *data)
{
    if (!PyList_Check(data))
    {
        PyErr_SetString(PyExc_TypeError, "Data must be a List");
        return NULL;
    }

    size_t rows = PyList_GET_SIZE(data);
    if (rows == 0)
    {
        PyErr_SetString(PyExc_ValueError, "Data must not be empty");
        return NULL;
    }

    PyObject *first_row = PyList_GetItem(data, 0);
    if (!PyList_Check(first_row))
    {
        PyErr_SetString(PyExc_TypeError, "Data must be a List of Lists");
        return NULL;
    }

    size_t cols = PyList_GET_SIZE(first_row);

    Frame *self = (Frame *)Frame_New(&FrameType, NULL, NULL);
    if (self == NULL)
    {
        return NULL;
    }

    frame *f = frame_init(cols, NULL, MMAPPED, 0);
    if (f == NULL)
    {
        Py_DECREF(self);
        PyErr_SetString(PyExc_RuntimeError, "Error creating DataFrame");
        return NULL;
    }

    for (size_t i = 0; i < rows; i++)
    {
        PyObject *row = PyList_GetItem(data, i);
        if (!PyList_Check(row) || PyList_GET_SIZE(row) != cols)
        {
            frame_free(f);
            Py_DECREF(self);
            PyErr_SetString(PyExc_ValueError, "All rows must have same number of columns");
            return NULL;
        }

        for (size_t j = 0; j < cols; j++)
        {
            PyObject *field = PyList_GetItem(row, j);
            if (!PyUnicode_Check(field))
            {
                frame_free(f);
                Py_DECREF(self);
                PyErr_SetString(PyExc_TypeError, "All fields must be strings");
                return NULL;
            }

            const char *field_str = PyUnicode_AsUTF8(field);
            if (field_str == NULL)
            {
                frame_free(f);
                Py_DECREF(self);
                PyErr_SetString(PyExc_RuntimeError, "Error converting field to string");
                return NULL;
            }

            if (dynamic_array_push(f->storage->data.cols[j], &field_str) != 0)
            {
                frame_free(f);
                Py_DECREF(self);
                PyErr_SetString(PyExc_RuntimeError, "Error adding field to DataFrame");
                return NULL;
            }
        }
        f->rows++;
    }

    self->_frame = f;
    return (PyObject *)self;
}

static PyObject *Frame_GetView(PyObject *self, size_t *row_indexes, size_t *col_indexes, size_t num_rows, size_t num_cols)
{
    Frame *frame = (Frame *)self;
    Frame *view = (Frame *)Frame_New(&FrameType, NULL, NULL);
    if (view == NULL)
    {
        return NULL;
    }

    // We will hold the same storage type as parent
    // If write operation is performed on view, it will be converted to IN_MEMORY
    lp_storage_type storage_type = frame->_frame->storage->type;

    view->_frame = frame_init(num_cols, NULL, storage_type, 1);
    if (view->_frame == NULL)
    {
        Py_DECREF(view);
        PyErr_SetString(PyExc_RuntimeError, "Error creating view");
        return NULL;
    }

    view->_mem_size = frame_mem_used(view->_frame);
    view->_frame->storage = frame->_frame->storage; // Share storage with parent frame
    frame->_frame->storage->ref_count++;            // Increment reference count

    view->_frame->rows = num_rows;
    view->_frame->cols = num_cols;
    view->_frame->_is_view = 1;

    view->_num_rows = num_rows;
    view->_num_cols = num_cols;
    view->_row_indexes = row_indexes;
    view->_col_indexes = col_indexes;

    return (PyObject *)view;
}

static size_t Frame_GetLen(PyObject *self)
{
    return ((Frame *)self)->_frame->rows - 1;
}

// Getter for num_rows
static PyObject *Frame_GetNumRows(Frame *self, void *closure)
{
    if (self->_frame == NULL)
    {
        Py_RETURN_NONE;
    }
    return PyLong_FromSize_t(self->_frame->rows - 1);
}

// Setter for num_rows
static int Frame_SetNumRows(Frame *self, PyObject *value, void *closure)
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
static PyObject *Frame_GetMemSize(Frame *self, void *closure)
{
    if (self->_frame == NULL)
    {
        Py_RETURN_NONE;
    }
    return PyLong_FromSize_t(frame_mem_used(self->_frame));
}

// TODO: Implement this properly
static PyObject *Frame_Str(PyObject *self)
{
    char *str = frame_str(((Frame *)self)->_frame);
    if (str == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "Error converting Frame to string");
        return NULL;
    }
    PyObject *str_obj = PyUnicode_FromString(str);
    free(str);
    return str_obj;
}

static PyObject *Frame_GetRow(Frame *self, PyObject *args)
{
    size_t row;
    if (!PyArg_ParseTuple(args, "n", &row))
    {
        return NULL;
    }

    short is_view = self->_frame->_is_view;

    // Calculate actual row index to return
    if (is_view && self->_row_indexes != NULL)
    {
        row = self->_row_indexes[row];
    }

    frame *f = self->_frame;
    size_t cols;
    size_t rows;

    if (is_view)
    {
        cols = self->_num_cols;
        rows = self->_num_rows;
    }
    else
    {
        cols = f->cols;
        rows = f->rows;
    }

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
        // Calculate actual column index to return
        size_t col = i;
        if (is_view && self->_col_indexes != NULL)
        {
            col = self->_col_indexes[col];
        }

        lp_ffield_t *field = frame_get(f, row, col);
        if (field == NULL)
        {
            PyErr_SetString(PyExc_RuntimeError, "Error getting field");
            return NULL;
        }

        if (field->quoted)
        {
            PyObject *field_obj = PyUnicode_FromStringAndSize(buffer + field->start + 1, (field->end - field->start - 2 > 0) ? field->end - field->start - 2 : 0);
            PyList_SetItem(list, i, field_obj);
            continue;
        }
        PyObject *field_obj = PyUnicode_FromStringAndSize(buffer + field->start, (field->end - field->start) > 0 ? field->end - field->start : 0);
        PyList_SetItem(list, i, field_obj);
    }

    return list;
}

// __getitem__ method in Python
// TODO: Refactor this, too many if-else
static PyObject *Frame_GetItem(Frame *self, PyObject *key)
{
    // [row_slice, col_slice]
    // Always return a view
    if (PySlice_Check(key))
    {
        Py_ssize_t start, stop, step;
        Py_ssize_t slicelength;
        if (PySlice_GetIndicesEx(key, self->_frame->rows - 1, &start, &stop, &step, &slicelength) == -1)
        {
            return NULL;
        }

        size_t num_rows = slicelength;
        size_t num_cols = self->_frame->cols;

        size_t *row_indexes = (size_t *)malloc(num_rows * sizeof(size_t));
        if (row_indexes == NULL)
        {
            PyErr_SetString(PyExc_MemoryError, "Error allocating memory for row indexes");
            return NULL;
        }

        for (size_t i = 0; i < num_rows; i++)
        {
            row_indexes[i] = start + i;
        }

        return Frame_GetView((PyObject *)self, row_indexes, NULL, num_rows, num_cols);
    }

    if (PyLong_Check(key))
    {
        size_t row = PyLong_AsSize_t(key);
        if (row >= self->_frame->rows - 1)
        {
            PyErr_SetString(PyExc_IndexError, "Index out of bounds");
            return NULL;
        }

        size_t num_rows = 1;
        size_t num_cols = self->_frame->cols;

        size_t *row_indexes = (size_t *)malloc(num_rows * sizeof(size_t));
        if (row_indexes == NULL)
        {
            PyErr_SetString(PyExc_MemoryError, "Error allocating memory for row indexes");
            return NULL;
        }

        row_indexes[0] = row;

        return Frame_GetView((PyObject *)self, row_indexes, NULL, num_rows, num_cols);
    }

    if (PyTuple_Check(key))
    {
        if (PyTuple_Size(key) != 2)
        {
            PyErr_SetString(PyExc_ValueError, "Invalid key");
            return NULL;
        }

        PyObject *row_obj = PyTuple_GetItem(key, 0);
        PyObject *col_obj = PyTuple_GetItem(key, 1);

        if (PySlice_Check(row_obj) && PySlice_Check(col_obj))
        {
            Py_ssize_t row_start, row_stop, row_step;
            Py_ssize_t row_slicelength;
            if (PySlice_GetIndicesEx(row_obj, self->_frame->rows - 1, &row_start, &row_stop, &row_step, &row_slicelength) == -1)
            {
                return NULL;
            }

            size_t num_rows = row_slicelength;

            Py_ssize_t col_start, col_stop, col_step;
            Py_ssize_t col_slicelength;
            if (PySlice_GetIndicesEx(col_obj, self->_frame->cols, &col_start, &col_stop, &col_step, &col_slicelength) == -1)
            {
                return NULL;
            }

            size_t num_cols = col_slicelength;

            size_t *row_indexes = (size_t *)malloc(num_rows * sizeof(size_t));
            if (row_indexes == NULL)
            {
                PyErr_SetString(PyExc_MemoryError, "Error allocating memory for row indexes");
                return NULL;
            }

            for (size_t i = 0; i < num_rows; i++)
            {
                row_indexes[i] = row_start + i;
            }

            size_t *col_indexes = (size_t *)malloc(num_cols * sizeof(size_t));
            if (col_indexes == NULL)
            {
                PyErr_SetString(PyExc_MemoryError, "Error allocating memory for col indexes");
                return NULL;
            }

            for (size_t i = 0; i < num_cols; i++)
            {
                col_indexes[i] = col_start + i;
            }

            return Frame_GetView((PyObject *)self, row_indexes, col_indexes, num_rows, num_cols);
        }

        if (PySlice_Check(row_obj) && PyLong_Check(col_obj))
        {
            Py_ssize_t start, stop, step;
            Py_ssize_t slicelength;
            if (PySlice_GetIndicesEx(row_obj, self->_frame->rows - 1, &start, &stop, &step, &slicelength) == -1)
            {
                return NULL;
            }

            size_t num_rows = slicelength;

            size_t col = PyLong_AsSize_t(col_obj);
            if (col >= self->_frame->cols)
            {
                PyErr_SetString(PyExc_IndexError, "Index out of bounds");
                return NULL;
            }

            size_t num_cols = 1;

            size_t *row_indexes = (size_t *)malloc(num_rows * sizeof(size_t));
            if (row_indexes == NULL)
            {
                PyErr_SetString(PyExc_MemoryError, "Error allocating memory for row indexes");
                return NULL;
            }

            for (size_t i = 0; i < num_rows; i++)
            {
                row_indexes[i] = start + i;
            }

            size_t *col_indexes = (size_t *)malloc(num_cols * sizeof(size_t));
            if (col_indexes == NULL)
            {
                PyErr_SetString(PyExc_MemoryError, "Error allocating memory for col indexes");
                return NULL;
            }

            col_indexes[0] = col;

            return Frame_GetView((PyObject *)self, row_indexes, col_indexes, num_rows, num_cols);
        }

        if (PyLong_Check(row_obj) && PySlice_Check(col_obj))
        {
            size_t row = PyLong_AsSize_t(row_obj);
            if (row >= self->_frame->rows - 1)
            {
                PyErr_SetString(PyExc_IndexError, "Index out of bounds");
                return NULL;
            }

            size_t num_rows = 1;

            Py_ssize_t start, stop, step;
            Py_ssize_t slicelength;
            if (PySlice_GetIndicesEx(col_obj, self->_frame->cols, &start, &stop, &step, &slicelength) == -1)
            {
                return NULL;
            }

            size_t num_cols = slicelength;

            size_t *row_indexes = (size_t *)malloc(num_rows * sizeof(size_t));
            if (row_indexes == NULL)
            {
                PyErr_SetString(PyExc_MemoryError, "Error allocating memory for row indexes");
                return NULL;
            }

            row_indexes[0] = row;

            size_t *col_indexes = (size_t *)malloc(num_cols * sizeof(size_t));
            if (col_indexes == NULL)
            {
                PyErr_SetString(PyExc_MemoryError, "Error allocating memory for col indexes");
                return NULL;
            }

            for (size_t i = 0; i < num_cols; i++)
            {
                col_indexes[i] = start + i;
            }

            return Frame_GetView((PyObject *)self, row_indexes, col_indexes, num_rows, num_cols);
        }

        if (PyLong_Check(row_obj) && PyLong_Check(col_obj))
        {
            size_t row = PyLong_AsSize_t(row_obj);
            size_t col = PyLong_AsSize_t(col_obj);

            if (row >= self->_frame->rows - 1 || col >= self->_frame->cols)
            {
                PyErr_SetString(PyExc_IndexError, "Index out of bounds");
                return NULL;
            }

            size_t num_rows = 1;
            size_t num_cols = 1;

            size_t *row_indexes = (size_t *)malloc(num_rows * sizeof(size_t));
            if (row_indexes == NULL)
            {
                PyErr_SetString(PyExc_MemoryError, "Error allocating memory for row indexes");
                return NULL;
            }

            row_indexes[0] = row;

            size_t *col_indexes = (size_t *)malloc(num_cols * sizeof(size_t));
            if (col_indexes == NULL)
            {
                PyErr_SetString(PyExc_MemoryError, "Error allocating memory for col indexes");
                return NULL;
            }

            col_indexes[0] = col;

            return Frame_GetView((PyObject *)self, row_indexes, col_indexes, num_rows, num_cols);
        }

        PyErr_SetString(PyExc_TypeError, "Invalid key");
        return NULL;
    }

    PyErr_SetString(PyExc_TypeError, "Invalid key");
    return NULL;
}

static PyObject *Frame_Free(Frame *self)
{
    frame_free(self->_frame);

    short is_view = self->_frame->_is_view;
    if (is_view)
    {
        free(self->_row_indexes);
        free(self->_col_indexes);
    }

    Py_TYPE(self)->tp_free((PyObject *)self);
    Py_RETURN_NONE;
}

static PySequenceMethods Frame_sequence_methods = {
    .sq_concat = NULL,                  /* concat */
    .sq_repeat = NULL,                  /* repeat */
    .was_sq_slice = NULL,               /* slice */
    .sq_ass_item = NULL,                /* assign item */
    .was_sq_ass_slice = NULL,           /* assign slice */
    .sq_contains = NULL,                /* contains */
    .sq_inplace_concat = NULL,          /* inplace concat */
    .sq_inplace_repeat = NULL,          /* inplace repeat */
    .sq_length = (lenfunc)Frame_GetLen, /* length */
    // .sq_item = (ssizeargfunc)NULL, /* item */
};

static PyGetSetDef FrameGetSet[] = {
    {"num_rows", (getter)Frame_GetNumRows, (setter)Frame_SetNumRows, "Number of rows", NULL},
    {"mem_size", (getter)Frame_GetMemSize, NULL, "Memory used", NULL},
    {NULL} // Sentinel
};

static PyMethodDef FrameMethods[] = {
    {"get", (PyCFunction)Frame_GetRow, METH_VARARGS, "Get a row"},
    {NULL, NULL, 0, NULL}};

// Frame type definition
PyTypeObject FrameType = {
    PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "lapis.DataFrame",
    .tp_basicsize = sizeof(Frame),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "DataFrame object for storing tabular data.",
    .tp_getset = FrameGetSet,
    .tp_new = Frame_New,
    .tp_init = (initproc)Frame_Init,
    .tp_str = Frame_Str,
    .tp_dealloc = (destructor)Frame_Free,
    .tp_methods = FrameMethods,
    .tp_as_sequence = &Frame_sequence_methods,
    .tp_as_mapping = &(PyMappingMethods){
        .mp_length = NULL,
        .mp_subscript = (binaryfunc)Frame_GetItem, // __getitem__ implementation
    },
};
