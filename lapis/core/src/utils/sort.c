#include "sort.h"
#include "timsort.h"
#include "dynamicarray.h"

static lp_dtype get_column_dtype(lp_storage_t *storage, int column_index)
{
  lp_field_t *column_field = dynamic_array_get(storage->data.cols[column_index], column_index);
  return column_field->dtype;
}

lp_storage_t *lp_sort_values(lp_storage_t *storage, const char *by_column, lp_bool ascending)
{
  lp_size_t column_index = lp_storage_get_col_index(storage, by_column);
  if (column_index < 0)
  {
    return NULL;
  }

  lp_dtype dtype = get_column_dtype(storage, column_index);
  dynamic_array *column_data = storage->data.cols[column_index];

  size_t n = dynamic_array_size(column_data);
  lp_tim_sort(column_data, n, dtype);

  if (!ascending)
  {
    for (size_t i = 0; i < n / 2; i++)
    {
      void *temp = dynamic_array_get(column_data, i);
      dynamic_array_set(column_data, i, dynamic_array_get(column_data, n - i - 1));
      dynamic_array_set(column_data, n - i - 1, temp);
    }
  }

  return storage;
}
