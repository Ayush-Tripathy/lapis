#pragma once

#include <stdbool.h>

#include "frame.h"
#include "storage.h"

typedef struct
{
    size_t start;
    size_t end;
} ffield_tt;

bool scan_fields_indexes(
    char *buffer,
    size_t *pos,
    size_t size,
    frame *f,
    char delim,
    char quote_char,
    char comment_char,
    char escape_char);