#include "filter.h"

static int string_condition_matches(const char *value, size_t len, FilterOp op, const char *condition_value)
{
  switch (op)
  {
  case OP_STARTS_WITH:
    return strncmp(value, condition_value, strlen(condition_value)) == 0;
  case OP_ENDS_WITH:
  {
    size_t cond_len = strlen(condition_value);
    return len >= cond_len && strcmp(value + len - cond_len, condition_value) == 0;
  }
  case OP_CONTAINS:
    return strstr(value, condition_value) != NULL;
  default:
    return 0;
  }
}

FilterResult filter_by_condition(frame *frame, const char *column_name, FilterOp op, const char *condition_value)
{
  // Find the column index by name
  // TODO: Refactor this to a separate function
  lp_size_t column_index = frame_get_col_index(frame, column_name);

  if (column_index == -1)
  {
    // Return an empty result if the column is not found
    FilterResult empty_result = {NULL, 0};
    return empty_result;
  }

  lp_dtype dtype = ((lp_field_t *)dynamic_array_get(frame->storage->data.cols[column_index], 0))->dtype;

  size_t rows_count = frame->storage->_shape.rows;

  FilterResult result;
  result.indexes = (int *)malloc(rows_count * sizeof(int));
  result.count = 0;

  if (frame->storage->type == MMAPPED)
  {
    char *buffer = frame->storage->handle.mmapped->buffer;

    for (size_t i = 0; i < rows_count; i++)
    {
      int match = 0;
      lp_field_t *field = (lp_field_t *)dynamic_array_get(frame->storage->data.cols[column_index], i);

      if (dtype == LP_INT || dtype == LP_FLOAT)
      {
        int value = lp_atoi(buffer, field->start, field->end);
        int condition = atoi(condition_value);

        switch (op)
        {
        case OP_GT:
          match = value > condition;
          break;
        case OP_LT:
          match = value < condition;
          break;
        case OP_EQ:
          match = value == condition;
          break;
        case OP_GTE:
          match = value >= condition;
          break;
        case OP_LTE:
          match = value <= condition;
          break;
        default:
          match = 0;
        }
      }
      else if (dtype == LP_STRING)
      {
        // Handle string comparison
        char *value = buffer + field->start;
        match = string_condition_matches(value, field->end - field->start, op, condition_value);
      }

      if (match)
      {
        result.indexes[result.count] = i;
        result.count++;
      }
    }
  }
  else
  {
    // TODO: for IN_MEMORY storage
  }

  return result;
}
