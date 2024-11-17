#include "sort.h"

static lp_dtype get_column_dtype(lp_storage_t *storage, int column_index, lp_bool has_header)
{
  lp_field_t *column_field;
  if (has_header)
  {
    column_field = dynamic_array_get(storage->data.cols[column_index], 1);
  }
  else
  {
    column_field = dynamic_array_get(storage->data.cols[column_index], 0);
  }

  if (storage->type == MMAPPED)
  {
    lp_size_t start = column_field->start;
    lp_size_t end = column_field->end;
    lp_size_t len = end - start;

    char *buffer = storage->handle.mmapped->buffer;
    char *value = (char *)malloc(len + 1);
    if (value == NULL)
    {
      return -1;
    }

    strncpy(value, buffer + start, len);
    value[len] = '\0';

    lp_dtype dtype = detect_type(value);
    free(value);
    return dtype;
  }
  else if (storage->type == IN_MEMORY)
  {
    return detect_type(column_field->buffer);
  }
}

lp_storage_t *lp_sort_values(
    lp_storage_t *storage,
    const char *by_column,
    lp_bool ascending,
    lp_bool has_header,
    lp_size_t cols,
    const char *kind)
{
  lp_size_t column_index = lp_storage_get_col_index(storage, by_column);
  if (column_index < 0)
  {
    return NULL;
  }

  lp_dtype dtype = get_column_dtype(storage, column_index, has_header);
  dynamic_array *column_data = storage->data.cols[column_index];
  size_t n = dynamic_array_size(column_data);
  if (strcmp(kind, "timsort") == 0) // TODO: fix timsort
  {
    lp_tim_sort(column_index, column_data, n, dtype, storage->type, has_header, cols, storage->data.cols);
  }
  else if (strcmp(kind, "mergesort") == 0)
  {
    lp_merge_sort(column_index, column_data, n, dtype, storage->type, has_header, cols, storage->data.cols, storage);
  }
  else
  {
    LOG_ERROR("lp_sort_values: Invalid kind");
    return NULL;
  }

  if (!ascending)
  {
    for (size_t i = has_header ? 1 : 0; i < n / 2; i++)
    {
      void *temp = dynamic_array_get(column_data, i);
      lp_field_t temp2 = {
          .buffer = ((lp_field_t *)temp)->buffer,
          .dtype = ((lp_field_t *)temp)->dtype,
          .quoted = ((lp_field_t *)temp)->quoted,
          .start = ((lp_field_t *)temp)->start,
          .end = ((lp_field_t *)temp)->end,
      };
      dynamic_array_set(column_data, i, dynamic_array_get(column_data, n - i - 1));
      dynamic_array_set(column_data, n - i - 1, &temp2);
    }
  }

  return storage;
}
