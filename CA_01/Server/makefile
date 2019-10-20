# Define required macros here
SHELL = /bin/sh

all: server.o client.o

server.o: server.c
	gcc server.c -o Server/server

client.o: client.c
	gcc client.c -o client

clean:
	-rm -f *.o

