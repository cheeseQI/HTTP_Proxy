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
    string statusCode;
    string statusText;
    string body;
    string date;
    string cacheControl;
    string expire;
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
            } else if (line.find("Date: ") == 0) {
                this->date = line.substr(6);
            } else if (line.find("Cache-Control: ") == 0) {
                this->cacheControl = line.substr(15);
            } else if (line.find("Expire: ") == 0) {
                this->expire = line.substr(8);
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
        this->isChunked = false;
        this->contentLength = -1;
        this->date = "";
        this->cacheControl = "";
        this->expire = "";
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
    
    string getStatusCode() {
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

    bool canCache() {
        return (cacheControl != "" && cacheControl.find("private") == string::npos 
        && cacheControl.find("no-cache") == string::npos && cacheControl.find("no-store") == string::npos);
    }

    string getDate() {
        return date;
    }

    string getCacheControl() {
        return cacheControl;
    }

    string getExpire() {
        return expire;
    }
};
#endif
