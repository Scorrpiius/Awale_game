CC = gcc

OBJ = awale.o socket_client.o socket_server.o 
EXEC = client awale server

all: $(EXEC)

client : socket_client.o 
		$(CC) -o client socket_client.o

server : socket_server.o 
		$(CC) -o server socket_server.o

awale : awale.o
		$(CC) -o awale awale.o

socket_client.o : socket_client.c
		$(CC) -c socket_client.c 

socket_server.o : socket_server.c
		$(CC) -c socket_server.c 

awale.o : awale.c
		$(CC) -c awale.c 

clean :
		rm *.o client server awale
	
