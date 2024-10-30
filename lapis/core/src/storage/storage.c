#include "storage.h"

lp_storage_t *lp_storage_init(lp_storage_type type, lp_shape shape)
{
    lp_storage_t *storage = (lp_storage_t *)malloc(sizeof(lp_storage_t));
    if (storage == NULL)
    {
        return NULL;
    }

    storage->type = type;
    storage->_shape = shape;

    switch (type)
    {
    case MMAPPED:
        // Setup handle
        storage->handle.mmapped = (lp_mmapped_t *)malloc(sizeof(lp_mmapped_t));
        if (storage->handle.mmapped == NULL)
        {
            free(storage);
            return NULL;
        }
        storage->handle.mmapped->buffer = NULL;
        storage->handle.mmapped->buffer_size = 0;

        // Setup data storage
        if (shape.cols == 1) // 1D array
        {
            storage->data.fields = dynamic_array_init(shape.rows, sizeof(lp_field_t));
            if (storage->data.fields == NULL)
            {
                free(storage->handle.mmapped);
                free(storage);
                return NULL;
            }
        }
        else
        {
            storage->data.cols = (dynamic_array **)malloc(shape.cols * sizeof(dynamic_array *));
            if (storage->data.cols == NULL)
            {
                free(storage->handle.mmapped);
                free(storage);
                return NULL;
            }

            for (size_t i = 0; i < shape.cols; i++)
            {
                storage->data.cols[i] = dynamic_array_init(shape.rows, sizeof(lp_field_t));
                if (storage->data.cols[i] == NULL)
                {
                    for (size_t j = 0; j < i; j++)
                    {
                        dynamic_array_free(storage->data.cols[j]);
                    }
                    free(storage->data.cols);
                    free(storage->handle.mmapped);
                    free(storage);
                    return NULL;
                }
            }
        }
        break;

    case IN_MEMORY:
        // storage->handle.in_memory = (lp_in_memory_t *)malloc(sizeof(lp_field_t));
        // if (storage->handle.in_memory == NULL)
        // {
        //     free(storage);
        //     return NULL;
        // }

        if (shape.cols == 1) // 1D array
        {
            storage->data.fields = dynamic_array_init(shape.rows, sizeof(lp_field_t));
            if (storage->data.fields == NULL)
            {
                // free(storage->handle.in_memory);
                free(storage);
                return NULL;
            }
        }
        else
        {
            storage->data.cols = (dynamic_array **)malloc(shape.cols * sizeof(dynamic_array *));
            if (storage->data.cols == NULL)
            {
                // free(storage->handle.in_memory);
                free(storage);
                return NULL;
            }

            for (size_t i = 0; i < shape.cols; i++)
            {
                storage->data.cols[i] = dynamic_array_init(shape.rows, sizeof(lp_field_t));
                if (storage->data.cols[i] == NULL)
                {
                    for (size_t j = 0; j < i; j++)
                    {
                        dynamic_array_free(storage->data.cols[j]);
                    }
                    free(storage->data.cols);
                    // free(storage->handle.in_memory);
                    free(storage);
                    return NULL;
                }
            }
        }
        break;

    default:
        free(storage);
        return NULL;
    }

    storage->ref_count = 0;
    return storage;
}

void lp_storage_free(lp_storage_t *storage)
{
    if (storage == NULL)
    {
        return;
    }

    if (storage->ref_count > 0)
    {
        storage->ref_count--;
        return;
    }

    switch (storage->type)
    {
    case MMAPPED:
        free(storage->handle.mmapped);
        if (storage->data.cols != NULL)
        {
            for (size_t i = 0; i < storage->_shape.cols; i++)
            {
                dynamic_array_free(storage->data.cols[i]);
            }
            free(storage->data.cols);
        }
        else
        {
            dynamic_array_free(storage->data.fields);
        }

        // Unmap file
        if (storage->handle.mmapped->buffer != NULL)
        {
            unmap_file(storage->handle.mmapped->buffer, storage->handle.mmapped->buffer_size);
        }
        break;

    case IN_MEMORY:
        // if (storage->handle.in_memory != NULL)
        // {
        //     free(storage->handle.in_memory);
        // }

        if (storage->data.cols != NULL)
        {
            for (size_t i = 0; i < storage->_shape.cols; i++)
            {
                dynamic_array_free(storage->data.cols[i]);
            }
            free(storage->data.cols);
        }
        else
        {
            dynamic_array_free(storage->data.fields);
        }
        break;

    default:
        break;
    }

    free(storage);
}

lp_bool lp_storage_is_mmapped(lp_storage_t *storage)
{
    return storage->type == MMAPPED;
}

lp_bool lp_storage_is_in_memory(lp_storage_t *storage)
{
    return storage->type == IN_MEMORY;
}

lp_bool lp_storage_is_valid(lp_storage_t *storage)
{
    return storage != NULL;
}

lp_bool lp_storage_is_2d(lp_storage_t *storage)
{
    // if (storage->type == IN_MEMORY)
    // {
    //     return storage->handle.in_memory->is_2d;
    // }

    return 0;
}

lp_shape lp_storage_get_shape(lp_storage_t *storage)
{
    return storage->_shape;
}

lp_shape lp_storage_get_capacity(lp_storage_t *storage)
{
    // if (storage->type == IN_MEMORY)
    // {
    //     return storage->handle.in_memory->_capacity;
    // }

    return (lp_shape){0, 0};
}

size_t lp_storage_get_mem_used(lp_storage_t *storage)
{
    // if (storage->type == IN_MEMORY)
    // {
    //     return storage->handle.in_memory->_mem_used;
    // }

    return 0;
}

lp_bool lp_storage_set_2d(lp_storage_t *storage, lp_bool is_2d)
{
    // if (storage->type == IN_MEMORY)
    // {
    //     storage->handle.in_memory->is_2d = is_2d;
    //     return 1;
    // }

    return 0;
}

void lp_storage_set_shape(lp_storage_t *storage, lp_shape shape)
{
    storage->_shape = shape;
}

lp_bool lp_storage_set_capacity(lp_storage_t *storage, lp_shape capacity)
{
    // if (storage->type == IN_MEMORY)
    // {
    //     storage->handle.in_memory->_capacity = capacity;
    //     return 1;
    // }

    return 0;
}

lp_bool lp_storage_set_mem_used(lp_storage_t *storage, size_t mem_used)
{
    // if (storage->type == IN_MEMORY)
    // {
    //     storage->handle.in_memory->_mem_used = mem_used;
    //     return 1;
    // }

    return 0;
}

lp_bool lp_storage_inc_mem_used(lp_storage_t *storage, size_t mem_used)
{
    // if (storage->type == IN_MEMORY)
    // {
    //     storage->handle.in_memory->_mem_used += mem_used;
    //     return 1;
    // }

    return 0;
}

lp_bool lp_storage_dec_mem_used(lp_storage_t *storage, size_t mem_used)
{
    // if (storage->type == IN_MEMORY)
    // {
    //     storage->handle.in_memory->_mem_used -= mem_used;
    //     return 1;
    // }

    return 0;
}
