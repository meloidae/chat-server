#include "chat_util.h"

#include <stdlib.h>
#include <string.h>

cvector *cvector_init(size_t item_size, size_t max_items) {
    cvector *vec = (cvector *)malloc(sizeof(cvector));
    vec->ptr = malloc(item_size * max_items);
    vec->item_size = item_size;
    vec->item_count = 0;
    vec->max_items = max_items;
    return vec;
} // cvector_init

void cvector_push(cvector *vec, void *item) {
    if (vec->item_count == vec->max_items) {
        // Increase vector size
        void *tmp = realloc(vec ->ptr, vec->item_size * vec->max_items * 2); 
        vec->ptr = tmp;
        vec->max_items *= 2;
    } // if
    char *ptr = vec->ptr;
    memcpy(ptr + (vec->item_size * vec->item_count), item, vec->item_size);
    vec->item_count++;
} // cvector_push

void *cvector_get(cvector *vec, size_t index) {
    char *ptr = vec->ptr;
    return (void *)(ptr + (vec->item_size * index));
} // cvector_get

void cvector_remove(cvector *vec, size_t index) {
    char *ptr = vec->ptr;
    char *rm_ptr = ptr + (vec->item_size * index);
    char *cp_ptr = ptr + (vec->item_size * (index + 1));
    size_t cp_size = (vec->item_count - index) * vec->item_size;
    if (cp_size > 0) {
        memcpy(rm_ptr, cp_ptr, cp_size);
    } // if
    vec->item_count--;
} // cvector_remove


int cvector_index_of(cvector *vec, void*item) {
    char *ptr = vec->ptr;
    for (size_t i = 0; i < vec->item_count; i++) {
        int comp = memcmp(ptr, item, vec->item_size);
        if (comp == 0) {
            return i;
        } // if
        ptr += vec->item_size;
    } // for
    return -1;
} // cvector_index_of

void cvector_free(cvector *vec) {
    free(vec->ptr);
    free(vec);
} // cvector_free


int array_int_exists(int *array, int item, int array_len) {
    for (int i = 0; i < array_len; i++) {
        if (array[i] == item) {
            return 1;
        } // if
    } // for
    return 0;
} // array_int_exists
