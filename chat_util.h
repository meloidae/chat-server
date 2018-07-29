#ifndef CHATUTIL_H
#define CHATUTIL_H

#include <stddef.h>

typedef struct cvector {
    size_t item_size;
    size_t item_count;
    size_t max_items;
    void *ptr;
} cvector;

cvector *cvector_init(size_t item_size, size_t max_items);
void cvector_push(cvector *vec, void *item);
void *cvector_get(cvector *vec, size_t index);
void cvector_remove(cvector *vec, size_t index);
int cvector_index_of(cvector *vec, void*item);

int array_int_exists(int *array, int item, int array_len);


#endif
