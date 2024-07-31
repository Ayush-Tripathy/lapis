#pragma once

#include <Python.h>
#include <structmember.h>

#include "frame.h"

typedef struct Frame
{
    PyObject_HEAD frame *_frame;
} Frame;

extern PyTypeObject FrameType;

static int Frame_init(Frame *self, PyObject *args, PyObject *kwds);
static PyObject *FrameNew(PyTypeObject *type, PyObject *args, PyObject *kwds);
static PyObject *Frame_get_num_rows(Frame *self, void *closure);
static int Frame_set_num_rows(Frame *self, PyObject *value, void *closure);
static PyObject *Frame_get_mem_size(Frame *self, void *closure);
static PyObject *Frame_get_row(Frame *self, PyObject *args);
static PyObject *Frame_get_item(PyObject *self, Py_ssize_t i);
static size_t Frame_get_len(PyObject *self);
