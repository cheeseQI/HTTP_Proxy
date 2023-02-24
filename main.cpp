#include "Socket.h"
#include "Server.h"
#include "ExceptionHandler.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {   
    struct addrinfo hints, *address;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    try {
        // todo: should use "http" port
        if (getaddrinfo(NULL, "8080", &hints, &address) != 0) {
            throw ProxyHostAddressException();
        }  
        Server proxyServer(address);
        proxyServer.run();
    } catch (exception &e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    } 
    return EXIT_SUCCESS; 
}