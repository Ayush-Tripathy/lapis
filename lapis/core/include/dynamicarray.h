#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logger.h"

typedef struct dynamic_array
{
  size_t size;
  size_t capacity;
  void *data;
  size_t element_size;
} dynamic_array;

dynamic_array *dynamic_array_init(size_t capacity, size_t element_size);
void dynamic_array_free(dynamic_array *array);
short dynamic_array_push(dynamic_array *array, void *element);
void *dynamic_array_get(dynamic_array *array, size_t index);
size_t dynamic_array_size(dynamic_array *array);
size_t dynamic_array_capacity(dynamic_array *array);
void dynamic_array_resize(dynamic_array *array, size_t new_capacity);
void *dynamic_array_set(dynamic_array *array, size_t index, void *element);
dynamic_array *dynamic_array_copy(dynamic_array *src);
void *dynamic_array_get_copy(dynamic_array *array, size_t index);
