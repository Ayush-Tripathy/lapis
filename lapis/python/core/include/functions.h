#pragma once

#include <Python.h>

#include "frameobj.h"
#include "lpio.h"
#include "frame.h"

PyObject *read_csv(PyObject *self, PyObject *args, PyObject *kwargs);
