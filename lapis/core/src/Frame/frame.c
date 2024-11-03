/*
 * FILE: frame.c
 *
 * 2 Dimensional dynamic array
 */

#include "frame.h"

/**
 * @brief Initializes a frame with options.
 *
 * @param cols: Number of columns
 * @param col_names: Column names
 * @param type: Storage type
 * @param has_header: Whether frame has header or not
 * @param is_view: Whether frame is a view or not
 * @return frame*
 */
frame *frame_init(size_t cols, char **col_names, lp_storage_type type, lp_bool has_header, lp_bool is_view)
{
  frame *f = (frame *)malloc(sizeof(frame));
  if (f == NULL)
    return NULL;

  f->cols = cols;
  f->rows = 0;
  f->_mem_used = sizeof(frame);
  f->_is_view = is_view;
  f->_is_transposed = LP_FALSE;
  f->_row_indexes = NULL;
  f->_col_indexes = NULL;
  f->has_header = has_header;

  // TODO: Make a intial guess of rows based on cols, currently immature
  size_t rows;
  if (cols > 50)
  {
    rows = 1;
  }
  else
  {
    rows = 10000;
  }

  lp_shape shape = {
      .rows = rows, // Initial size
      .cols = cols};

  f->storage = lp_storage_init(type, shape, col_names);
  if (f->storage == NULL)
  {
    free(f);
    return NULL;
  }

  f->_mem_used += lp_storage_get_mem_used(f->storage);
  return f;
}

/**
 * @brief Returns memory used by the frame
 *
 * // TODO: Implement IN_MEMORY storage mem_used operation
 *
 * @param f: frame
 * @return size_t
 */
size_t frame_mem_used(frame *f)
{
  if (f == NULL)
    return 0;

  return f->_mem_used;
}

/**
 * @brief Frees the frame and its storage, no need to free storage separately.
 *
 * @param f: frame
 */
void frame_free(frame *f)
{
  if (f == NULL)
    return;

  short is_view = f->_is_view;
  if (is_view)
  {
    free(f->_row_indexes);
    free(f->_col_indexes);
  }

  lp_storage_free(f->storage);

  free(f);
  f = NULL;
}

/**
 * @brief Returns field at given row and column index.
 * It simply returns the field from the storage, if row 0 is accessed then it will return the header field (if present)
 *
 * @param f: frame
 * @param row: row index
 * @param col: column index
 * @return lp_field_t*
 */
lp_field_t *frame_get(frame *f, size_t row, size_t col)
{
  // TODO: For now we are ignoring out of bounds check for view
  lp_bool is_view = f->_is_view;

  if (f == NULL || (!is_view && (row >= f->rows || col >= f->cols)))
  {
    LOG_ERROR("frame_get: Invalid row or column index");
    exit(1);
  }

  if (is_view)
  {
    if (f->_row_indexes != NULL)
      row = f->_row_indexes[row % f->rows];

    if (f->_col_indexes != NULL)
      col = f->_col_indexes[col % f->cols];
  }

  if (f->storage->data.cols == NULL)
  {
    LOG_DEBUG("frame_get: f->storage->data.cols is NULL");
    exit(1);
  }

  lp_field_t *field = (lp_field_t *)dynamic_array_get(f->storage->data.cols[col], row);
  return field;
}

/**
 * @brief Returns column name for the given column index
 *
 * @param f: frame
 * @param col: column index
 * @return lp_string
 */
lp_string frame_get_col_name(frame *f, size_t col)
{
  if (f == NULL || col >= f->cols)
  {
    LOG_ERROR("frame_get_col_name: Invalid column index");
    exit(1);
  }

  // if (f->storage->type == MMAPPED)
  // {
  //     if (f->storage->data.cols != NULL)
  //     {
  //         return (lp_field_t *)dynamic_array_get(f->storage->data.cols[col], 0);
  //     }
  //     else
  //     {
  //         return NULL;
  //     }
  // }
  // else if (f->storage->type == IN_MEMORY)
  // {

  lp_bool is_view = f->_is_view;
  lp_bool has_header = f->has_header;

  if (!has_header)
  {
    return NULL;
  }

  if (is_view)
  {
    if (f->_col_indexes != NULL)
      col = f->_col_indexes[col];
  }

  if (f->storage->col_names != NULL)
  {
    return f->storage->col_names[col];
  }
  else
  {
    return NULL;
  }
  // }
}

/**
 * @brief Returns string representation of the frame
 *
 * @param f: frame
 * @return char*
 */
