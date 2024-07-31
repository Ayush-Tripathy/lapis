/*
 * FILE: fileparser.c
 *
 * This is used to parse the csv file using a file handle.
 * Mainly used to infer the schema of the csv file.
 * If the file is too large, and cannot be mapped to memory, then
 * we can use these functions to parse the complete file in chunks.
 */

// Created: 2023 by Ayush Tripathy
// github.com/Ayush-Tripathy

/*
 * This library provides functions to handle reading, writing csv files.
 */

/*
    MIT License

    Copyright (c) 2023 Ayush Tripathy

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// ccsv header file
#include "ccsv.h"

const char *status_messages[] = {
    "Success",
    "Error",
    "Memory allocation failure.",
    "Malformed CSV file.",
    "Not started writing, CSV_WRITE_ROW_START() not called.",
    "Already writing field, CSV_WRITE_ROW_START() already called."};

const char *ccsv_get_status_message(short status)
{
    if (status > 0)
        return status_messages[0];

    if (status < -5)
        return NULL;
    return status_messages[-1 * status];
}

int _get_object_type(void *obj)
{
    if (obj == NULL)
        return CCSV_NULL_CHAR;

    if (((ccsv_reader *)obj)->object_type == CCSV_READER)
        return CCSV_READER;

    if (((ccsv_writer *)obj)->object_type == CCSV_WRITER)
        return CCSV_WRITER;

    return CCSV_NULL_CHAR;
}

int ccsv_is_error(void *obj, short *status)
{
    if (obj == NULL)
        return 0;

    if (_get_object_type(obj) == CCSV_READER)
    {
        short __status = ((ccsv_reader *)obj)->status;
        if (status != NULL)
            *status = __status;
        return __status < 0;
    }

    if (_get_object_type(obj) == CCSV_WRITER)
    {
        short __status = ((ccsv_writer *)obj)->write_status;
        if (status != NULL)
            *status = __status;
        return __status < 0;
    }

    return 0;
}

/* Reader */

// These macros should be used only in read_row() function
#define ADD_FIELD(field)                                              \
    field[field_pos++] = CCSV_NULL_CHAR;                              \
    fields_count++;                                                   \
    fields = (char **)realloc(fields, sizeof(char *) * fields_count); \
    if (fields == NULL)                                               \
    {                                                                 \
        _free_multiple(3, field, row_string, row);                    \
        reader->status = CCSV_ERNOMEM;                                \
        return NULL;                                                  \
    }                                                                 \
    fields[fields_count - 1] = field;

#define GROW_FIELD_BUFFER_IF_NEEDED(field, field_size, field_pos) \
    if (field_pos > field_size - 1)                               \
    {                                                             \
        field_size += MAX_FIELD_SIZE;                             \
        field = (char *)realloc(field, field_size + 1);           \
        if (field == NULL)                                        \
        {                                                         \
            _free_multiple(3, fields, row_string, row);           \
            reader->status = CCSV_ERNOMEM;                        \
            return NULL;                                          \
        }                                                         \
    }

#define GROW_ROW_BUFFER_IF_NEEDED(row_string, row_len, row_pos)        \
    if (row_pos > row_len - 1)                                         \
    {                                                                  \
        row_string_size += reader->__buffer_size;                      \
        row_string = (char *)realloc(row_string, row_string_size + 1); \
        if (row_string == NULL)                                        \
        {                                                              \
            _free_multiple(2, fields, row);                            \
            reader->status = CCSV_ERNOMEM;                             \
            return NULL;                                               \
        }                                                              \
    }

#define RETURN_IF_WRITE_ERROR(writer, desired_status) \
    if (writer->write_status != desired_status)       \
        return writer->write_status;

