CC=g++

CFLAGS=-c

all:
	$(CC) $(CFLAGS) datastore.cpp
	$(CC) $(CFLAGS) dataparse.cpp
	$(CC) $(CFLAGS) network.cpp
	$(CC) $(CFLAGS) main.cpp
	$(CC) main.o network.o datastore.o dataparse.o -o main -lssl

clean:
	rm -rf *.o main
