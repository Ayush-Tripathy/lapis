#include "ccsv.h"

/* Writer */

#define RETURN_IF_WRITE_ERROR(writer, desired_status) \
    if (writer->write_status != desired_status)       \
        return writer->write_status;

ccsv_writer *ccsv_init_writer(ccsv_writer_options *options, short *status)
{
    char delim, quote_char, escape_char;
    WriterState state = WRITER_NOT_STARTED;
    if (options == NULL)
    {
        delim = DEFAULT_DELIMITER;
        quote_char = DEFAULT_QUOTE_CHAR;
        escape_char = DEFAULT_ESCAPE_CHAR;
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

        if (options->escape_char == CCSV_NULL_CHAR)
            escape_char = DEFAULT_ESCAPE_CHAR;

        else
            escape_char = options->escape_char;
    }

    // Writer
    ccsv_writer *writer = (ccsv_writer *)malloc(sizeof(ccsv_writer));
    if (writer == NULL)
    {
        if (status != NULL)
            *status = CCSV_ERNOMEM;
        return NULL;
    }
    writer->__delim = delim;
    writer->__quote_char = quote_char;
    writer->__escape_char = escape_char;
    writer->__state = state;

    writer->write_status = WRITER_NOT_STARTED;
    writer->object_type = CCSV_WRITER;

    return writer;
}

int ccsv_write(ccsv_writer *writer, ccsv_row row)
{
    if (writer == NULL)
        return WRITE_ERNOTSTARTED;

    if (writer->__fp == NULL)
        return CCSV_ERNULLFP;

    return write_row(writer->__fp, writer, row);
}

int ccsv_write_from_array(ccsv_writer *writer, char **fields, int fields_len)
{
    if (writer == NULL)
        return WRITE_ERNOTSTARTED;

    if (writer->__fp == NULL)
        return CCSV_ERNULLFP;

    return write_row_from_array(writer->__fp, writer, fields, fields_len);
}

int write_row(FILE *fp, ccsv_writer *writer, ccsv_row row)
{
    const int fields_count = row.fields_count;
    char **fields = row.fields;
    return (write_row_from_array(fp, writer, fields, fields_count));
}

int write_row_from_array(FILE *fp, ccsv_writer *writer, char **fields, int row_len)
{
    CCSV_WRITE_ROW_START(fp, writer);
    RETURN_IF_WRITE_ERROR(writer, WRITE_STARTED);

    for (int i = 0; i < row_len; i++)
    {
        const char *field = fields[i];
        CCSV_WRITE_FIELD(fp, writer, field);
        RETURN_IF_WRITE_ERROR(writer, WRITE_SUCCESS);
    }
    CCSV_WRITE_ROW_END(fp, writer, NULL);
    RETURN_IF_WRITE_ERROR(writer, WRITE_ENDED);

    writer->write_status = WRITE_SUCCESS;
    return WRITE_SUCCESS;
}

int _write_row_start(FILE *fp, ccsv_writer *writer)
{
    long file_size;
    char last_char;

    switch (writer->__state)
    {
    case WRITER_NOT_STARTED:
        writer->__state = WRITER_ROW_START; /* Start writing row */

        /* Move the file pointer to the end to get the file size */
        fseek(fp, 0, SEEK_END);
        file_size = ftell(fp);

        /* Move the pointer to the penultimate position */
        fseek(fp, -1, SEEK_END);

        last_char = fgetc(fp);

        if (last_char != CCSV_LF && last_char != CCSV_CR && file_size > 0)
        {
            fputc(CCSV_CR, fp);
            fputc(CCSV_LF, fp);
        }

        /* Rewind the file pointer */
        fseek(fp, -file_size, SEEK_END);
        break;

    case WRITER_ROW_END:
        writer->__state = WRITER_ROW_START; /* Start writing row */
        break;

    case WRITER_WRITING_FIELD:
    case WRITER_ROW_START:
        writer->__state = WRITER_ROW_END;
        writer->write_status = WRITE_ERALWRITING; /* Already writing field */
        return WRITE_ERALWRITING;

    default:
        break;
    }

    writer->write_status = WRITE_STARTED;
    return WRITE_STARTED;
}

int _write_row_end(FILE *fp, ccsv_writer *writer)
{
    switch (writer->__state)
    {
    case WRITER_NOT_STARTED:
        writer->__state = WRITER_ROW_END;
        writer->write_status = WRITE_ERNOTSTARTED; /* Not started writing, CSV_WRITE_ROW_START() not called */
        return WRITE_ERNOTSTARTED;

    case WRITER_ROW_START:
    case WRITER_WRITING_FIELD:
        writer->__state = WRITER_ROW_END;
        fputc(CCSV_CR, fp);
        fputc(CCSV_LF, fp);
        break;

    case WRITER_ROW_END:
        writer->__state = WRITER_NOT_STARTED; /* Reset the state */
        break;

    default:
        break;
    }

    writer->write_status = WRITE_ENDED;
    return WRITE_ENDED;
}

int _write_field(FILE *fp, ccsv_writer *writer, const char *string)
{
    WriterState state = writer->__state;
    if (state != WRITER_ROW_START && state != WRITER_WRITING_FIELD)
    {
        /* Not started writing, CSV_WRITE_ROW_START() not called */
        writer->write_status = WRITE_ERNOTSTARTED;
        return WRITE_ERNOTSTARTED;
    }

    const char DELIM = writer->__delim;
    const char QUOTE_CHAR = writer->__quote_char;
    const char ESCAPE_CHAR = writer->__escape_char;

    int inside_quotes = 0;

    size_t string_len = strlen(string);

    char ch;
    for (size_t i = 0; i < string_len; i++)
    {
        ch = string[i];
        if (ch == DELIM || ch == QUOTE_CHAR || ch == CCSV_CR || ch == CCSV_LF)
        {
            inside_quotes = 1;
            break;
        }
    }

    if (inside_quotes)
    {
        fputc(QUOTE_CHAR, fp);
        for (size_t i = 0; i < string_len; i++)
        {
            ch = string[i];
            /* Escape the quote character */
            if (ch == QUOTE_CHAR)
                fputc(ESCAPE_CHAR, fp);

            fputc(ch, fp);
        }
        fputc(QUOTE_CHAR, fp);
    }
    else
        fputs(string, fp);

    writer->write_status = WRITE_SUCCESS;
    return WRITE_SUCCESS;
}