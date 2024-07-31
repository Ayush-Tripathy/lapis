#include "storage.h"

lp_storage_t *lp_storage_init(lp_storage_type type, lp_shape shape)
{
    lp_storage_t *storage = (lp_storage_t *)malloc(sizeof(lp_storage_t));
    if (storage == NULL)
    {
        return NULL;
    }

    storage->type = type;

    switch (type)
    {
    case MMAPPED:
        storage->handle.mmapped = (lp_mmapped_t *)malloc(sizeof(lp_mmapped_t));
        if (storage->handle.mmapped == NULL)
        {
            free(storage);
            return NULL;
        }
        break;

    case IN_MEMORY:
        storage->handle.in_memory = (lp_mfield_t *)malloc(sizeof(lp_mfield_t));
        if (storage->handle.in_memory == NULL)
        {
            free(storage);
            return NULL;
        }
        break;

    default:
        free(storage);
        return NULL;
    }

    return storage;
}

void lp_storage_free(lp_storage_t *storage)
{
    if (storage == NULL)
    {
        return;
    }

    switch (storage->type)
    {
    case MMAPPED:
        free(storage->handle.mmapped);
        break;

    case IN_MEMORY:
        free(storage->handle.in_memory);
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
    if (storage->type == IN_MEMORY)
    {
        return storage->handle.in_memory->is_2d;
    }

    return 0;
}

lp_shape lp_storage_get_shape(lp_storage_t *storage)
{
    if (storage->type == IN_MEMORY)
    {
        return storage->handle.in_memory->_shape;
    }

    return (lp_shape){0, 0};
}

lp_shape lp_storage_get_capacity(lp_storage_t *storage)
{
    if (storage->type == IN_MEMORY)
    {
        return storage->handle.in_memory->_capacity;
    }

    return (lp_shape){0, 0};
}

size_t lp_storage_get_mem_used(lp_storage_t *storage)
{
    if (storage->type == IN_MEMORY)
    {
        return storage->handle.in_memory->_mem_used;
    }

    return 0;
}

lp_bool lp_storage_set_2d(lp_storage_t *storage, lp_bool is_2d)
{
    if (storage->type == IN_MEMORY)
    {
        storage->handle.in_memory->is_2d = is_2d;
        return 1;
    }

    return 0;
}

lp_bool lp_storage_set_shape(lp_storage_t *storage, lp_shape shape)
{
    if (storage->type == IN_MEMORY)
    {
        storage->handle.in_memory->_shape = shape;
        return 1;
    }

    return 0;
}

lp_bool lp_storage_set_capacity(lp_storage_t *storage, lp_shape capacity)
{
    if (storage->type == IN_MEMORY)
    {
        storage->handle.in_memory->_capacity = capacity;
        return 1;
    }

    return 0;
}

lp_bool lp_storage_set_mem_used(lp_storage_t *storage, size_t mem_used)
{
    if (storage->type == IN_MEMORY)
    {
        storage->handle.in_memory->_mem_used = mem_used;
        return 1;
    }

    return 0;
}

lp_bool lp_storage_inc_mem_used(lp_storage_t *storage, size_t mem_used)
{
    if (storage->type == IN_MEMORY)
    {
        storage->handle.in_memory->_mem_used += mem_used;
        return 1;
    }

    return 0;
}