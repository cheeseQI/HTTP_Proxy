CC=g++
CFLAGS=-Wall -std=c++14 -pedantic 
main: main.cpp
	$(CC) $(CFLAGS) -g -o main main.cpp Socket.cpp Server.cpp
clean:
	\rm -f main