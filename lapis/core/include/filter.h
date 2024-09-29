#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "storage.h" 

typedef struct {
    int* indexes; 
    size_t count;  
} FilterResult;

FilterResult filter_by_condition(int* column_data, lp_shape dataset_shape, int condition_value);

