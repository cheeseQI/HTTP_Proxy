CC=g++
CFLAGS=-Wall -std=gnu++11 -pedantic 
main: main.cpp
	$(CC) $(CFLAGS) -g -o main main.cpp Socket.cpp
clean:
	\rm -f main