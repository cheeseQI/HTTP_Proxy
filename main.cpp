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
        int status;
        if ((status = getaddrinfo(NULL, "8080", &hints, &address)) != 0) {
            string serverInfo("proxy server: ");
            throw ProxyHostAddressException(serverInfo + gai_strerror(status));
        }  
        Server proxyServer(address);
        proxyServer.run();
    } catch (exception &e) {
        cout << e.what() << endl;
        return EXIT_FAILURE;
    } 
    return EXIT_SUCCESS; 
}