#pragma once

#include <stdio.h>

#include "memmap.h"
#include "frame.h"
#include "series.h"
#include "ccsv.h"
#include "strparser.h"
#include "storage.h"
#include "dynamicarray.h"
#include "logger.h"

frame *_read_csv(
    const char *filename,
    lp_bool has_header,
    const char delim,
    const char quote_char,
    const char comment_char,
    const char escape_char);
