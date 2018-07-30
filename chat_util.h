#ifndef CHATUTIL_H
#define CHATUTIL_H

#include <stddef.h>

#define MESSAGE_BUFFER_SIZE 1024

enum message_type { CHAT_CREATE, CHAT_ENTER, CHAT_DELETE, CHAT_REFRESH , CHAT_SPEAK, CHAT_QUIT,
    CHAT_NEW
};

typedef struct cvector {
    size_t item_size;
    size_t item_count;
    size_t max_items;
    void *ptr;
} cvector;

typedef struct chat_room {
    char name[256];
    int filefd;
    cvector *members;
} chat_room;

typedef struct chat_server_data {
    cvector *rooms;
} chat_server_data;

typedef struct chat_message_data {
    int type;
    char buffer[MESSAGE_BUFFER_SIZE];
} chat_message_data;


cvector *cvector_init(size_t item_size, size_t max_items);
void cvector_push(cvector *vec, void *item);
void *cvector_get(cvector *vec, size_t index);
void cvector_remove_at(cvector *vec, size_t index);
int cvector_index_of(cvector *vec, void *item);
void cvector_remove(cvector *vec, void *item);
void cvector_free(cvector *vec);

int array_int_exists(int *array, int item, int array_len);
int is_regular_file(const char* path);
char *str_join(const char *str1, const char *str2);

chat_server_data *init_server_data(const char *dir_path);

void chat_message_serialize(const chat_message_data *data, char *send_buffer);
void chat_message_deserialize(chat_message_data *data, const char *recv_buffer);


#endif
