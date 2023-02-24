#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H    
#include "sstream"
#include "string"
#include "vector"
#include "iostream"
using namespace std;

class HttpResponse {
private:
    string header;
    string version;
    int statusCode;
    string statusText;
    string body;
    bool isChunked;
    int contentLength;
    // may add other element in headers

    void parseHeader() {
        vector<string> lines = split(header, "\r\n");
        for (string line : lines) {
            if (line.find("HTTP") == 0) {
                istringstream iss(line);
                string method, requestUri, httpVersion;
                iss >> httpVersion >> statusCode >> statusText;
                this->version = httpVersion;
                this->statusCode = statusCode;
                this->statusText = statusText;
            } else if (line.find("Transfer-Encoding: ") == 0 && line.substr(19) == "chunked") {
                // todo: need test;
                this->isChunked = true;
            } else if (line.find("Content-Length: ") == 0) {
                this->contentLength = stoi(line.substr(16));
            }
        }
    }

    vector<string> split(string s, string delimiter) {
        string line;
        vector<string> lines;
        size_t idx = 0;
        while ((idx = s.find(delimiter)) != string::npos) {
            line = s.substr(0, idx);
            lines.push_back(line);
            s.erase(0, idx + delimiter.length());
        }
        lines.push_back(s);
        return lines;
    }


public:
    HttpResponse(string message) {
        vector<string> parts = split(message, "\r\n\r\n");
        if (parts.size() > 0) {
            header = parts[0];
        }
        if (parts.size() > 1) {
            body = parts[1];
        }
        isChunked = false;
        parseHeader();
    }

    string getHeader() {
        return header;
    }

    string getBody() {
        return body;
    }

    string getVersion() {
        return version;
    }
    
    int getStatusCode() {
        return statusCode;
    }

    string getStatusText() {
        return statusText;
    }

    bool getIsChuncked() {
        return isChunked;
    }

    int getContentLength() {
        return contentLength;
    }
};
#endif