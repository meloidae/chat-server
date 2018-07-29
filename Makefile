CC=gcc
CFLAGS=-std=c99 

server: chat_server.c chat_util.c
	$(CC) $(CFLAGS) -o $@ $^ 