ccsv_reader *ccsv_init_reader(ccsv_reader_options *options, short *status)
{
    char delim, quote_char, comment_char, escape_char;
    int skip_initial_space, skip_empty_lines, skip_comments;
    if (options == NULL)
    {
        delim = DEFAULT_DELIMITER;
        quote_char = DEFAULT_QUOTE_CHAR;
        comment_char = DEFAULT_COMMENT_CHAR;
        escape_char = DEFAULT_ESCAPE_CHAR;
        skip_initial_space = 0;
        skip_empty_lines = 0;
        skip_comments = 0;
    }
    else
    {
        // It is not mandatory to pass all options to options struct
        // So check if the option is passed or not, if not then use the default value
        if (options->delim == CCSV_NULL_CHAR)
            delim = DEFAULT_DELIMITER;

        else
            delim = options->delim;

        if (options->quote_char == CCSV_NULL_CHAR)
            quote_char = DEFAULT_QUOTE_CHAR;

        else
            quote_char = options->quote_char;

        if (options->comment_char == CCSV_NULL_CHAR)
            comment_char = DEFAULT_COMMENT_CHAR;

        else
            comment_char = options->comment_char;

        if (options->escape_char == CCSV_NULL_CHAR)
            escape_char = DEFAULT_ESCAPE_CHAR;

        else
            escape_char = options->escape_char;

        if (options->skip_initial_space == CCSV_NULL_CHAR)
            skip_initial_space = 0;

        else
            skip_initial_space = options->skip_initial_space;

        if (options->skip_empty_lines == CCSV_NULL_CHAR)
            skip_empty_lines = 0;

        else
            skip_empty_lines = options->skip_empty_lines;

        if (options->skip_comments == CCSV_NULL_CHAR)
            skip_comments = 0;

        else
            skip_comments = options->skip_comments;
    }

    // Parser
    ccsv_reader *parser = (ccsv_reader *)malloc(sizeof(ccsv_reader));
    if (parser == NULL)
    {
        if (status != NULL)
            *status = CCSV_ERNOMEM;
        return NULL;
    }
    parser->__delim = delim;
    parser->__quote_char = quote_char;
    parser->__comment_char = comment_char;
    parser->__escape_char = escape_char;
    parser->__skip_initial_space = skip_initial_space;
    parser->__skip_empty_lines = skip_empty_lines;
    parser->__skip_comments = skip_comments;

    parser->__fp = NULL;

    parser->rows_read = 0;
    parser->status = CCSV_SUCCESS;
    parser->object_type = CCSV_READER;

    return parser;
}

void ccsv_free_row(ccsv_row *row)
{
    const int fields_count = row->fields_count;
    for (int i = 0; i < fields_count; i++)
    {
        free(row->fields[i]);
    }
    free(row->fields);
    free(row);
}

void _free_multiple(int num, ...)
{
    va_list args;
    va_start(args, num);

    for (int i = 0; i < num; ++i)
    {
        void *ptr = va_arg(args, void *);
        free(ptr);
    }

    va_end(args);
}

void *ccsv_open(const char *filename, short object_type, const char *mode, void *options, short *status)
{
    if (filename == NULL)
        return NULL;

    if (object_type != CCSV_READER && object_type != CCSV_WRITER)
    {
        if (status != NULL)
            *status = CCSV_ERINVOBJTYPE;
        return NULL;
    }

    if (strcmp(mode, "r") != 0 &&
        strcmp(mode, "rb") != 0 &&
        strcmp(mode, "r+") != 0 &&
        strcmp(mode, "rb+") != 0 &&
        strcmp(mode, "w+") != 0 &&
        strcmp(mode, "a+") != 0)
    {
        if (status != NULL)
            *status = CCSV_ERMODE;
        return NULL;
    }

    if (object_type == CCSV_READER)
    {
        short init_status;

#ifdef __cplusplus
        ccsv_reader_options *reader_options = reinterpret_cast<ccsv_reader_options *>(options);
        ccsv_reader *reader = ccsv_init_reader(reader_options, &init_status);
#else
        options = (ccsv_reader_options *)options;
        ccsv_reader *reader = ccsv_init_reader(options, &init_status);
#endif

        if (init_status == CCSV_ERNOMEM || reader == NULL)
        {
            if (status != NULL)
                *status = CCSV_ERNOMEM;
            return NULL;
        }
        FILE *fp = fopen(filename, mode);
        if (fp == NULL)
        {
            if (status != NULL)
                *status = CCSV_EROPEN;
            return NULL;
        }

        size_t buffer_size = CCSV_BUFFER_SIZE;

        size_t file_size;

        fseek(fp, 0, SEEK_END);
        file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        reader->__file_size = file_size;
        reader->__file_pos = 0;

        if (file_size >= CCSV_LARGE_FILE_SIZE)
            buffer_size = CCSV_HIGH_BUFFER_SIZE;
        else if (file_size >= CCSV_MED_FILE_SIZE)
            buffer_size = CCSV_MED_BUFFER_SIZE;
        else
            buffer_size = CCSV_LOW_BUFFER_SIZE;

        reader->__buffer = (char *)malloc(buffer_size + 1);
        if (reader->__buffer == NULL)
        {
            free(reader);
            if (status != NULL)
                *status = CCSV_ERNOMEM;
            return NULL;
        }
        reader->__buffer[0] = CCSV_NULL_CHAR;

        reader->__buffer_size = buffer_size;
        // reader->__buffer_allocated = true;

        reader->__fp = fp;
        reader->object_type = object_type;

        return reader;
    }
    else if (object_type == CCSV_WRITER)
    {
        FILE *fp = fopen(filename, mode);
        if (fp == NULL)
        {
            if (status != NULL)
                *status = CCSV_EROPEN;
            return NULL;
        }

        short init_status;

#ifdef __cplusplus
        ccsv_writer_options *writer_options = reinterpret_cast<ccsv_writer_options *>(options);
        ccsv_writer *writer = ccsv_init_writer(writer_options, &init_status);
#else
        options = (ccsv_writer_options *)options;
        ccsv_writer *writer = ccsv_init_writer(options, &init_status);
#endif
        if (init_status == CCSV_ERNOMEM || writer == NULL)
        {
            if (status != NULL)
                *status = CCSV_ERNOMEM;
            return NULL;
        }
        writer->__fp = fp;
        writer->object_type = object_type;

        return writer;
    }
    return NULL;
}

