CC=gcc
CFLAGS=



util.o: chat_util.c
	$(CC) -c -o $@ $^

ui.o: chat_ui.c
	$(CC) -c -o $@ $^ -lncurses

server: chat_server.c util.o
	$(CC) -o $@ $^

client: chat_client.c util.o ui.o
	$(CC) -o $@ $^ -lncurses

clean:
	-rm *.o server client
