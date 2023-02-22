#include "Socket.h"
#include "Server.h"
#include "ExceptionHandler.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {   
    // socket variables
    struct addrinfo hints, *address;
    memset(&hints, 0, sizeof(hints));
    // support ipv4 and ipv6， tcp stream socket， fill in self-ip
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    try {
        // todo: should use "http" port
        if (getaddrinfo(NULL, "8080", &hints, &address) != 0) {
            throw ProxyHostAddressException();
        }  
        Server proxyServer(address);
        freeaddrinfo(address);
        proxyServer.run();
        //unique_ptr<Socket>& test_ptr = proxyServer.getListenSocketPtr();
        //cout << "test for socket " << test_ptr->getFd() << " availability" << endl;
    } catch (exception &e) { // todo: will be all changed to catch exception&e, if no special case
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    } 
    return EXIT_SUCCESS; 
}