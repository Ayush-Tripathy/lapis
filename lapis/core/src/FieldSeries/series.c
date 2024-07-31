/*
 * FILE: series.c
 *
 * 1 Dimensional dynamic array
 */

#include "series.h"

#define SERIES_INITIAL_CAPACITY 1000

field_series_t *field_series_init(char *name)
{
    field_series_t *series = (field_series_t *)malloc(sizeof(field_series_t));
    series->_mem_used = sizeof(field_series_t);
    if (series == NULL)
        return NULL;

    series->size = 0;
    series->capacity = SERIES_INITIAL_CAPACITY;
    series->fields = (ffield_t *)malloc(series->capacity * sizeof(ffield_t));
    series->_mem_used += series->capacity * sizeof(ffield_t);
    if (series->fields == NULL)
    {
        free(series);
        return NULL;
    }

    size_t col_len = strlen(name) + 1;
    series->name = (char *)malloc(col_len);
    series->_mem_used += col_len;
    if (series->name == NULL)
    {
        free(series->fields);
        free(series);
        return NULL;
    }

    strcpy(series->name, name);

    return series;
}

void field_series_free(field_series_t *series)
{
    if (series == NULL)
        return;

    free(series->fields);
    free(series->name);
    free(series);
}

bool field_series_append(field_series_t *series, ffield_t field)
{
    if (series == NULL)
        return false;

    size_t capacity = series->capacity;
    if (series->size >= capacity)
    {
        capacity *= 2;
        ffield_t *new_fields = (ffield_t *)realloc(series->fields, capacity * sizeof(ffield_t));
        if (new_fields == NULL)
            return false;
        series->fields = new_fields;
    }

    series->fields[series->size++] = field;
    series->capacity = capacity;
    return true;
}

size_t field_series_mem_used(field_series_t *series)
{
    return series->_mem_used + (series->capacity * sizeof(ffield_t));
}

ffield_t field_series_get(field_series_t *series, size_t index)
{
    if (series == NULL || index >= series->size)
    {
        ffield_t field = {0, 0, false};
        return field;
    }

    return series->fields[index];
}
