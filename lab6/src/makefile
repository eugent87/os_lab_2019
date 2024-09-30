CC=gcc
CFLAGS=-I. -pthread

objects = server client start_servers

all: $(objects)

server: server.o multmodulo.o
	$(CC) server.o multmodulo.o -o server $(CFLAGS)

client: client.o multmodulo.o
	$(CC) client.o multmodulo.o -o client $(CFLAGS)

start_servers:
	./server --port 20001 --tnum 5 &
	./server --port 20002 --tnum 5 &

start:
	./client --k 10 --mod 19 --servers servers.txt

server.o: server.c
	$(CC) -c server.c -o server.o $(CFLAGS)

client.o: client.c
	$(CC) -c client.c -o client.o $(CFLAGS)

common.o: multmodulo.c multmodulo.h
	$(CC) -c multmodulo.c -o multmodulo.o $(CFLAGS)

clean:
	rm -f client server *.o
	fuser -k 20001/tcp
	fuser -k 20002/tcp