char *frame_str(frame *f)
{
  if (f == NULL)
    return NULL;

  // lp_shape shape = lp_storage_get_shape(f->storage);

  // size_t rows = shape.rows;
  // size_t cols = shape.cols;
  size_t rows = f->rows;
  size_t cols = f->cols;
  lp_bool has_header = f->has_header;

  // if (f->has_header)
  // {
  //   rows = rows - 1;
  // }

  lp_shape shape = {
      .rows = rows,
      .cols = cols,
  };

  // return string with first 3 and last 1 rows and first 3 and last 1 columns
  // Format -
  // | col1     | col2     | col3     | ... | colN     |
  // | row1Val1 | row1Val2 | row1val3 | ... | row1valN |
  // | row2Val1 | row2Val2 | row2val3 | ... | row2valN |
  // | row3Val1 | row3Val2 | row3val3 | ... | row3valN |
  // | ...      | ...      | ...      | ... | ...      |
  // | rowNVal1 | rowNVal2 | rowNval3 | ... | rowNvalN |

  int actual_rows = rows > 10 ? 10 : rows;
  int actual_cols = cols > 4 ? 4 : cols;

  size_t str_len = 4096; // Calculate total length of the string based on maximum length in column fields in the range
  char *str = (char *)malloc(str_len * sizeof(char));
  if (str == NULL)
    return NULL;

  size_t pos = 0;
  size_t len = 0;

  // All fields must take this much space, regardless of their actual length, align center
  const int CHARS_WIDTH = 15;

  char *buffer;

  if (f->storage->type == MMAPPED)
  {
    buffer = f->storage->handle.mmapped->buffer;

    // Calculate total length of the string based on maximum length in column fields in the range
    lp_size_t *FIELD_LENS = (lp_size_t *)malloc(actual_cols * sizeof(lp_size_t)); // respective fields in each column will take this much space if len < FIELD_LEN then add padding at start
    if (FIELD_LENS == NULL)
    {
      free(str);
      return NULL;
    }

    for (size_t i = 0; i < actual_cols; i++)
    {
      FIELD_LENS[i] = 0;
    }

    const int half = (actual_rows / 2) == 0 ? 1 : (actual_rows / 2);
    // 1st half rows

    size_t row_start = has_header ? 1 : 0;
    for (size_t i = row_start; i < half; i++)
    {
      for (size_t j = 0; j < actual_cols; j++)
      {
        lp_field_t *field = frame_get(f, i, j);

        size_t start = field->start;
        size_t end = field->end;
        size_t len = end - start;

        if (len > FIELD_LENS[j])
        {
          FIELD_LENS[j] = len;
        }
      }
    }

    // 2nd half rows
    for (size_t i = rows - actual_rows / 2; i < rows; i++)
    {
      for (size_t j = 0; j < actual_cols; j++)
      {
        lp_field_t *field = frame_get(f, i, j);

        size_t start = field->start;
        size_t end = field->end;
        size_t len = end - start;

        if (len > FIELD_LENS[j])
        {
          FIELD_LENS[j] = len;
        }
      }
    }

    // for (size_t i = 0; i < actual_cols; i++)
    // {
    //   // LOG_DEBUG("FIELD_LENS[%zu]: %zu", i, FIELD_LENS[i]);
    //   FIELD_LENS[i] = FIELD_LENS[i] > CHARS_WIDTH ? CHARS_WIDTH : FIELD_LENS[i];
    //   // LOG_DEBUG("AFTER FIELD_LENS[%zu]: %zu", i, FIELD_LENS[i]);
    // }

    // Print column names
    // TODO: Handle case when column names are not present
    for (size_t i = 0; i < actual_cols; i++)
    {
      lp_string col_name = frame_get_col_name(f, i);
      len = snprintf(str + pos, str_len - pos, "| %*s ", (int)FIELD_LENS[i], col_name);
      pos += len;
    }

    len = snprintf(str + pos, str_len - pos, "|\n");
    pos += len;

    if (shape.rows > 0)
    {
      // Print 1st half rows
      for (size_t i = row_start; i < half; i++)
      {
        for (size_t j = 0; j < actual_cols; j++)
        {
          lp_field_t *field = frame_get(f, i, j);

          size_t start = field->start;
          size_t end = field->end;
          size_t len = end - start;
          char *row_val = (char *)malloc((len + 1) * sizeof(char));
          if (row_val == NULL)
          {
            free(str);
            return NULL;
          }

          strncpy(row_val, buffer + start, len);
          row_val[len] = '\0';

          len = snprintf(str + pos, str_len - pos, "| %*s ", (int)FIELD_LENS[j], row_val);
          pos += len;
        }

        len = snprintf(str + pos, str_len - pos, "|\n");
        pos += len;
      }

      // Print ... if rows > rows/2
      if (actual_rows < rows)
      {
        for (size_t i = 0; i < actual_cols; i++)
        {
          len = snprintf(str + pos, str_len - pos, "| %*s ", (int)FIELD_LENS[i], "...");
          pos += len;
        }

        len = snprintf(str + pos, str_len - pos, "|\n");
        pos += len;
      }

      // Print 2nd half rows
      for (size_t i = rows - actual_rows / 2; i < rows; i++)
      {
        for (size_t j = 0; j < actual_cols; j++)
        {
          lp_field_t *field = frame_get(f, i, j);

          size_t start = field->start;
          size_t end = field->end;
          size_t len = end - start;
          char *row_val = (char *)malloc((len + 1) * sizeof(char));
          if (row_val == NULL)
          {
            free(str);
            return NULL;
          }

          strncpy(row_val, buffer + start, len);
          row_val[len] = '\0';

          len = snprintf(str + pos, str_len - pos, "| %*s ", (int)FIELD_LENS[j], row_val);
          pos += len;
        }

        len = snprintf(str + pos, str_len - pos, "|\n");
        pos += len;
      }
    }
    else /* Empty frame */
    {
      len = snprintf(str + pos, str_len - pos, "Empty DataFrame\n");
      pos += len;
    }

    // Attach shape
    len = snprintf(str + pos, str_len - pos, "Shape: (%zu, %zu)\n", has_header ? shape.rows - 1 : shape.rows, shape.cols);
  }
  else if (f->storage->type == IN_MEMORY)
  {
    buffer = NULL;

    // Print column names start
    for (size_t i = 0; i < actual_cols - 1; i++) // Print all columns except last one, as we need to check if we need to print ...
    {
      lp_field_t *field = frame_get(f, 0, i);

      char *col_name = (char *)malloc(CHARS_WIDTH * sizeof(char));
      if (col_name == NULL)
      {
        free(str);
        return NULL;
      }

      size_t k = 0;
      size_t field_len = strlen(field->buffer) > CHARS_WIDTH ? CHARS_WIDTH - 3 : strlen(field->buffer);
      for (size_t j = 0; j < field_len; j++)
      {
        col_name[k++] = field->buffer[j];
      }
      // Add ... if string is too long
      if (field_len == CHARS_WIDTH - 3)
      {
        col_name[k++] = '.';
        col_name[k++] = '.';
        col_name[k++] = '.';
      }
      col_name[k] = '\0';

      len = snprintf(str + pos, str_len - pos, "| %*s ", CHARS_WIDTH, col_name);
      pos += len;
    }

    if (cols > 4)
    {
      len = snprintf(str + pos, str_len - pos, "| %*s ", CHARS_WIDTH, "...");
      pos += len;
    }

    lp_field_t *field = frame_get(f, 0, cols - 1);

    char *col_name = (char *)malloc(CHARS_WIDTH * sizeof(char));
    if (col_name == NULL)
    {
      free(str);
      return NULL;
    }

    size_t k = 0;
    size_t field_len = strlen(field->buffer) > CHARS_WIDTH ? CHARS_WIDTH - 3 : strlen(field->buffer);
    for (size_t j = 0; j < field_len; j++)
    {
      col_name[k++] = field->buffer[j];
    }
    // Add ... if string is too long
    if (field_len == CHARS_WIDTH - 3)
    {
      col_name[k++] = '.';
      col_name[k++] = '.';
      col_name[k++] = '.';
    }
    col_name[k] = '\0';

    len = snprintf(str + pos, str_len - pos, "| %*s ", CHARS_WIDTH, col_name);
    pos += len;

    len = snprintf(str + pos, str_len - pos, "|\n");
    pos += len;
    // Print column names end

    // Print rows start
    for (size_t i = 1; i < actual_rows; i++)
    {
      for (size_t j = 0; j < actual_cols - 1; j++)
      {
        lp_field_t *field = frame_get(f, i, j);

        char *row_val = (char *)malloc(CHARS_WIDTH * sizeof(char));
        if (row_val == NULL)
        {
          free(str);
          return NULL;
        }

        size_t k = 0;
        size_t field_len = strlen(field->buffer) > CHARS_WIDTH ? CHARS_WIDTH : strlen(field->buffer);
        for (size_t j = 0; j < field_len; j++)
        {
          row_val[k++] = field->buffer[j];
        }
        row_val[k] = '\0';

        len = snprintf(str + pos, str_len - pos, "| %*s ", CHARS_WIDTH, row_val);
        pos += len;
      }

      if (cols > 4)
      {
        len = snprintf(str + pos, str_len - pos, "| %*s ", CHARS_WIDTH, "...");
        pos += len;
      }

      lp_field_t *field = frame_get(f, i, cols - 1);

      char *row_val = (char *)malloc(CHARS_WIDTH * sizeof(char));
      if (row_val == NULL)
      {
        free(str);
        return NULL;
      }

      size_t k = 0;
      size_t field_len = strlen(field->buffer) > CHARS_WIDTH ? CHARS_WIDTH : strlen(field->buffer);
      for (size_t j = 0; j < field_len; j++)
      {
        row_val[k++] = field->buffer[j];
      }
      row_val[k] = '\0';

      len = snprintf(str + pos, str_len - pos, "| %*s ", CHARS_WIDTH, row_val);
      pos += len;

      len = snprintf(str + pos, str_len - pos, "|\n");
      pos += len;
    }

    if (rows > 4)
    {
      if (cols > 4)
      {
        len = snprintf(str + pos, str_len - pos, "| %*s | %*s | %*s | %*s | %*s |\n", CHARS_WIDTH, "...", CHARS_WIDTH, "...", CHARS_WIDTH, "...", CHARS_WIDTH, "...", CHARS_WIDTH, "...");
        pos += len;
      }
      else
      {
        len = snprintf(str + pos, str_len - pos, "| %*s | %*s | %*s | %*s |\n", CHARS_WIDTH, "...", CHARS_WIDTH, "...", CHARS_WIDTH, "...", CHARS_WIDTH, "...");
        pos += len;
      }

      for (size_t i = rows - 1; i < rows; i++)
      {
        for (size_t j = 0; j < actual_cols - 1; j++)
        {
          lp_field_t *field = frame_get(f, i, j);

          char *row_val = (char *)malloc(CHARS_WIDTH * sizeof(char));
          if (row_val == NULL)
          {
            free(str);
            return NULL;
          }

          size_t k = 0;
          size_t field_len = strlen(field->buffer) > CHARS_WIDTH ? CHARS_WIDTH : strlen(field->buffer);
          for (size_t j = 0; j < field_len; j++)
          {
            row_val[k++] = field->buffer[j];
          }
          row_val[k] = '\0';

          len = snprintf(str + pos, str_len - pos, "| %*s ", CHARS_WIDTH, row_val);
          pos += len;
        }

        if (cols > 4)
        {
          len = snprintf(str + pos, str_len - pos, "| %*s ", CHARS_WIDTH, "...");
          pos += len;
        }

        lp_field_t *field = frame_get(f, i, cols - 1);

        char *row_val = (char *)malloc(CHARS_WIDTH * sizeof(char));
        if (row_val == NULL)
        {
          free(str);
          return NULL;
        }

        size_t k = 0;
        size_t field_len = strlen(field->buffer) > CHARS_WIDTH ? CHARS_WIDTH : strlen(field->buffer);
        for (size_t j = 0; j < field_len; j++)
        {
          row_val[k++] = field->buffer[j];
        }
        row_val[k] = '\0';

        len = snprintf(str + pos, str_len - pos, "| %*s ", CHARS_WIDTH, row_val);
        pos += len;

        len = snprintf(str + pos, str_len - pos, "|\n");
        pos += len;
      }
    }
    // Print rows end

    // Attach shape
    len = snprintf(str + pos, str_len - pos, "Shape: (%zu, %zu)\n", has_header ? shape.rows - 1 : shape.rows, shape.cols);
  }
  return str;
}

/**
 * @brief Returns column index by column name
 *
 * @param f: frame
 * @param col_name: column name
 * @return lp_size_t
 */
lp_size_t frame_get_col_index(frame *f, const char *col_name)
{
  if (f == NULL)
    return -1;

  return lp_storage_get_col_index(f->storage, col_name);
}

/**
 * @brief Returns shape of the frame
 *
 * @param f: frame
 * @return lp_shape
 */
lp_shape frame_get_shape(frame *f)
{
  if (f == NULL)
  {
    lp_shape shape = {
        .rows = 0,
        .cols = 0};
    return shape;
  }

  return f->storage->_shape;
}

/**
 * @brief Returns value at given row and column index as string
 *
 * @param f: frame
 * @param row: row index
 * @param col: column index
 * @return lp_string
 */
static lp_string frame_get_value_as_string(frame *f, size_t row, size_t col)
{
  if (f == NULL)
  {
    return NULL;
  }

  if (f->has_header)
  {
    row = row + 1;
  }

  return lp_storage_get_value_as_string(f->storage, row, col);
}