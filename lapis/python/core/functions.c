#include "functions.h"

PyObject *read_csv(PyObject *self, PyObject *args, PyObject *kwargs)
{
    const char *filename;
    const char *delim = ",";
    const char *quote_char = "\"";
    const char *comment_char = "#";
    const char *escape_char = "\\";
    lp_bool has_header = LP_TRUE;

    static char *kwlist[] = {"filename", "delim", "quote_char", "comment_char", "escape_char", "has_header", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|ssssp", kwlist, &filename, &delim, &quote_char, &comment_char, &escape_char, &has_header))
    {
        return NULL;
    }

    Frame *_frame = (Frame *)FrameType.tp_new(&FrameType, NULL, NULL);

    if (_frame == NULL)
    {
        return NULL;
    }

    _frame->_frame = _read_csv(filename, has_header, delim[0], quote_char[0], comment_char[0], escape_char[0]);

    if (_frame->_frame == NULL)
    {
        Py_DECREF(_frame);
        PyErr_SetString(PyExc_FileNotFoundError, "Error reading CSV file"); // TODO: Make this more informative
        return NULL;
    }

    return (PyObject *)_frame;
}