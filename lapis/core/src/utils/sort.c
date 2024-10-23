#include "sort.h"
#include "dynamicarray.h"

static int get_column_index(lp_storage_t *storage, const char *by_column) {
    for (size_t i = 0; i < storage->_shape.cols; i++) {
        lp_field_t *field = dynamic_array_get(storage->data.cols, i);
        if (strcmp(field->buffer, by_column) == 0) { 
            return i;
        }
    }
    return -1;  
}

static lp_dtype get_column_dtype(lp_storage_t *storage, int column_index) {
    lp_field_t *column_field = dynamic_array_get(storage->data.cols, column_index);
    return column_field->dtype;
}

lp_storage_t* lp_sort_values(lp_storage_t *storage, const char *by_column, lp_bool ascending) {
    int column_index = get_column_index(storage, by_column);
    
    if (column_index < 0) {
        return NULL;
    }

    lp_dtype dtype = get_column_dtype(storage, column_index);
    dynamic_array *column_data = storage->data.cols[column_index]; 
    
    int n = dynamic_array_size(column_data);
    
    tim_sort(column_data, n, dtype);

    if (!ascending) {
        for (int i = 0; i < n / 2; i++) {
            void *temp = dynamic_array_get(column_data, i);
            dynamic_array_set(column_data, i, dynamic_array_get(column_data, n - i - 1));
            dynamic_array_set(column_data, n - i - 1, temp);
        }
    }
    
    return storage;
}
