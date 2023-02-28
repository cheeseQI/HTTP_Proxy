#ifndef SafeLog_H
#define SafeLog_H 

#include "ExceptionHandler.h"
#include "mutex"
#include "thread"

class SafeLog {
private:
    ofstream logFile;
    mutex fileMutex;
public:
    SafeLog(string path) : logFile(path), fileMutex() {
        checkValid();
    }

    void writeRequestToLog(string uuidStr, string firstLine, string host) {
        checkValid();
        lock_guard<mutex> lock(fileMutex);
        logFile << uuidStr << ": Requesting \"" << firstLine << "\" from " << host << " @ " << timeStap() << endl;
    }

    void writeServerResponseToLog(string uuidStr, string firstLine, string host) {
        checkValid();
        lock_guard<mutex> lock(fileMutex);
        logFile << uuidStr << ": Received \"" << firstLine << "\" from " << host << endl;
    }

    void writeResponseToClientToLog(string uuidStr, string firstLine) {
        checkValid();
        lock_guard<mutex> lock(fileMutex);
        logFile << uuidStr << ": Responding \"" << firstLine << "\"" << endl;
    }

    void writeTunnelClosedLog(string uuidStr) {
        checkValid();
        lock_guard<mutex> lock(fileMutex);
        logFile << uuidStr <<": Tunnel closed" << endl;
    }

    void writeCacheHitLog(string uuidStr, string firstLine) {
        checkValid();
        lock_guard<mutex> lock(fileMutex);
        logFile << uuidStr <<": in cache, valid" << endl;
        logFile << uuidStr <<": Responding \"" << firstLine << "\"" << endl;
    }

    void writeCacheStoreSuccessLog(string uuidStr, string hint) {
        checkValid();
        lock_guard<mutex> lock(fileMutex);
        logFile << uuidStr <<": cached, " << hint << endl;
    }

    void writeCacheStoreFailedLog(string uuidStr, string hint) {
        checkValid();
        lock_guard<mutex> lock(fileMutex);
        logFile << uuidStr <<": not cacheable because " << hint << endl;
    }

    string timeStap() {
        time_t t = time(nullptr);
        vector<char> buffer(80);
        strftime(&buffer.data()[0], 80, "%a %b %e %T %Y", gmtime(&t));
        return string(buffer.begin(), buffer.end());
    }

    void checkValid() {
        if (!logFile.is_open()) {
            throw FileOpenException();
        }
    }
};

#endif