#include "mergesort.h"

static int compare_fields(lp_field_t *a, lp_field_t *b, lp_dtype dtype)
{
  if (!a->buffer || !b->buffer)
    return 0;

  switch (dtype)
  {
  case LP_INT:
  {
    long a_val = strtol(a->buffer, NULL, 10);
    long b_val = strtol(b->buffer, NULL, 10);
    return (a_val > b_val) - (a_val < b_val);
  }
  case LP_FLOAT:
  {
    double a_val = strtod(a->buffer, NULL);
    double b_val = strtod(b->buffer, NULL);
    return (a_val > b_val) - (a_val < b_val);
  }
  default:
    return strcmp(a->buffer, b->buffer);
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

static void merge(void *arr, lp_size_t left, lp_size_t mid, lp_size_t right, lp_dtype dtype, dynamic_array **col_data, lp_size_t cols, lp_size_t col_index)
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
    if (compare_fields(dynamic_array_get(leftArr[col_index], i), dynamic_array_get(rightArr[col_index], j), dtype) <= 0)
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
    lp_size_t col_index)
{
  if (left < right)
  {

    // Calculate the midpoint
    lp_size_t mid = left + (right - left) / 2;

    // Sort first and second halves
    mergesort_recursive(arr, left, mid, dtype, col_data, cols, col_index);
    mergesort_recursive(arr, mid + 1, right, dtype, col_data, cols, col_index);

    // Merge the sorted halves
    merge(arr, left, mid, right, dtype, col_data, cols, col_index);
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
    dynamic_array **col_data)
{
  if (!array || size <= 1 || !col_data)
    return array;

  size_t start = has_header ? 1 : 0;
  size_t actual_size = size - start;

  if (col_index >= cols)
    return array;

  // Sort the column data
  mergesort_recursive(col_data[col_index], start, actual_size, dtype, col_data, cols, col_index); // TODO: Should actual_size be actual_size - 1?

  return col_data[col_index];
}