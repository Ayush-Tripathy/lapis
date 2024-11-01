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
#include "logger.h"

typedef struct
{
  size_t cols;
  size_t rows;
  size_t _mem_used;
  lp_storage_t *storage;
  lp_bool _is_view;
  lp_bool _is_transposed;
  size_t *_row_indexes;
  size_t *_col_indexes;
  lp_bool has_header;
} frame;

frame *frame_init(size_t cols, char **col_names, lp_storage_type type, lp_bool has_header, lp_bool is_view);
void frame_free(frame *f);
lp_field_t *frame_get(frame *f, size_t row, size_t col);
size_t frame_mem_used(frame *f);
char *frame_str(frame *f);
lp_string frame_get_col_name(frame *f, size_t col);
lp_size_t frame_get_col_index(frame *f, const char *col_name);
lp_shape frame_get_shape(frame *f);
static lp_string frame_get_value_as_string(frame *f, size_t row, size_t col);

#endif // FRAME_H
