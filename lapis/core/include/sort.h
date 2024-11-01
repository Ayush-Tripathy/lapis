#pragma once

#include "storage.h"
#include "logger.h"

lp_storage_t *lp_sort_values(lp_storage_t *storage, const char *by_column, lp_bool ascending);
