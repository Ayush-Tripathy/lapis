#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef enum
{
    INT,
    FLOAT,
    STRING
} type;

typedef struct
{
    char *buffer;
    size_t start;
    size_t end;
    bool quoted;
    type dtype;
} ffield_t;

typedef struct field_series_t
{
    size_t size;
    size_t capacity;
    ffield_t *fields;
    char *name;
    size_t _mem_used;
} field_series_t;

field_series_t *field_series_init(char *name);
void field_series_free(field_series_t *series);
bool field_series_append(field_series_t *series, ffield_t field);
ffield_t field_series_get(field_series_t *series, size_t index);
size_t field_series_mem_used(field_series_t *series);
