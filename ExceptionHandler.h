#ifndef ERROR_H
#define ERROR_H 

#include <iostream>
#include <exception>
using namespace std;

class MyException : public exception {
public:    
    const char* what() const throw() {
        return "my test exception: ";
    }
};


class ProxyHostAddressException : public exception {
public:    
    const char* what() const throw() {
        return "cannot get host adrress correctly: ";
    }
};


class SokectBuildException : public exception {
public:    
    const char* what() const throw() {
        return "socket failed:";
    }
};


class SokectSetException : public exception {
public:    
    const char* what() const throw() {
        return "setsockopt failed: ";
    }
};

class ServerBindException : public exception {
public:    
    const char* what() const throw() {
        return "server bind to socket failed: ";
    }
};


class ServerListenException : public exception {
public:    
    const char* what() const throw() {
        return "server listen failed: ";
    }
};


class SelectException : public exception {
public:    
    const char* what() const throw() {
        return "server select failed: ";
    }
};


class AcceptException : public exception {
public:    
    const char* what() const throw() {
        return "server accept failed: ";
    }
};

class SendException : public exception {
public:    
    const char* what() const throw() {
        return "server send message failed: ";
    }
};

class RecvException : public exception {
public:    
    const char* what() const throw() {
        return "server receive message failed: ";
    }
};

#endif