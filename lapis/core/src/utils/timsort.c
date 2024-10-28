#include "timsort.h"
#include <stdlib.h>
#include <string.h>

#define MIN_RUN 32

static int compare(const void *a, const void *b, lp_dtype dtype) {
    if (dtype == INT) {
        return (*(int*)a > *(int*)b) - (*(int*)a < *(int*)b);
    } else if (dtype == FLOAT) {
        return (*(float*)a > *(float*)b) - (*(float*)a < *(float*)b);
    } else {
        return strcmp((char*)a, (char*)b);
    }
}

static void insertion_sort(dynamic_array *array, size_t left, size_t right, lp_dtype dtype) {
    for (size_t i = left + 1; i <= right; i++) {
        void *key = dynamic_array_get(array, i);
        size_t j = i;
        while (j > left && compare(dynamic_array_get(array, j - 1), key, dtype) > 0) {
            dynamic_array_set(array, j, dynamic_array_get(array, j - 1));
            j--;
        }
        dynamic_array_set(array, j, key);
    }
}

static void merge(dynamic_array *array, size_t left, size_t mid, size_t right, lp_dtype dtype) {
    size_t len1 = mid - left + 1;
    size_t len2 = right - mid;

    void **left_array = malloc(len1 * sizeof(void *));
    void **right_array = malloc(len2 * sizeof(void *));

    for (size_t i = 0; i < len1; i++) left_array[i] = dynamic_array_get(array, left + i);
    for (size_t i = 0; i < len2; i++) right_array[i] = dynamic_array_get(array, mid + 1 + i);

    size_t i = 0, j = 0, k = left;
    while (i < len1 && j < len2) {
        if (compare(left_array[i], right_array[j], dtype) <= 0) {
            dynamic_array_set(array, k++, left_array[i++]);
        } else {
            dynamic_array_set(array, k++, right_array[j++]);
        }
    }

    while (i < len1) dynamic_array_set(array, k++, left_array[i++]);
    while (j < len2) dynamic_array_set(array, k++, right_array[j++]);

    free(left_array);
    free(right_array);
}

dynamic_array* lp_tim_sort(dynamic_array *array, size_t size, lp_dtype dtype) {
    for (size_t i = 0; i < size; i += MIN_RUN) {
        size_t end = (i + MIN_RUN - 1 < size - 1) ? i + MIN_RUN - 1 : size - 1;
        insertion_sort(array, i, end, dtype);
    }

    for (size_t size_run = MIN_RUN; size_run < size; size_run *= 2) {
        for (size_t left = 0; left < size; left += 2 * size_run) {
            size_t mid = left + size_run - 1;
            size_t right = (left + 2 * size_run - 1 < size - 1) ? left + 2 * size_run - 1 : size - 1;

            if (mid < right) {
                merge(array, left, mid, right, dtype);
            }
        }
    }

    return array;
}
