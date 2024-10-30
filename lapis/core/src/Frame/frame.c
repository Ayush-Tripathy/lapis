/*
 * FILE: frame.c
 *
 * 2 Dimensional dynamic array
 */

#include "frame.h"

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

    f->storage = lp_storage_init(type, shape);
    if (f->storage == NULL)
    {
        free(f);
        return NULL;
    }

    f->_mem_used += lp_storage_get_mem_used(f->storage);
    return f;
}

// TODO: Fix this, not showing correct memory usage
size_t frame_mem_used(frame *f)
{
    if (f == NULL)
        return 0;

    return f->_mem_used;
}

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

// TODO: Implement IN_MEMORY storage get operation
lp_field_t *frame_get(frame *f, size_t row, size_t col)
{
    // TODO: For now we are ignoring out of bounds check for view
    lp_bool is_view = f->_is_view;
    lp_bool has_header = f->has_header;

    if (f == NULL || (!is_view && (row >= f->rows || col >= f->cols)))
        return NULL;

    if (has_header == LP_TRUE)
    {
        row = row + 1;
    }

    if (is_view)
    {
        if (f->_row_indexes != NULL)
            row = f->_row_indexes[row];

        if (f->_col_indexes != NULL)
            col = f->_col_indexes[col];
    }

    if (f->storage->type == MMAPPED)
    {
        if (f->storage->data.cols != NULL)
        {
            lp_field_t *field = (lp_field_t *)dynamic_array_get(f->storage->data.cols[col], row);
            return field;
        }
        else
        {
            lp_field_t *field = (lp_field_t *)dynamic_array_get(f->storage->data.fields, row);
            return field;
        }
    }
    else if (f->storage->type == IN_MEMORY)
    {
        if (f->storage->data.cols != NULL)
        {
            lp_field_t *field = (lp_field_t *)dynamic_array_get(f->storage->data.cols[col], row);
            return field;
        }
        else
        {
            lp_field_t *field = (lp_field_t *)dynamic_array_get(f->storage->data.fields, row);
            return field;
        }
    }
}

lp_field_t *frame_get_col(frame *f, size_t col)
{
    if (f == NULL || col >= f->cols)
        return NULL;

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

    if (f->storage->data.cols != NULL)
    {
        return (lp_field_t *)dynamic_array_get(f->storage->data.cols[col], 0);
    }
    else
    {
        return NULL;
    }
    // }
}

