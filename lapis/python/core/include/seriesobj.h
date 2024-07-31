#include <Python.h>

#include "series.h"

typedef struct Series
{
    PyObject_HEAD field_series_t *_series;
} Series;

extern PyTypeObject SeriesType;

static PyObject *Series_New(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int Series_Init(Series *self, PyObject *args, PyObject *kwds);
static void Series_Free(Series *self);
static PyObject *Series_Str(Series *self);
static Py_ssize_t Series_Get_Len(Series *self);
