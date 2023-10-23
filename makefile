CC = gcc
CCFLAGS = -w
OBJ = awale.o socket_client.o socket_server.o 
EXEC = client server

all: $(EXEC)

awale : awale.o
		$(CC) -o awale awale.o $(CCFLAGS)

awale.o : awale.c awale.h
		$(CC) -c awale.c awale.h $(CCFLAGS)

socket_client.o : socket_client.c
		$(CC) -c socket_client.c $(CCFLAGS)

socket_server.o : socket_server.c awale.h
		$(CC) -c socket_server.c awale.h $(CCFLAGS)

client : socket_client.o 
		$(CC) -o client socket_client.o $(CCFLAGS)

server : socket_server.o awale.o
		$(CC) -o server socket_server.o awale.o $(CCFLAGS)

clean :
		rm *.o client server
	
