#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <ncurses.h>
#include "chat_util.h"


#define DEFAULT_PORT "31610"
#define DEFAULT_NODE "54.249.36.78"

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

void init_ncurses() {
    initscr();
    raw();
    noecho();
} // init_ncurses

void terminate_ncurses() {
    endwin();
} // terminate_ncurses

void client_loop(int sockfd) {
    int ch;
    char buffer[MESSAGE_BUFFER_SIZE];
    while (1) {
        ch = getch();
        if (ch == 'q') {
            break;
        } else {
            printw("Key pressed: %c", ch);
            refresh();
        } // else
    } // while
} // client_loop

int main(int argc, char *argv[]) {
    int sockfd;
    char *node_name, *service_name;
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

    init_ncurses();

    terminate_ncurses();
    close(sockfd);
} // main

