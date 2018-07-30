#ifndef CHATDATA_H
#define CHATDATA_H
#include "chat_util.h"

#define MESSAGE_BUFFER_SIZE 1024

enum message_type { CHAT_CREATE, CHAT_ENTER, CHAT_DELETE, CHAT_REFRESH , CHAT_SPEAK, CHAT_QUIT,
    CHAT_NEW
};

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


chat_server_data *init_server_data(const char *dir_path);

void chat_message_serialize(const chat_message_data *data, char *send_buffer);
void chat_message_deserialize(chat_message_data *data, const char *recv_buffer);

#endif
