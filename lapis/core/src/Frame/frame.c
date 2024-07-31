/*
 * FILE: frame.c
 *
 * 2 Dimensional dynamic array
 * Collection of field_series_t
 */

#include "frame.h"

frame *frame_init(size_t cols, char **col_names)
{
    frame *f = (frame *)malloc(sizeof(frame));
    if (f == NULL)
        return NULL;

    f->cols = cols;
    f->rows = 0;
    f->_mem_used = sizeof(frame);

    f->series = (field_series_t **)malloc(cols * sizeof(field_series_t));
    f->_mem_used += (cols * sizeof(field_series_t));

    for (size_t i = 0; i < cols; i++)
    {
        if (f->series == NULL)
        {
            free(f);
            return NULL;
        }

        f->series[i] = field_series_init(col_names[i]);

        if ((f->series[i]) == NULL)
        {
            free(f->series);
            free(f);
            return NULL;
        }
    }

    return f;
}

size_t frame_mem_used(frame *f)
{
    if (f == NULL)
        return 0;

    size_t mem = f->_mem_used;
    size_t cols = f->cols;

    for (size_t i = 0; i < cols; i++)
    {
        mem += field_series_mem_used(f->series[i]);
    }

    return mem;
}

void frame_free(frame *f)
{
    if (f == NULL)
        return;

    for (size_t i = 0; i < f->cols; i++)
    {
        field_series_free(f->series[i]);
    }

    if (f->buffer != NULL)
        unmap_file(f->buffer, f->buffer_size);

    free(f->series);
    free(f);
}

bool frame_append(frame *f, ffield_t *fields)
{
    if (f == NULL)
        return false;

    if (f->rows >= f->series[0]->size)
    {
        for (size_t i = 0; i < f->cols; i++)
        {
            if (!field_series_append(f->series[i], fields[i]))
                return false;
        }
        f->rows++;
    }

    return true;
}

ffield_t *frame_get(frame *f, size_t row, size_t col)
{
    if (f == NULL || row >= f->rows || col >= f->cols)
        return NULL;

    return &(f->series[col]->fields[row]);
}