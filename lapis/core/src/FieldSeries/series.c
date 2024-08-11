/*
 * FILE: series.c
 *
 * 1 Dimensional dynamic array
 */

#include "series.h"

#define SERIES_INITIAL_CAPACITY 1000

field_series_t *field_series_init(char *name, lp_storage_type type)
{
    field_series_t *series = (field_series_t *)malloc(sizeof(field_series_t));
    series->_mem_used = sizeof(field_series_t);
    if (series == NULL)
        return NULL;

    series->size = 0;
    series->capacity = SERIES_INITIAL_CAPACITY;
    // series->fields = (ffield_t *)malloc(series->capacity * sizeof(ffield_t));

    lp_shape shape = {
        .rows = 1, // Initial size
        .cols = 1  // As it is a 1D array
    };

    series->storage = lp_storage_init(type, shape);
    if (series->storage == NULL)
    {
        free(series);
        return NULL;
    }

    series->_mem_used += lp_storage_get_mem_used(series->storage);

    // series->_mem_used += series->capacity * sizeof(ffield_t);
    // if (series->fields == NULL)
    // {
    //     free(series);
    //     return NULL;
    // }

    size_t col_len = strlen(name) + 1;
    series->name = (char *)malloc(col_len);
    series->_mem_used += col_len;
    if (series->name == NULL)
    {
        // free(series->fields);
        lp_storage_free(series->storage);
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

    // free(series->fields);
    lp_storage_free(series->storage);
    free(series->name);
    free(series);
}

bool field_series_append(field_series_t *series, lp_ffield_t field, lp_mfield_t mfield)
{
    if (series == NULL)
        return false;

    // size_t capacity = series->capacity;
    // if (series->size >= capacity)
    // {
    //     capacity *= 2;
    //     ffield_t *new_fields = (ffield_t *)realloc(series->fields, capacity * sizeof(ffield_t));
    //     if (new_fields == NULL)
    //         return false;
    //     series->fields = new_fields;
    // }

    // series->fields[series->size++] = field;
    // series->capacity = capacity;

    // lp_shape shape = lp_storage_get_shape(series->storage);

    if (series->storage->type == MMAPPED)
    {
        dynamic_array_push(series->storage->data.fields, &field);
    }
    else if (series->storage->type == IN_MEMORY)
    {
        dynamic_array_push(series->storage->data.fields, &mfield);
    }

    return true;
}

size_t field_series_mem_used(field_series_t *series)
{
    // return series->_mem_used + (series->capacity * sizeof(ffield_t));
    return series->_mem_used;
}

lp_field_t field_series_get(field_series_t *series, size_t index)
{
    if (series == NULL || index >= series->size)
    {
        // ffield_t field = {0, 0, false};
        // return field;
        return (lp_field_t){0, 0, false, INT};
    }

    // return series->fields[index];
    if (series->storage->type == MMAPPED)
    {
        lp_ffield_t field = *(lp_ffield_t *)dynamic_array_get(series->storage->data.fields, index);
        return (lp_field_t){
            .start = field.start,
            .end = field.end,
            .quoted = field.quoted,
            .dtype = field.dtype};
    }
    else if (series->storage->type == IN_MEMORY)
    {
        lp_mfield_t *field = ((lp_mfield_t *)dynamic_array_get(series->storage->data.fields, index));
        return (lp_field_t){
            .buffer = field->buffer,
            .quoted = field->quoted,
            .dtype = field->dtype};
    }
}
