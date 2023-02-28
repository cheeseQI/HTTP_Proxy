#ifndef ERROR_H
#define ERROR_H 

#include "iostream"
#include "exception"
using namespace std;

class MyException : public exception {
public:    
    const char* what() const throw() {
        return "my test exception: ";
    }
};


class ProxyHostAddressException : public exception {
private:
    string errMsg;
public:    
    ProxyHostAddressException(const string& errMsg) : errMsg(errMsg) {}
    virtual const char* what() const throw() override {
        string msg = "cannot get host address correctly: ";
        msg += errMsg;
        return msg.c_str();
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
private:
    string errMsg;
public:    
    SendException(const string& errMsg) : errMsg(errMsg) {}
    const char* what() const throw() {
        string msg = "server send message failed: ";
        msg += errMsg;
        return msg.c_str();
    }
};

class RecvException : public exception {
public:    
    const char* what() const throw() {
        return "server receive message failed: ";
    }
};


class ParseException : public exception {
public:    
    const char* what() const throw() {
        return "parse message failed: ";
    }
};


class ConnectException : public exception {
public:    
    const char* what() const throw() {
        return "connect failed: ";
    }
};

class FileOpenException : public exception {
public:    
    const char* what() const throw() {
        return "open proxy file failed: ";
    }
};
#endif