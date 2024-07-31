#include "functions.h"

PyObject *read_csv(PyObject *self, PyObject *args)
{
    const char *filename;
    const char *delim = ",";
    const char *quote_char = "\"";
    const char *comment_char = "#";
    const char *escape_char = "\\";

    if (!PyArg_ParseTuple(args, "s|ssss", &filename, &delim, &quote_char, &comment_char, &escape_char))
    {
        return NULL;
    }

    Frame *_frame = (Frame *)FrameType.tp_new(&FrameType, NULL, NULL);

    if (_frame == NULL)
    {
        return NULL;
    }

    _frame->_frame = _read_csv(filename, delim[0], quote_char[0], comment_char[0], escape_char[0]);

    if (_frame->_frame == NULL)
    {
        Py_DECREF(_frame);
        PyErr_SetString(PyExc_FileNotFoundError, "Error reading CSV file"); // TODO: Make this more informative
        return NULL;
    }

    return (PyObject *)_frame;
}