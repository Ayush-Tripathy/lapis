#include "filter.h"  

FilterResult filter_by_condition(int* column_data, lp_shape dataset_shape, int condition_value) {
    FilterResult result;
    result.indexes = (int*)malloc(dataset_shape.rows * sizeof(int));  
    result.count = 0;

    for (size_t i = 0; i < dataset_shape.rows; i++) {
        if (column_data[i] > condition_value) {  
            result.indexes[result.count] = i;  
            result.count++;
        }
    }

    return result;
}
