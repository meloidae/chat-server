#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "chat_util.h"

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
    memmove(ptr + (vec->item_size * vec->item_count), item, vec->item_size);
    vec->item_count++;
} // cvector_push

void *cvector_get(cvector *vec, size_t index) {
    char *ptr = vec->ptr;
    return (void *)(ptr + (vec->item_size * index));
} // cvector_get

void cvector_remove_at(cvector *vec, size_t index) {
    char *ptr = vec->ptr;
    char *rm_ptr = ptr + (vec->item_size * index);
    char *cp_ptr = ptr + (vec->item_size * (index + 1));
    size_t cp_size = (vec->item_count - index) * vec->item_size;
    if (cp_size > 0) {
        memmove(rm_ptr, cp_ptr, cp_size);
    } // if
    vec->item_count--;
} // cvector_remove


int cvector_index_of(cvector *vec, void *item) {
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

void cvector_remove(cvector *vec, void *item) {
    int index = cvector_index_of(vec, item);
    if (index != -1) {
       cvector_remove_at(vec, index);
    } // if 
} // cvector_remove

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

int is_regular_file(const char* path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
} // is_regular_file

char *str_join(const char *str1, const char *str2) {
    char *joined;
    size_t strlen1 = strlen(str1);
    size_t strlen2 = strlen(str2);
    joined = malloc(strlen1 + strlen2 + 1);
    memset(joined, 0, strlen1 + strlen2 + 1);
    memmove(joined, str1, strlen1);
    memmove(joined + strlen1, str2, strlen2);
    return joined;
} // str_join

chat_server_data *init_server_data(const char *dir_path) {
    struct dirent *dp;
    chat_server_data *server_data = (chat_server_data *)malloc(sizeof(chat_server_data));
    DIR *dir = opendir(dir_path);
    while (dp = readdir(dir)) {
        char *full_path = str_join(dir_path, dp->d_name);
        if (is_regular_file(full_path)) {
            // Read room file 
        } // if
        fprintf(stderr, "%s\n", dp->d_name);
    } // while
    closedir(dir);
    return server_data;
} // chat_server_data

void chat_message_serialize(const chat_message_data *data, char *send_buffer) {
    memset(send_buffer, 0, MESSAGE_BUFFER_SIZE);

    char *ptr = send_buffer;
    memmove(ptr, &(data->type), sizeof(int));
    ptr += sizeof(int);

    int len = strlen(data->buffer);
    memmove(ptr, &len, sizeof(int));
    ptr += sizeof(int);
    
    if (len > 0) {
        memmove(ptr, data->buffer, len);
    } // if
} // chat_message_serialize

void chat_message_deserialize(chat_message_data *data, const char *recv_buffer) {
    memset(data, 0, sizeof(chat_message_data));

    const char *ptr = recv_buffer;
    int type = -1;
    memmove(&type, ptr, sizeof(int));
    data->type = type;
    ptr += sizeof(int);

    int len = -1;
    memmove(&len, ptr, sizeof(int));
    ptr += sizeof(int);    

    if (len > 0) {
        memmove(data->buffer, ptr, len);
    } // if 
} // chat_message_deserialize

