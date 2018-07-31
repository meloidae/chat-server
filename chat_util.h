#ifndef CHATUTIL_H
#define CHATUTIL_H

#include <stddef.h>
#include <time.h>

#define BUFFER_SIZE 1028
#define MESSAGE_BUFFER_SIZE 512
#define TIME_STR_SIZE 25

enum message_type { CHAT_CLI_MESSAGE, CHAT_CLI_QUIT,
    CHAT_REMOVE_MEMBER, CHAT_NEW_MEMBER, CHAT_NEW_MESSAGE
};

typedef struct cvector {
    size_t item_size;
    size_t item_count;
    size_t max_items;
    void *ptr;
} cvector;

typedef struct chat_data {
    int type;
    char time_str[TIME_STR_SIZE];
    char buffer[MESSAGE_BUFFER_SIZE];
} chat_data;

typedef struct chat_user {
    int sockfd;
    size_t name_size;
    char *name;
} chat_user;

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

//chat_server_data *init_server_data(const char *dir_path);

int chat_data_serialize(const chat_data *data, char *send_buffer);
void chat_data_deserialize(chat_data *data, const char *recv_buffer);
void chat_data_set(chat_data *data, int type, chat_user *user);

void add_user(cvector *users, int sockfd);
void remove_user(cvector *users, int sockfd);
chat_user *find_user(cvector *users, int sockfd);
void broadcast_to_users(cvector *users, const chat_data * data, char *send_buffer);
void pack_recv_data_to_send_data(chat_data *send_data, const chat_data *recv_data, chat_user *user);
void get_time_str(char *time_str);
#endif
