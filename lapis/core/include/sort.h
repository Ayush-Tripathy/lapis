#pragma once

#include "storage.h"
#include <stdio.h>
#include <stdlib.h>

// Function to sort the storage based on a specific column
lp_storage_t* lp_sort_values(lp_storage_t *storage, const char *by_column, lp_bool ascending);

// Add any other necessary declarations or includes here
