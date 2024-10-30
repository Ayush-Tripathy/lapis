#include "filter.h"

static int string_condition_matches(const char* value, FilterOp op, const char* condition_value) {
    switch (op) {
        case OP_STARTS_WITH:
            return strncmp(value, condition_value, strlen(condition_value)) == 0;
        case OP_ENDS_WITH: {
            size_t value_len = strlen(value);
            size_t cond_len = strlen(condition_value);
            return value_len >= cond_len && strcmp(value + value_len - cond_len, condition_value) == 0;
        }
        case OP_CONTAINS:
            return strstr(value, condition_value) != NULL;
        default:
            return 0;
    }
}


FilterResult filter_by_condition(lp_frame* frame, const char* column_name, FilterOp op, const char* condition_value) {
    // Find the column index by name
    int column_index = -1;
    for (size_t i = 0; i < frame->shape.cols; i++) {
        if (strcmp(frame->column_names[i], column_name) == 0) {
            column_index = i;
            break;
        }
    }

    
    if (column_index == -1) {
        // Return an empty result if the column is not found
        FilterResult empty_result = {NULL, 0};
        return empty_result;
    }

    
    lp_dtype dtype = frame->storage->data.fields[column_index]->dtype;

    FilterResult result;
    result.indexes = (int*)malloc(frame->shape.rows * sizeof(int));  
    result.count = 0;

    for (size_t i = 0; i < frame->shape.rows; i++) {
        int match = 0;  

        if (dtype == INT || dtype == FLOAT) {
            int value = atoi(frame->storage->data.fields[column_index]->buffer + i * sizeof(int));
            int condition = atoi(condition_value); 

            switch (op) {
                case OP_GT: match = value > condition; break;
                case OP_LT: match = value < condition; break;
                case OP_EQ: match = value == condition; break;
                case OP_GTE: match = value >= condition; break;
                case OP_LTE: match = value <= condition; break;
                default: match = 0;
            }
        } else if (dtype == STRING) {
            // Handle string comparison
            char* value = frame->storage->data.fields[column_index]->buffer + i * sizeof(char*);

            match = string_condition_matches(value, op, condition_value);
        }

        if (match) {
            result.indexes[result.count] = i; 
            result.count++;
        }
    }

    return result; 
}
