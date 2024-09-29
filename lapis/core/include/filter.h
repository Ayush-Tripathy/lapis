#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "storage.h"
#include "frame.h"  

typedef struct {
    int* indexes;  
    size_t count; 
} FilterResult; 


typedef enum {
    OP_GT,   // >
    OP_LT,   // <
    OP_EQ,   // =
    OP_GTE,  // >=
    OP_LTE,  // <=
    OP_STARTS_WITH,  // String starts with
    OP_ENDS_WITH,    // String ends with
    OP_CONTAINS      // String contains
} FilterOp;

FilterResult filter_by_condition(frame* frame, const char* column_name, FilterOp op, const char* condition_value);


