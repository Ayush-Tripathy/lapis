#ifndef FRAME_H
#define FRAME_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "series.h"
#include "memmap.h"
#include "storage.h"

typedef struct
{
    size_t cols;
    size_t rows;
    size_t _mem_used;
    field_series_t **series;
    lp_storage_t *storage;
} frame;

frame *frame_init(size_t cols, char **col_names);
void frame_free(frame *f);
bool frame_append(frame *f, ffield_t *fields);
ffield_t *frame_get(frame *f, size_t row, size_t col);
size_t frame_mem_used(frame *f);

#endif // FRAME_H
