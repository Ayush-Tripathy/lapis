#pragma once

#include <stdlib.h>
#include <string.h>

#include "dynamicarray.h"
#include "logger.h"
#include "storage.h"

dynamic_array *lp_merge_sort(
    lp_size_t col_index,
    dynamic_array *array,
    size_t size,
    lp_dtype dtype,
    lp_storage_type stype,
    lp_bool has_header,
    lp_size_t cols,
    dynamic_array **col_data,
    lp_storage_t *storage);
