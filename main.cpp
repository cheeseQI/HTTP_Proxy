#include "Socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
int main(int argc, char *argv[]) {   
    // socket variables
    struct addrinfo hints, *address;
    int state;
    memset(&hints, 0, sizeof(hints));
    // support both ipv4 and ipv6
    hints.ai_family = AF_UNSPEC;
    // tcp stream socket
    hints.ai_socktype = SOCK_STREAM;
    // fill in my ip
    hints.ai_flags = AI_PASSIVE;
    //todo: should use "http" port
    if ((state = getaddrinfo(NULL, "8080", &hints, &address)) != 0) {
        perror("cannot get hostname correctly");
        exit(EXIT_FAILURE);
    }  
    Socket skt(address);

    return EXIT_SUCCESS; 
}