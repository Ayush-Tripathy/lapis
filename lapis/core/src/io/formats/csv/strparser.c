/*
 * File: strparser.c
 *
 * This is used to parse the csv string.
 * When using memory mapped files, we can use this to parse the csv string.
 */

#include <stdio.h>

#include "strparser.h"

#define IS_TERMINATOR(c) (c == '\0' || c == '\n' || c == '\r')

// This function expects number of fields already determined
// We can use the fileparser to determine the number of fields
bool scan_fields_indexes(
    char *buffer,
    size_t *pos,
    size_t size,
    frame *f,
    char delim,
    char quote_char,
    char comment_char,
    char escape_char)
{
    size_t fields_count = 0;
    size_t i = *pos;
    size_t start = i;
    size_t end = i;
    bool in_quote = false;

    if (i >= size)
        return false;

    bool actual_quoted = false;
    bool actual_comment = false;
    size_t field_pos = 0, row_pos = 0;
    char ch = buffer[i];
    field_series_t **field_series = f->series;
    while (i < size && (!IS_TERMINATOR(ch) || (IS_TERMINATOR(ch) && in_quote)))
    {
        if (ch == ' ' && !in_quote && field_pos == 0)
        {
            // field_pos = -1;
            // Skip the leading spaces
            while (i < size && buffer[i] == ' ')
            {
                i++;
            }

            start = i;
            end = i;
            ch = buffer[i];
            field_pos = 0;
            continue;
        }
        else if (ch == ' ' && !in_quote)
        {
            end = i - 1;
            // Skip the trailing spaces
            while (i < size && buffer[i] == ' ')
            {
                i++;
            }

            field_pos = -1;
            ch = buffer[i];
            continue;
        }
        else if (ch == quote_char)
        {
            if (field_pos == 0)
            {
                // printf("quote at start\n");
                in_quote = true;
                actual_quoted = true;
            }
            else if (buffer[i - 1] == escape_char)
            {
                // Do nothing
            }
            else
            {
                in_quote = !in_quote;
            }

            // in_quote = !in_quote;
        }
        else if (ch == comment_char && !in_quote && row_pos == 0)
        {
            actual_comment = true;
            // // Increment the position till any non terminator character is found
            while (i < size && !IS_TERMINATOR(buffer[i]))
            {
                i++;
            }

            start = i + 1;
            end = i + 1;
            ch = buffer[++i];
            field_pos = 0;
            actual_quoted = false;
            actual_comment = false;
            continue;
        }
        else if (ch == delim && !in_quote)
        {
            ffield_t field = {
                .start = start,
                .end = end + 1,
                .quoted = actual_quoted,
            };
            field_series_append(field_series[fields_count], field);
            fields_count++;
            start = i + 1;
            field_pos = -1;
            actual_quoted = false;
        }

        end = i;
        ch = buffer[++i];
        field_pos++;
        row_pos++;
    }

    ffield_t field = {
        .start = start,
        .end = end + 1,
        .quoted = actual_quoted,
    };

    if (!actual_comment)
    {
        field_series_append(field_series[fields_count], field);
    }

    // if (fields_count != f->cols - 1)
    // {
    //     printf("Fields count mismatch: %lu != %lu at %ld\n", fields_count, f->cols, i);
    //     return false;
    // }

    // Increment the position till any non terminator character is found
    while (i < size && IS_TERMINATOR(buffer[i]))
    {
        i++;
    }

    *pos = i;

    return true;
}