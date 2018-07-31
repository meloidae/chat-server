#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>
#include "chat_util.h"
#include "chat_ui.h"


#define DEFAULT_PORT "31610"
#define DEFAULT_NODE "54.249.36.78"
#define EPOLL_FD_SIZE 100 // arbitrary
#define EPOLL_MAX_EVENTS 100

int connect_socket(char *node_name, char *service_name) {
    struct addrinfo hints;
    struct addrinfo *address_info;
    int rc, sockfd;

    // Obtain address info
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    rc = getaddrinfo(node_name, service_name, &hints, &address_info);
    if (rc < 0) {
        perror("getaadrinfo");
        exit(1);
    } // if

    // Prepare socket
    sockfd = socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    } // if

    rc = connect(sockfd, address_info->ai_addr, address_info->ai_addrlen);
    if (rc < 0) {
        perror("connect");
        exit(1);
    } // if

    return sockfd;
} // init_socket


void send_to_server(chat_ui *ui, chat_data *data, int sockfd, char *send_buffer) {
    // Get time
    int rc;
    size_t nbytes;

    memset(data->time_str, 0, TIME_STR_SIZE);
    memset(data->buffer, 0, MESSAGE_BUFFER_SIZE);

    data->type = CHAT_CLI_MESSAGE;
    get_time_str(data->time_str);

    memmove(data->buffer, ui->buffer, ui->buf_size); 
    nbytes = chat_data_serialize(data, send_buffer);

    rc = write(sockfd, send_buffer, nbytes); 
    if (rc < 0) {
        perror("write");
        exit(1);
    } // if
} // send_to_server

void client_loop(chat_ui *ui, int sockfd) {
    int ch, flag, epfd;
    struct epoll_event ev, ev_ret[EPOLL_MAX_EVENTS];
    char send_buffer[BUFFER_SIZE];
    char recv_buffer[BUFFER_SIZE];
    chat_data send_data;
    chat_data recv_data;

    // Initialize epoll, register stdin & socket
    epfd = epoll_create(EPOLL_MAX_EVENTS);
    if (epfd < 0) {
        perror("epoll_create");
        exit(1);
    } // if
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) < 0) {
        perror("epoll_ctl");
        exit(1);
    } // if

    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev) < 0) {
        perror("epoll_ctl");
        exit(1);
    } // if

    print_ui_status(ui);
    while (1) {
        int num_fds = epoll_wait(epfd, ev_ret, EPOLL_MAX_EVENTS, -1);
        if (num_fds < 0) {
            perror("epoll_wait");
            exit(1);
        } // if
        for (int i = 0; i < num_fds; i++) {
            int target_fd = ev_ret[i].data.fd;
            if (target_fd == STDIN_FILENO) { // handle getch
                ch = getch();
                flag = handle_key(ui, ch);
                if (flag == FLAG_SEND) {
                    send_to_server(ui, &send_data, sockfd, send_buffer);
                    reset_ui_buffer(ui);
                } else if (flag == FLAG_QUIT) {
                    break;
                } // else if
            } else { // handle incoming data from server
                memset(recv_buffer, 0, BUFFER_SIZE);
                int num_read = read(target_fd, recv_buffer, BUFFER_SIZE);
                if (num_read < 0) {
                    perror("read");
                    exit(1);
                } else if (num_read == 0) { // Connection is closed
                    if (epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, &ev_ret[i]) < 0) {
                       perror("epoll_ctl_del");
                       exit(1);
                    } // if
                    flag = FLAG_QUIT;
                    break;
                } // else if
                chat_data_deserialize(&recv_data, recv_buffer);
                print_ui_main(ui, recv_data.time_str, recv_data.buffer);
            } // else
        } // for
        if (flag == FLAG_QUIT) {
            break;
        } // if
    } // while

} // client_loop

int main(int argc, char *argv[]) {
    int sockfd;
    int max_rows, max_cols;
    char *node_name, *service_name;
    chat_ui *ui;
    // Deal with commandline arguements

    node_name = argv[1];
    service_name = argv[2];
    if (argc < 2) {
        fprintf(stderr, "server address is set to default: %s\n", DEFAULT_NODE);
        node_name = DEFAULT_NODE;
    } else {
        node_name = argv[1]; 
    } // else
    if (argc < 3) {
        fprintf(stderr, "server port is set to default: %s\n", DEFAULT_PORT);
        service_name = DEFAULT_PORT;
    } else {
        service_name = argv[2];
    } // else

    sockfd = connect_socket(node_name, service_name);
    
    fprintf(stderr, "Connected: sock=%d\n", sockfd);


    ui = init_ui(MESSAGE_BUFFER_SIZE);
    client_loop(ui, sockfd);

    destroy_ui(ui);
    close(sockfd);
} // main

