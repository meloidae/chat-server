#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "chat_data.h"


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
