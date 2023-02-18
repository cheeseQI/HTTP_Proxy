#ifndef ERROR_H
#define ERROR_H 

#include <iostream>
#include <exception>
using namespace std;

class MyException : public exception {
public:    
    const char* what() const throw() {
        return "my test exception";
    }
};


class ProxyHostAddressException : public exception {
public:    
    const char* what() const throw() {
        return "cannot get host adrress correctly";
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


#endif