void ccsv_close(void *obj)
{
    if (obj == NULL)
        return;

    if (_get_object_type(obj) == CCSV_READER)
    {
        ccsv_reader *reader = (ccsv_reader *)obj;
        fclose(reader->__fp);
        free(reader->__buffer);
        free(reader);
    }
    else if (_get_object_type(obj) == CCSV_WRITER)
    {
        ccsv_writer *writer = (ccsv_writer *)obj;
        fclose(writer->__fp);
        free(writer);
    }
    else
    {
        return;
    }
}

ccsv_row *ccsv_next(ccsv_reader *reader)
{
    if (reader == NULL)
        return NULL;

    if (reader->__buffer == NULL)
    {
        reader->status = CCSV_ERBUFNTALLOC;
        return NULL;
    }

    if (reader->__fp == NULL)
    {
        reader->status = CCSV_ERNULLFP;
        return NULL;
    }

    return _next(reader->__fp, reader);
}

#define IS_TERMINATOR(c) (c == CCSV_CR || c == CCSV_LF || c == CCSV_NULL_CHAR)

ccsv_row *_next(FILE *fp, ccsv_reader *reader)
{
    ccsv_row *row = (ccsv_row *)malloc(sizeof(ccsv_row));
    if (row == NULL)
    {
        reader->status = CCSV_ERNOMEM;
        return NULL;
    }

    const char DELIM = reader->__delim;
    const char QUOTE_CHAR = reader->__quote_char;
    const char COMMENT_CHAR = reader->__comment_char;
    const char ESCAPE_CHAR = reader->__escape_char;
    const int SKIP_INITIAL_SPACE = reader->__skip_initial_space;
    const int SKIP_EMPTY_LINES = reader->__skip_empty_lines;
    const int SKIP_COMMENTS = reader->__skip_comments;

    size_t buffer_size = reader->__buffer_size;

    State state = FIELD_START;

    char *row_string = reader->__buffer;
    size_t row_pos = 0;

    char **fields = (char **)malloc(sizeof(char *));
    if (fields == NULL)
    {
        _free_multiple(2, row_string, row);
        reader->status = CCSV_ERNOMEM;
        return NULL;
    }

    size_t fields_count = 0;

    char *field = (char *)malloc(MAX_FIELD_SIZE + 1);
    if (field == NULL)
    {
        _free_multiple(3, row_string, fields, row);
        reader->status = CCSV_ERNOMEM;
        return NULL;
    }

    size_t field_size = MAX_FIELD_SIZE;
    size_t field_pos = 0;

    size_t bytes_read;
readfile:

    /* Checking buffer */
    if (_is_buffer_empty(reader))
    {
        bytes_read = fread(reader->__buffer, sizeof(char), buffer_size, fp);
        reader->__file_pos += bytes_read;

        if (bytes_read <= 0)
        {
            if (fields_count > 0 || field_pos > 0)
            {
                /* Add the last field */

                /*
                 * If fields_count > 0:
                 * This happens when the function holding the values of last row
                 * but yet to return the last row
                 *
                 * if field_pos > 0:
                 * This only happens when there is a single element in the last row and also
                 * there is no line after the current line
                 * So we need to add the only field of the last row
                 */
                ADD_FIELD(field);
                goto end;
            }

            _free_multiple(4, row_string, field, fields, row);
            return NULL;
        }

        reader->__buffer[bytes_read] = CCSV_NULL_CHAR;
        reader->__buffer_size = bytes_read;
        reader->__buffer_pos = 0;
        row_pos = 0;
        row_string = reader->__buffer;

        if (IS_TERMINATOR(row_string[row_pos]) && state == FIELD_START)
            row_pos++;
    }
    else
    {
        row_string = reader->__buffer;
        bytes_read = reader->__buffer_size;
        row_pos = reader->__buffer_pos;
    }

    for (; row_pos < bytes_read;)
    {
        char c = row_string[row_pos++];

        switch (state)
        {
        case FIELD_START:
            if (c == QUOTE_CHAR)
                state = INSIDE_QUOTED_FIELD; /* Start of quoted field */
            else if (SKIP_INITIAL_SPACE && c == CCSV_SPACE)
                state = FIELD_NOT_STARTED; /* Skip initial spaces */
            else if (c == DELIM || IS_TERMINATOR(c))
            {
                state = FIELD_END; /* Empty field or empty row */
                row_pos--;
            }
            else
            {
                state = FIELD_STARTED;
                field[field_pos++] = c;
            }
            break;

        case INSIDE_QUOTED_FIELD:
            GROW_FIELD_BUFFER_IF_NEEDED(field, field_size, field_pos);
            if (c == QUOTE_CHAR)
                state = MAY_BE_ESCAPED; /* Might be the end of the field, or it might be a escaped quote */
            else if (c == ESCAPE_CHAR)
            {
                field[field_pos++] = c; /* Escaped escape character */
                row_pos++;
            }
            else
                field[field_pos++] = c;

            break;

        case MAY_BE_ESCAPED:
            if (c == QUOTE_CHAR)
            {
                state = INSIDE_QUOTED_FIELD; /* Escaped quote */
                field[field_pos++] = c;
            }
            else if (c == DELIM || IS_TERMINATOR(c))
            {
                state = FIELD_END; /* End of field */
                row_pos--;
            }
            else
            {
                state = FIELD_STARTED;
                field[field_pos++] = c;
            }

            break;

        case FIELD_NOT_STARTED:
            if (c == QUOTE_CHAR)
                state = INSIDE_QUOTED_FIELD; /* Start of quoted field */
            else if (c == DELIM)
            {
                state = FIELD_END; /* Return empty field */
                row_pos--;
            }
            else if (c == CCSV_SPACE)
                state = FIELD_NOT_STARTED; /* Skip initial spaces, will only get to this point if skip_initial_spaces = 1 */
            else
            {
                state = FIELD_STARTED;
                field[field_pos++] = c; /* Start of non-quoted field */
            }
            break;

        case FIELD_STARTED:
            GROW_FIELD_BUFFER_IF_NEEDED(field, field_size, field_pos);

            if (c == DELIM || IS_TERMINATOR(c))
            {
                state = FIELD_END; /* End of field */
                row_pos--;
            }
            else
                field[field_pos++] = c; /* Add the character to the field */
            break;

        case FIELD_END:
            state = FIELD_START;

            if (SKIP_EMPTY_LINES &&
                fields_count == 0 &&
                field_pos == 0 &&
                IS_TERMINATOR(c))
            {
                /* Do not return empty lines, parse again */
                reader->__buffer_pos = row_pos;
                goto readfile;
            }
            else if (SKIP_COMMENTS &&
                     fields_count == 0 &&
                     field_pos > 0 &&
                     field[0] == COMMENT_CHAR)
            {
                /* Do not return comment lines, parse again */
                field_pos = 0;
                reader->__buffer_pos = row_pos + 1;
                goto readfile;
            }
            else
            {
                ADD_FIELD(field);
            }

            field = (char *)malloc(MAX_FIELD_SIZE + 1);
            field_size = MAX_FIELD_SIZE;
            if (field == NULL)
            {
                _free_multiple(3, fields, row_string, row);
                reader->status = CCSV_ERNOMEM;
                return NULL;
            }
            field_pos = 0;

            if (IS_TERMINATOR(c)) /* CR or LF */
            {

                if (IS_TERMINATOR(row_string[row_pos])) /* CRLF */
                    row_pos++;

                free(field);
                goto end;
            }
            break;

        default:
            break;
        }
    }

    // This point is reached only when for loop is completed fully
    if (row_pos > bytes_read - 1)
    {
        reader->__buffer[0] = CCSV_NULL_CHAR; /* Reset the buffer */
        goto readfile;
    }

end:
    row->fields = fields;
    row->fields_count = fields_count;

    if (row_pos > bytes_read - 1)
        reader->__buffer[0] = CCSV_NULL_CHAR; /* Reset the buffer */
    else
        reader->__buffer_pos = row_pos;

    reader->rows_read++;
    reader->status = CCSV_SUCCESS;
    return row;
}

int _is_buffer_empty(ccsv_reader *reader)
{
    return reader->__buffer[0] == CCSV_NULL_CHAR;
}