CC=gcc
CFLAGS=



util.o: chat_util.c
	$(CC) -c -o $@ $^

server: chat_server.c util.o
	$(CC) -o $@ $^

client: chat_client.c util.o
	$(CC) -o $@ $^ -lncurses

clean:
	-rm *.o server client
