#ifndef SOCKET_H
#define SOCKET_H 
class Socket {
private:
    int socketFd;
public:
    Socket(struct addrinfo * address);
    ~Socket();
    int getFd();
}; 
#endif