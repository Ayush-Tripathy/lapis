#include "lpio.h"

frame *_read_csv(
    const char *filename,
    const char delim,
    const char quote_char,
    const char comment_char,
    const char escape_char)
{
    ccsv_reader_options options = {
        .delim = delim,
        .quote_char = quote_char,
        .comment_char = comment_char,
        .escape_char = escape_char,
    };
    short status = 0;
    ccsv_reader *reader = (ccsv_reader *)ccsv_open(filename, CCSV_READER, "r", &options, &status);
    if (reader == NULL)
    {
        return NULL;
    }
    ccsv_row *row = ccsv_next(reader);

    frame *f = frame_init(row->fields_count, row->fields);
    if (f == NULL)
    {
        ccsv_free_row(row);
        ccsv_close(reader);
        return NULL;
    }

    ccsv_free_row(row);
    ccsv_close(reader);

    f->buffer = map_file(filename, &f->buffer_size);
    if (f->buffer == NULL)
    {
        frame_free(f);
        return NULL;
    }

    size_t pos = 0, count = 0;
    size_t buffer_size = f->buffer_size;
    char *buffer = f->buffer;
    size_t cols = f->cols;

    while (scan_fields_indexes(buffer, &pos, buffer_size, f, ',', '\"', '#', '\\'))
    {
        count++;
    }

    f->rows += count;

    return f;
}