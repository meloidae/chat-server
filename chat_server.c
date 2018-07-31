/************************************** 
 *
 *
 *
 *
 *
 *
 **************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include "chat_util.h"


#define DEFAULT_PORT "31610"
#define DEFAULT_BACKLOG 5
#define SERVER_SOCK_SIZE 5 
#define INITIAL_CLIENT_SOCK_SIZE 5
#define EPOLL_FD_SIZE 100 // arbitrary
#define EPOLL_MAX_EVENTS 100

typedef struct clientinfo {
    int fd;
    int status;
} clientinfo;


// Initialize the server socket and returns its fd
int init_server_sockets(char *port, int backlog, cvector *server_socks) {
    int sock, num_sock, yes = 1;
    struct addrinfo hints, *ai, *ai0;
    
    // Initialize addrinfo hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    // Get address info
    int g;
    if (g = getaddrinfo(NULL, port, &hints, &ai0)) {
        fprintf(stderr, "%s\n", gai_strerror(g));
        exit(1);
    } // if

    // Bind sockets
    for (ai = ai0, num_sock = 0; ai != NULL; ai = ai->ai_next) {
        sock = socket(ai->ai_family, ai->ai_socktype, 0);
        if (sock < 0) {
            perror("socket");
            continue;
        } // if
        setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes));
        if (bind(sock, (struct sockaddr *)ai->ai_addr, ai->ai_addrlen) < 0) {
            perror("bind");
            close(sock);
            continue;
        } // if
        if (listen(sock, backlog) < 0) {
            perror("listen");
            close(sock);
            continue;
        } // if
        cvector_push(server_socks, &sock);
        num_sock++;
    } // while

    if (num_sock == 0) {
        perror("unable to create any server socket");
        exit(1);
    } // if
    return 0;

} // init_server_sockets

void server_loop(cvector *server_socks, cvector *users) {
    int epfd;
    socklen_t sock_len;
    struct sockaddr_storage sock_storage; 
    struct epoll_event ev, ev_ret[EPOLL_MAX_EVENTS];
    char recv_buffer[BUFFER_SIZE];
    char send_buffer[BUFFER_SIZE];
    chat_data send_data;
    chat_data recv_data;
    // Initialize epoll
    epfd = epoll_create(EPOLL_MAX_EVENTS);
    if (epfd < 0) {
        perror("epoll_create");
        exit(1);
    } // if
    
    // Register server sockets to epoll
    for (int i = 0; i < server_socks->item_count; i++) {
        memset(&ev, 0, sizeof(ev));
        ev.events = EPOLLIN;
        ev.data.ptr = malloc(sizeof(clientinfo));
        int fd = *((int *)cvector_get(server_socks, i));
        ((clientinfo *)ev.data.ptr)->fd = fd;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
           perror("epoll_ctl");
           exit(1);
        } // if 
    } // for
    
    // Infinite server loop
    while (1) {
        // Wait for events using epoll
        int num_fds = epoll_wait(epfd, ev_ret, EPOLL_MAX_EVENTS, -1);
        if (num_fds < 0) {
            perror("epoll_wait");
            exit(1);
        } // if

        fprintf(stderr, "After epoll_wait: num_fds =%d\n", num_fds);
        
        // Loop through activated fds
        for (int i = 0; i < num_fds; i++) {
            clientinfo *ci = ev_ret[i].data.ptr;
            fprintf(stderr, "Notfication on fd=%d\n", ci->fd);

            if (cvector_index_of(server_socks, &(ci->fd)) != -1) {
                // Accept an incoming connection
                sock_len = sizeof(sock_storage);
                int accept_sock = accept(ci->fd, (struct sockaddr *)&sock_storage, &sock_len);
                if (accept_sock < 0) {
                    perror("accept");
                    exit(1);
                } // if
                fprintf(stderr, "Accepted connection: fd=%d\n", accept_sock);

                // Register client socket to epoll
                memset(&ev, 0, sizeof(ev));
                ev.events = EPOLLIN;
                ev.data.ptr = malloc(sizeof(clientinfo));
                ((clientinfo *)ev.data.ptr)->fd = accept_sock;

                if (epoll_ctl(epfd, EPOLL_CTL_ADD, accept_sock, &ev) < 0) {
                    perror("epoll_ctl");
                    exit(1);
                } // if
                add_user(users, accept_sock);
                chat_data_set(&send_data, CHAT_NEW_MEMBER, find_user(users, accept_sock));
                broadcast_to_users(users, &send_data, send_buffer);
            } else {
                if (ev_ret[i].events & EPOLLIN) {
                    memset(recv_buffer, 0, sizeof(char) * BUFFER_SIZE);
                    int num_read = read(ci->fd, recv_buffer, BUFFER_SIZE);
                    if (num_read < 0) {
                        perror("read");
                        exit(1);
                    } else if (num_read == 0) { // Connection is closed
                        fprintf(stderr, "Closed connection: fd=%d\n", ci->fd);
                        chat_data_set(&send_data, CHAT_REMOVE_MEMBER, find_user(users, ci->fd));
                        broadcast_to_users(users, &send_data, send_buffer);
                        remove_user(users, ci->fd);
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, ci->fd, &ev_ret[i]) < 0) {
                           perror("epoll_ctl_del");
                           exit(1);
                        } // if
                        continue;
                    } // else if
                    fprintf(stderr, "Read from client: fd=%d\n", ci->fd);
                    chat_data_deserialize(&recv_data, recv_buffer);
                    chat_user *user = find_user(users, ci->fd);
                    pack_recv_data_to_send_data(&send_data, &recv_data, user);
                    broadcast_to_users(users, &send_data, send_buffer);
                    fprintf(stderr, "Message (time: %s): %s\n", recv_data.time_str, recv_data.buffer);
                } // if
            } // else
        } // for
    } // while
    
} // server_loop

int main(int argc, char *argv[]) {
    char *port;
    int backlog;

    cvector *server_socks = cvector_init(sizeof(int), SERVER_SOCK_SIZE);
    cvector *users = cvector_init(sizeof(chat_user), INITIAL_CLIENT_SOCK_SIZE);

    // Read commandline arguments
    if (argc < 2) {
        fprintf(stderr, "the server port is set to default: %s\n", DEFAULT_PORT);
        port = DEFAULT_PORT;
    } else {
        port = argv[1];
    } // else

    if (argc < 3) {
        fprintf(stderr, "the backlog for listen() is set to default: %i\n", DEFAULT_BACKLOG);
        backlog = DEFAULT_BACKLOG;
    } else {
        backlog = atoi(argv[2]);
        if (backlog <= 0) {
            fprintf(stderr, "the backlog for listen() is invalid so it is set to default: %i\n", DEFAULT_BACKLOG);
        } // if
    } // else

    // Set up server sockets; initialize, bind & listen

    int ret = init_server_sockets(port, backlog, server_socks);

    server_loop(server_socks, users);

    cvector_free(users);
    cvector_free(server_socks);
} // main
