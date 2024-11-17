#include "mergesort.h"

// TODO: Refactor this - Too many repeated code
static int compare_fields(lp_field_t *a, lp_field_t *b, lp_dtype dtype, lp_storage_type stype, lp_string mmaped_buffer)
{
  if (stype == IN_MEMORY && (!a->buffer || !b->buffer))
    return 0;

  switch (dtype)
  {
  case LP_INT:
  {
    if (stype == IN_MEMORY)
    {
      lp_long a_val = strtol(a->buffer, NULL, 10);
      lp_long b_val = strtol(b->buffer, NULL, 10);
      return (a_val > b_val) - (a_val < b_val);
    }
    else if (stype == MMAPPED)
    {

      lp_size_t astart = a->start;
      lp_size_t bstart = b->start;

      lp_size_t aend = a->end;
      lp_size_t bend = b->end;

      // build string from start to end
      lp_string a_val = (lp_string)malloc(aend - astart + 1);
      lp_string b_val = (lp_string)malloc(bend - bstart + 1);

      if (!a_val || !b_val)
      {
        return 0;
      }

      memcpy(a_val, mmaped_buffer + astart, aend - astart);
      memcpy(b_val, mmaped_buffer + bstart, bend - bstart);

      a_val[aend - astart] = '\0';
      b_val[bend - bstart] = '\0';

      lp_long a_val_int = strtol(a_val, NULL, 10);
      lp_long b_val_int = strtol(b_val, NULL, 10);

      free(a_val);
      free(b_val);

      return (a_val_int > b_val_int) - (a_val_int < b_val_int);
    }
  }
  case LP_FLOAT:
  {
    if (stype == IN_MEMORY)
    {
      double a_val = strtod(a->buffer, NULL);
      double b_val = strtod(b->buffer, NULL);
      return (a_val > b_val) - (a_val < b_val);
    }
    else if (stype == MMAPPED)
    {
      lp_size_t astart = a->start;
      lp_size_t bstart = b->start;

      lp_size_t aend = a->end;
      lp_size_t bend = b->end;

      // build string from start to end
      lp_string a_val = (lp_string)malloc(aend - astart + 1);
      lp_string b_val = (lp_string)malloc(bend - bstart + 1);

      if (!a_val || !b_val)
      {
        return 0;
      }

      memcpy(a_val, mmaped_buffer + astart, aend - astart);
      memcpy(b_val, mmaped_buffer + bstart, bend - bstart);

      a_val[aend - astart] = '\0';
      b_val[bend - bstart] = '\0';

      double a_val_float = strtod(a_val, NULL);
      double b_val_float = strtod(b_val, NULL);

      free(a_val);
      free(b_val);

      return (a_val_float > b_val_float) - (a_val_float < b_val_float);
    }
  }
  default:
    if (stype == IN_MEMORY)
    {
      return strcmp(a->buffer, b->buffer);
    }
    else if (stype == MMAPPED)
    {
      lp_size_t astart = a->start;
      lp_size_t bstart = b->start;

      lp_size_t aend = a->end;
      lp_size_t bend = b->end;

      // build string from start to end
      lp_string a_val = (lp_string)malloc(aend - astart + 1);
      lp_string b_val = (lp_string)malloc(bend - bstart + 1);

      if (!a_val || !b_val)
      {
        return 0;
      }

      memcpy(a_val, mmaped_buffer + astart, aend - astart);
      memcpy(b_val, mmaped_buffer + bstart, bend - bstart);

      a_val[aend - astart] = '\0';
      b_val[bend - bstart] = '\0';

      int cmp = strcmp(a_val, b_val);

      free(a_val);
      free(b_val);

      return cmp;
    }
  }
}

static void swap_rows(dynamic_array **col_data, size_t row1, size_t row2, size_t cols)
{
  for (size_t col = 0; col < cols; col++)
  {
    lp_size_t element_size = col_data[col]->element_size;
    void *temp = malloc(element_size);
    if (!temp)
      return;

    memcpy(temp, dynamic_array_get(col_data[col], row1), element_size);

    dynamic_array_set(col_data[col], row1, dynamic_array_get(col_data[col], row2));
    dynamic_array_set(col_data[col], row2, temp);
  }
}

