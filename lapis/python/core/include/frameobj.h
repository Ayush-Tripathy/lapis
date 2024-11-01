#pragma once

#include <Python.h>
#include <structmember.h>

#include "frame.h"

typedef struct Frame
{
  PyObject_HEAD frame *_frame;
} Frame;

extern PyTypeObject FrameType;

static int Frame_Init(Frame *self, PyObject *args, PyObject *kwds);
static PyObject *Frame_New(PyTypeObject *type, PyObject *args, PyObject *kwds);
frame *Frame_Create(PyObject *data, PyObject *columns);
frame *Frame_CreateFromList(PyObject *data, PyObject *columns);
frame *Frame_CreateFromDict(PyObject *data);
static PyObject *Frame_GetNumRows(Frame *self, void *closure);
static int Frame_SetNumRows(Frame *self, PyObject *value, void *closure);
static PyObject *Frame_GetMemSize(Frame *self, void *closure);
static PyObject *Frame_GetRow(Frame *self, PyObject *args);
static size_t Frame_GetLen(PyObject *self);
static PyObject *Frame_GetView(PyObject *self, size_t *row_indexes, size_t *col_indexes, size_t num_rows, size_t num_cols);
static PyObject *Frame_GetViewFromSlice(Frame *self, PyObject *key);
static PyObject *Frame_GetViewFromRowIndex(Frame *self, PyObject *key);
