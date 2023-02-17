#include "Socket.h"
#include "iostream"
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
using namespace std;


Socket::Socket(struct addrinfo * address) {
    if ((socketFd = socket(address->ai_family, address->ai_socktype, address->ai_protocol)) == -1) {
        perror("socket failed: ");
        exit(EXIT_FAILURE);
    }
    // set option for socket, it is socket(not tcp) level, can resuse(so restart at same port without throwing error)
    int FLAG = 1; 
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &FLAG, sizeof(int)) == -1) {
        perror("setsockopt failed: ");
        close(socketFd);
        exit(EXIT_FAILURE);
    }
    cout << "cool, the socket is generated with descriptor " << this->getFd() << endl; 
}

Socket::~Socket() {
    close(socketFd);
    cout << "cool, the socket is deleted with descriptor " << this->getFd() << endl; 
}

int Socket::getFd() {
    return socketFd;
} 