CXX = g++
CXXFLAGS = -Wall -std=c++14 -pedantic -g
LDFLAGS = -pthread -luuid

SRCS = main.cpp Socket.cpp Server.cpp ThreadPool.cpp Client.cpp
OBJS = $(SRCS:.cpp=.o)

all: main

main: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o main $(LDFLAGS)

main.o: main.cpp Socket.h Server.h ThreadPool.h
	$(CXX) $(CXXFLAGS) -c main.cpp $(LDFLAGS)

Socket.o: Socket.cpp Socket.h
	$(CXX) $(CXXFLAGS) -c Socket.cpp $(LDFLAGS)

Server.o: Server.cpp Server.h Socket.h ThreadPool.h
	$(CXX) $(CXXFLAGS) -c Server.cpp $(LDFLAGS)

ThreadPool.o: ThreadPool.cpp ThreadPool.h
	$(CXX) $(CXXFLAGS) -c ThreadPool.cpp $(LDFLAGS)

Client.o: Client.cpp Client.h
	$(CXX) $(CXXFLAGS) -c Client.cpp $(LDFLAGS)

clean:
	rm -f $(OBJS) main