static void merge(void *arr, lp_size_t left, lp_size_t mid, lp_size_t right, lp_dtype dtype, dynamic_array **col_data, lp_size_t cols, lp_size_t col_index, lp_storage_t *storage)
{
  lp_size_t i, j, k;
  lp_size_t n1 = mid - left + 1;
  lp_size_t n2 = right - mid;

  // Temporary arrays to hold copies of every row in the left and right subarrays
  dynamic_array **leftArr = (dynamic_array **)malloc(cols * sizeof(dynamic_array *));
  dynamic_array **rightArr = (dynamic_array **)malloc(cols * sizeof(dynamic_array *));
  // void **leftArr = malloc(n1 * sizeof(void *));
  // void **rightArr = malloc(n2 * sizeof(void *));

  // Copy data to temporary arrays
  // for (i = 0; i < n1; i++)
  // {
  //   leftArr[i] = dynamic_array_get_copy(arr, left + i);
  // }

  for (size_t col = 0; col < cols; col++)
  {
    leftArr[col] = dynamic_array_init(col_data[col]->size, col_data[col]->element_size);
    for (i = 0; i < n1; i++)
    {
      dynamic_array_push(leftArr[col], dynamic_array_get_copy(col_data[col], left + i));
    }
  }
  // for (j = 0; j < n2; j++)
  // {
  //   rightArr[j] = dynamic_array_get_copy(arr, mid + 1 + j);
  // }

  for (size_t col = 0; col < cols; col++)
  {
    rightArr[col] = dynamic_array_init(col_data[col]->size, col_data[col]->element_size);
    for (j = 0; j < n2; j++)
    {
      dynamic_array_push(rightArr[col], dynamic_array_get_copy(col_data[col], mid + 1 + j));
    }
  }
  // Merge the temporary arrays back into arr[left..right]
  i = 0;
  j = 0;
  k = left;
  while (i < n1 && j < n2)
  {
    if (compare_fields(
            dynamic_array_get(leftArr[col_index], i),
            dynamic_array_get(rightArr[col_index], j),
            dtype, storage->type, storage->handle.mmapped->buffer) <= 0)
    {
      for (size_t col = 0; col < cols; col++)
      {
        dynamic_array_set(col_data[col], k, dynamic_array_get(leftArr[col], i));
      }

      i++;
    }
    else
    {
      for (size_t col = 0; col < cols; col++)
      {
        dynamic_array_set(col_data[col], k, dynamic_array_get(rightArr[col], j));
      }

      j++;
    }

    // if (compare_fields(leftArr[i], rightArr[j], dtype) <= 0)
    // {
    //   dynamic_array_set(arr, k, leftArr[i]);
    //   i++;
    // }
    // else
    // {
    //   dynamic_array_set(arr, k, rightArr[j]);
    //   j++;
    // }

    k++;
  }

  // Copy the remaining elements of leftArr[], if any
  while (i < n1)
  {
    // dynamic_array_set(arr, k, leftArr[i]);

    for (size_t col = 0; col < cols; col++)
    {
      dynamic_array_set(col_data[col], k, dynamic_array_get(leftArr[col], i));
    }

    i++;
    k++;
  }

  // Copy the remaining elements of rightArr[], if any
  while (j < n2)
  {
    // dynamic_array_set(arr, k, rightArr[j]);

    for (size_t col = 0; col < cols; col++)
    {
      dynamic_array_set(col_data[col], k, dynamic_array_get(rightArr[col], j));
    }

    j++;
    k++;
  }

  // Free the temporary arrays
  // for (i = 0; i < n1; i++)
  // {
  //   free(leftArr[i]);
  // }
  // for (j = 0; j < n2; j++)
  // {
  //   free(rightArr[j]);
  // }
  // free(leftArr);
  // free(rightArr);

  for (size_t col = 0; col < cols; col++)
  {
    dynamic_array_free(leftArr[col]);
    dynamic_array_free(rightArr[col]);
  }
  free(leftArr);
  free(rightArr);
}

static void mergesort_recursive(
    void *arr,
    lp_size_t left,
    lp_size_t right,
    lp_dtype dtype,
    dynamic_array **col_data,
    lp_size_t cols,
    lp_size_t col_index,
    lp_storage_t *storage)
{
  if (left < right)
  {

    // Calculate the midpoint
    lp_size_t mid = left + (right - left) / 2;

    // Sort first and second halves
    mergesort_recursive(arr, left, mid, dtype, col_data, cols, col_index, storage);
    mergesort_recursive(arr, mid + 1, right, dtype, col_data, cols, col_index, storage);

    // Merge the sorted halves
    merge(arr, left, mid, right, dtype, col_data, cols, col_index, storage);
  }
}

dynamic_array *lp_merge_sort(
    size_t col_index,
    dynamic_array *array,
    size_t size,
    lp_dtype dtype,
    lp_storage_type stype,
    lp_bool has_header,
    size_t cols,
    dynamic_array **col_data,
    lp_storage_t *storage)
{
  if (!array || size <= 1 || !col_data)
    return array;

  size_t start = has_header ? 1 : 0;
  size_t actual_size = size - start;

  if (col_index >= cols)
    return array;

  // Sort the column data
  mergesort_recursive(col_data[col_index], start, actual_size, dtype, col_data, cols, col_index, storage); // TODO: Should actual_size be actual_size - 1?

  return col_data[col_index];
}