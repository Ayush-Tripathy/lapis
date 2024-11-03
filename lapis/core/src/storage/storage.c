#include "storage.h"

lp_storage_t *lp_storage_init(lp_storage_type type, lp_shape shape, lp_string *col_names)
{
  lp_storage_t *storage = (lp_storage_t *)malloc(sizeof(lp_storage_t));
  if (storage == NULL)
  {
    return NULL;
  }

  storage->type = type;
  storage->_shape = shape;

  if (col_names != NULL)
  {
    storage->col_names = (lp_string *)malloc(shape.cols * sizeof(lp_string));
    if (storage->col_names == NULL)
    {
      free(storage);
      return NULL;
    }

    for (size_t i = 0; i < shape.cols; i++)
    {
      size_t len = strlen(col_names[i]);
      storage->col_names[i] = (lp_string)malloc(len + 1);
      strncpy(storage->col_names[i], col_names[i], len);
      storage->col_names[i][len] = '\0';
    }
  }
  else
  {
    storage->col_names = NULL;
  }

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

  lp_size_t cols;
  if (storage->_shape.cols)
    cols = storage->_shape.cols;
  else
    cols = 0;

  if (storage->ref_count > 0)
  {
    storage->ref_count--;
    return;
  }

  if (storage->type)
  {
    switch (storage->type)
    {
    case MMAPPED:
      if (storage->data.cols != NULL)
      {
        if (cols > 0)
        {
          for (size_t i = 0; i < cols; i++)
          {
            if (storage->data.cols[i] != NULL)
            { // Added NULL check
              dynamic_array_free(storage->data.cols[i]);
            }
          }
        }
        free(storage->data.cols);
      }
      else if (storage->data.fields != NULL)
      {
        dynamic_array_free(storage->data.fields);
      }

      if (storage->handle.mmapped != NULL)
      { // Added NULL check
        if (storage->handle.mmapped->buffer != NULL)
        { // Nested NULL check
          unmap_file(storage->handle.mmapped->buffer, storage->handle.mmapped->buffer_size);
        }
        free(storage->handle.mmapped);
      }
      break;

    case IN_MEMORY:
      if (storage->data.cols != NULL)
      {
        if (cols > 0)
        {
          for (size_t i = 0; i < cols; i++)
          {
            if (storage->data.cols[i] != NULL)
            { // Added NULL check
              dynamic_array_free(storage->data.cols[i]);
            }
          }
        }
        free(storage->data.cols);
      }
      else if (storage->data.fields != NULL)
      {
        dynamic_array_free(storage->data.fields);
      }
      break;

    default:
      break;
    }
  }

  if (storage->col_names != NULL)
  {
    if (cols > 0)
    {
      for (size_t i = 0; i < cols; i++)
      {
        if (storage->col_names[i] != NULL)
        { // Added NULL check
          free(storage->col_names[i]);
        }
      }
    }
    free(storage->col_names);
  }

  if (storage != NULL)
  {
    free(storage);
  }

  // if (storage == NULL)
  // {
  //   return;
  // }

  // lp_size_t cols = storage->_shape.cols;

  // if (storage->ref_count > 0)
  // {
  //   storage->ref_count--;
  //   return;
  // }

  // switch (storage->type)
  // {
  // case MMAPPED:
  //   if (storage->data.cols != NULL)
  //   {
  //     if (cols > 0)
  //     {
  //       for (size_t i = 0; i < cols; i++)
  //       {
  //         dynamic_array_free(storage->data.cols[i]);
  //       }
  //     }
  //     free(storage->data.cols);
  //   }
  //   else if (storage->data.fields != NULL)
  //   {
  //     dynamic_array_free(storage->data.fields);
  //   }

  //   // Unmap file
  //   if (storage->handle.mmapped->buffer != NULL)
  //   {
  //     unmap_file(storage->handle.mmapped->buffer, storage->handle.mmapped->buffer_size);
  //   }

  //   if (storage->handle.mmapped != NULL)
  //   {
  //     free(storage->handle.mmapped);
  //   }
  //   break;

  // case IN_MEMORY:
  //   // if (storage->handle.in_memory != NULL)
  //   // {
  //   //     free(storage->handle.in_memory);
  //   // }

  //   if (storage->data.cols != NULL)
  //   {
  //     if (cols > 0)
  //     {
  //       for (size_t i = 0; i < cols; i++)
  //       {
  //         dynamic_array_free(storage->data.cols[i]);
  //       }
  //     }
  //     free(storage->data.cols);
  //   }
  //   else if (storage->data.fields != NULL)
  //   {
  //     dynamic_array_free(storage->data.fields);
  //   }
  //   break;

  // default:
  //   break;
  // }

  // if (storage->col_names != NULL)
  // {
  //   if (cols > 0)
  //   {
  //     for (size_t i = 0; i < cols; i++)
  //     {
  //       if (storage->col_names[i] != NULL)
  //         free(storage->col_names[i]);
  //     }
  //   }
  //   free(storage->col_names);
  // }

  // if (storage != NULL)
  //   free(storage);
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

lp_size_t lp_storage_get_col_index(lp_storage_t *storage, const char *col_name)
{
  for (size_t i = 0; i < storage->_shape.cols; i++)
  {
    if (strcmp(storage->col_names[i], col_name) == 0)
    {
      return i;
    }
  }

  return -1;
}

lp_string lp_storage_get_value_as_string(lp_storage_t *storage, size_t row, size_t col)
{
  if (storage == NULL)
  {
    return NULL;
  }

  if (storage->type == MMAPPED)
  {
    if (storage->data.cols != NULL)
    {
      lp_field_t *field = (lp_field_t *)dynamic_array_get(storage->data.cols[col], row);

      size_t start = field->start;
      size_t end = field->end;
      size_t len = end - start;
      char *buffer = storage->handle.mmapped->buffer;
      char *value = (char *)malloc(len + 1);
      if (value == NULL)
      {
        return NULL;
      }

      strncpy(value, buffer + start, len);
      value[len] = '\0';
      return value;
    }
    else
    {
      lp_field_t *field = (lp_field_t *)dynamic_array_get(storage->data.fields, row);

      size_t start = field->start;
      size_t end = field->end;
      size_t len = end - start;
      char *buffer = storage->handle.mmapped->buffer;
      char *value = (char *)malloc(len + 1);
      if (value == NULL)
      {
        return NULL;
      }

      strncpy(value, buffer + start, len);
      value[len] = '\0';
      return value;
    }
  }
  else if (storage->type == IN_MEMORY)
  {
    if (storage->data.cols != NULL)
    {
      lp_field_t *field = (lp_field_t *)dynamic_array_get(storage->data.cols[col], row);
      return field->buffer;
    }
    else
    {
      lp_field_t *field = (lp_field_t *)dynamic_array_get(storage->data.fields, row);
      return field->buffer;
    }
  }

  return NULL;
}