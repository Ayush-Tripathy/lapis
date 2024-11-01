#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "storage.h"
#include "logger.h"

typedef struct field_series_t
{
  size_t size;
  size_t capacity;
  lp_storage_t *storage;
  char *name;
  size_t _mem_used;
} field_series_t;

field_series_t *field_series_init(char *name, lp_storage_type type);
void field_series_free(field_series_t *series);
bool field_series_append(field_series_t *series, lp_field_t field, lp_field_t mfield);
lp_field_t field_series_get(field_series_t *series, size_t index);
size_t field_series_mem_used(field_series_t *series);
