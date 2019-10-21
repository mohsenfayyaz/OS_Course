# Define required macros here
SHELL = /bin/sh

all: server.o client.o

server.o: server.c
	gcc server.c -o Server/server

client.o: client.c
	gcc client.c -o client
	gcc client.c -o Client1/client
	gcc client.c -o Client2/client

clean:
	-rm -f *.o

