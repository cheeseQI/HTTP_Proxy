#include "Socket.h"
#include "ExceptionHandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
int main(int argc, char *argv[]) {   
    // socket variables
    struct addrinfo hints, *address;
    int state;
    memset(&hints, 0, sizeof(hints));
    // support ipv4 and ipv6， tcp stream socket， fill in self-ip
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    try {
        // todo: should use "http" port
        if ((state = getaddrinfo(NULL, "8080", &hints, &address)) != 0) {
            throw ProxyHostAddressException();
        }  
        Socket listener(address);
        
    } catch (ProxyHostAddressException &e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (SokectBuildException &e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (SokectSetException &e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS; 
}