char *frame_str(frame *f)
{
    if (f == NULL)
        return NULL;

    size_t rows = f->rows;
    size_t cols = f->cols;

    // return string with first 3 and last 1 rows and first 3 and last 1 columns
    // Format -
    // | col1     | col2     | col3     | ... | colN     |
    // | row1Val1 | row1Val2 | row1val3 | ... | row1valN |
    // | row2Val1 | row2Val2 | row2val3 | ... | row2valN |
    // | row3Val1 | row3Val2 | row3val3 | ... | row3valN |
    // | ...      | ...      | ...      | ... | ...      |
    // | rowNVal1 | rowNVal2 | rowNval3 | ... | rowNvalN |

    int actual_rows = rows > 4 ? 4 : rows;
    int actual_cols = cols > 4 ? 4 : cols;

    size_t str_len = 1024;
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

        // Print column names
        for (size_t i = 0; i < actual_cols - 1; i++)
        {
            lp_field_t *field = frame_get_col(f, i);

            // Construct string from indexes
            char *col_name = (char *)malloc(CHARS_WIDTH * sizeof(char));
            if (col_name == NULL)
            {
                free(str);
                return NULL;
            }

            size_t k = 0;
            size_t field_len = field->end - field->start > CHARS_WIDTH ? (CHARS_WIDTH + field->start) - 3 : field->end;
            for (size_t j = field->start; j < field_len; j++)
            {
                col_name[k++ % CHARS_WIDTH] = buffer[j];
            }
            // Add ... if string is too long
            if (field_len == (CHARS_WIDTH + field->start) - 3)
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

        lp_field_t *field = frame_get_col(f, cols - 1);

        // Construct string from indexes
        char *col_name = (char *)malloc(CHARS_WIDTH * sizeof(char));
        if (col_name == NULL)
        {
            free(str);
            return NULL;
        }

        size_t k = 0;
        size_t field_len = field->end - field->start > CHARS_WIDTH ? (CHARS_WIDTH + field->start) - 3 : field->end;
        for (size_t j = field->start; j < field_len; j++)
        {
            col_name[k++ % CHARS_WIDTH] = buffer[j];
        }
        // Add ... if string is too long
        if (field_len == (CHARS_WIDTH + field->start) - 3)
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

        // Print rows
        for (size_t i = 0; i < actual_rows; i++)
        {
            for (size_t j = 0; j < actual_cols - 1; j++)
            {
                lp_field_t *field = frame_get(f, i, j);

                // Construct string from indexes
                char *row_val = (char *)malloc(CHARS_WIDTH * sizeof(char));
                if (row_val == NULL)
                {
                    free(str);
                    return NULL;
                }

                size_t k = 0;
                size_t field_len = field->end - field->start > CHARS_WIDTH ? (CHARS_WIDTH + field->start) - 3 : field->end;
                for (size_t j = field->start; j < field_len; j++)
                {
                    row_val[k++ % CHARS_WIDTH] = buffer[j];
                }
                // Add ... if string is too long
                if (field_len == (CHARS_WIDTH + field->start) - 3)
                {
                    row_val[k++] = '.';
                    row_val[k++] = '.';
                    row_val[k++] = '.';
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

            // Construct string from indexes
            char *row_val = (char *)malloc(CHARS_WIDTH * sizeof(char));
            if (row_val == NULL)
            {
                free(str);
                return NULL;
            }

            size_t k = 0;
            size_t field_len = field->end - field->start > CHARS_WIDTH ? (CHARS_WIDTH + field->start) - 3 : field->end;
            for (size_t j = field->start; j < field_len; j++)
            {
                row_val[k++ % CHARS_WIDTH] = buffer[j];
            }
            // Add ... if string is too long
            if (field_len == (CHARS_WIDTH + field->start) - 3)
            {
                row_val[k++] = '.';
                row_val[k++] = '.';
                row_val[k++] = '.';
            }
            row_val[k] = '\0';

            len = snprintf(str + pos, str_len - pos, "| %*s ", CHARS_WIDTH, row_val);
            pos += len;

            len = snprintf(str + pos, str_len - pos, "|\n");
            pos += len;
        }

        if (rows > 4)
        {
            if (rows > 5)
            {
                if (cols > 4)
                {
                    len = snprintf(str + pos, str_len - pos, "| %*s | %*s | %*s | %*s | %*s |\n", CHARS_WIDTH, "...", CHARS_WIDTH, "...", CHARS_WIDTH, "...", CHARS_WIDTH, "...", CHARS_WIDTH, "...");
                    pos += len;
                }
                else if (cols == 4)
                {
                    len = snprintf(str + pos, str_len - pos, "| %*s | %*s | %*s | %*s |\n", CHARS_WIDTH, "...", CHARS_WIDTH, "...", CHARS_WIDTH, "...", CHARS_WIDTH, "...");
                    pos += len;
                }
                else if (cols == 3)
                {
                    len = snprintf(str + pos, str_len - pos, "| %*s | %*s | %*s |\n", CHARS_WIDTH, "...", CHARS_WIDTH, "...", CHARS_WIDTH, "...");
                    pos += len;
                }
                else if (cols == 2)
                {
                    len = snprintf(str + pos, str_len - pos, "| %*s | %*s |\n", CHARS_WIDTH, "...", CHARS_WIDTH, "...");
                    pos += len;
                }
                else if (cols == 1)
                {
                    len = snprintf(str + pos, str_len - pos, "| %*s |\n", CHARS_WIDTH, "...");
                    pos += len;
                }
            }
            for (size_t i = rows - 1; i < rows; i++)
            {
                for (size_t j = 0; j < actual_cols - 1; j++)
                {
                    lp_field_t *field = frame_get(f, i, j);

                    // Construct string from indexes
                    char *row_val = (char *)malloc(CHARS_WIDTH * sizeof(char));
                    if (row_val == NULL)
                    {
                        free(str);
                        return NULL;
                    }

                    size_t k = 0;
                    size_t field_len = field->end - field->start > CHARS_WIDTH ? (CHARS_WIDTH + field->start) - 3 : field->end;
                    for (size_t j = field->start; j < field_len; j++)
                    {
                        row_val[k++ % CHARS_WIDTH] = buffer[j];
                    }
                    // Add ... if string is too long
                    if (field_len == (CHARS_WIDTH + field->start) - 3)
                    {
                        row_val[k++] = '.';
                        row_val[k++] = '.';
                        row_val[k++] = '.';
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

                // Construct string from indexes
                char *row_val = (char *)malloc(CHARS_WIDTH * sizeof(char));
                if (row_val == NULL)
                {
                    free(str);
                    return NULL;
                }

                size_t k = 0;
                size_t field_len = field->end - field->start > CHARS_WIDTH ? (CHARS_WIDTH + field->start) - 3 : field->end;
                for (size_t j = field->start; j < field_len; j++)
                {
                    row_val[k++ % CHARS_WIDTH] = buffer[j];
                }
                // Add ... if string is too long
                if (field_len == (CHARS_WIDTH + field->start) - 3)
                {
                    row_val[k++] = '.';
                    row_val[k++] = '.';
                    row_val[k++] = '.';
                }
                row_val[k] = '\0';

                len = snprintf(str + pos, str_len - pos, "| %*s ", CHARS_WIDTH, row_val);
                pos += len;

                len = snprintf(str + pos, str_len - pos, "|\n");
                pos += len;
            }
        }

        // Attach shape
        len = snprintf(str + pos, str_len - pos, "Shape: (%zu, %zu)\n", f->rows, f->cols);
    }
    else if (f->storage->type == IN_MEMORY)
    {
        buffer = NULL;

        // Print column names
        for (size_t i = 0; i < actual_cols - 1; i++)
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

        // Print rows
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

        // Attach shape
        len = snprintf(str + pos, str_len - pos, "Shape: (%zu, %zu)\n", f->rows, f->cols);
    }
    return str;
}