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
    lp_storage_t *storage;
    lp_bool _is_view;
} frame;

frame *frame_init(size_t cols, char **col_names, lp_storage_type type, lp_bool is_view);
void frame_free(frame *f);
lp_ffield_t *frame_get(frame *f, size_t row, size_t col);
size_t frame_mem_used(frame *f);
char *frame_str(frame *f);

#endif // FRAME_H
