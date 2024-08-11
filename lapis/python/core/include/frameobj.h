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
static PyObject *Frame_Create(PyObject *data);
static PyObject *Frame_GetNumRows(Frame *self, void *closure);
static int Frame_SetNumRows(Frame *self, PyObject *value, void *closure);
static PyObject *Frame_GetMemSize(Frame *self, void *closure);
static PyObject *Frame_GetRow(Frame *self, PyObject *args);
static size_t Frame_GetLen(PyObject *self);
