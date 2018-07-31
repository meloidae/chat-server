#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

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

// chat_server_data *init_server_data(const char *dir_path) {
//     struct dirent *dp;
//     chat_server_data *server_data = (chat_server_data *)malloc(sizeof(chat_server_data));
//     DIR *dir = opendir(dir_path);
//     while (dp = readdir(dir)) {
//         char *full_path = str_join(dir_path, dp->d_name);
//         if (is_regular_file(full_path)) {
//             // Read room file 
//         } // if
//         fprintf(stderr, "%s\n", dp->d_name);
//     } // while
//     closedir(dir);
//     return server_data;
// } // chat_server_data

int chat_data_serialize(const chat_data *data, char *send_buffer) {
    memset(send_buffer, 0, MESSAGE_BUFFER_SIZE);
    int32_t type = data->type;
    char *ptr = send_buffer;
    memmove(ptr, &type, sizeof(int32_t));
    ptr += sizeof(int32_t);

    memmove(ptr, data->time_str, TIME_STR_SIZE);
    ptr += TIME_STR_SIZE;

    int32_t len = strlen(data->buffer);
    memmove(ptr, &len, sizeof(int32_t));
    ptr += sizeof(int32_t);
    
    if (len > 0) {
        memmove(ptr, data->buffer, len);
    } // if
    return sizeof(int32_t) * 2 + TIME_STR_SIZE + len + 1;
} // chat_message_serialize

void chat_data_deserialize(chat_data *data, const char *recv_buffer) {
    memset(data, 0, sizeof(chat_data));

    const char *ptr = recv_buffer;
    int32_t type = -1;
    memmove(&type, ptr, sizeof(int32_t));
    data->type = type;
    ptr += sizeof(int32_t);

    memmove(data->time_str, ptr, TIME_STR_SIZE);
    ptr += TIME_STR_SIZE;

    int32_t len = -1;
    memmove(&len, ptr, sizeof(int32_t));
    ptr += sizeof(int32_t);    

    if (len > 0) {
        memmove(data->buffer, ptr, len);
    } // if 
} // chat_message_deserialize

void chat_data_set(chat_data *data, int type, chat_user *user) {
    // Get time
    size_t nbytes;

    memset(data->time_str, 0, TIME_STR_SIZE);
    memset(data->buffer, 0, MESSAGE_BUFFER_SIZE);

    data->type = type;
    get_time_str(data->time_str);
    switch(type) {
        case CHAT_NEW_MEMBER:
            sprintf(data->buffer, "(%s) entered the chat.", user->name);
            break;
        case CHAT_REMOVE_MEMBER:
            sprintf(data->buffer, "(%s) left the chat.", user->name);
            break;
        default:
            break;
    } // switch
} // chat_data_set

void add_user(cvector *users, int sockfd) {
    chat_user *user = (chat_user *)malloc(sizeof(chat_user));
    user->sockfd = sockfd;
    user->name_size = 25;
    user->name = (char *)malloc(user->name_size);
    memset(user->name, 0, user->name_size);
    sprintf(user->name, "user%d", sockfd);
    cvector_push(users, user);
    fprintf(stderr, "Added %s\n", user->name);
} // add_user

void remove_user(cvector *users, int sockfd) {
    chat_user *user;
    for (int i = 0; i < users->item_count; i++) {
        user = (chat_user *)cvector_get(users, i);
        if (user->sockfd == sockfd) {
            fprintf(stderr, "Removed %s\n", user->name);
            free(user->name);
            cvector_remove_at(users, i);
            break;
        } // if
    } // for
} // remove_user

chat_user *find_user(cvector *users, int sockfd) {
    chat_user *user;
    for (int i = 0; i < users->item_count; i++) {
        user = (chat_user *)cvector_get(users, i);
        if (user->sockfd == sockfd) {
            return user;
        } // if
    } // for
} // find_user

void broadcast_to_users(cvector *users, const chat_data * data, char *send_buffer) {
    int rc;
    chat_user *user;
    size_t nbytes = chat_data_serialize(data, send_buffer);
    for (int i = 0; i < users->item_count; i++) {
        user = (chat_user *)cvector_get(users, i);
        rc = write(user->sockfd, send_buffer, nbytes);
    } // for
} // broadcast_to_users

void pack_recv_data_to_send_data(chat_data *send_data, const chat_data *recv_data, chat_user *user) {
    size_t name_len;
    size_t max_copy_len;
    switch(recv_data->type) {
        case CHAT_CLI_MESSAGE:
            memset(send_data, 0, sizeof(chat_data));
            send_data->type = CHAT_NEW_MESSAGE;
            memmove(send_data->time_str, recv_data->time_str, TIME_STR_SIZE); 
            name_len = strlen(user->name);
            sprintf(send_data->buffer, "(%s): ", user->name);
            max_copy_len = MESSAGE_BUFFER_SIZE - name_len - 4;
            memmove((send_data->buffer + name_len + 4), recv_data->buffer, max_copy_len); 
            break;
        default:
            break;
    } // switch
} // pack_recv_data_to_send_data

void get_time_str(char *time_str) {
    time_t current_time;
    struct tm *time_info;
    time(&current_time);
    time_info = localtime(&current_time); 
    strftime(time_str, TIME_STR_SIZE, "%Y/%m/%d %H:%M:%S", time_info);
} // get_time_str
