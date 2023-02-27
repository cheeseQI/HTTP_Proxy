#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H    
#include "sstream"
#include "string"
#include "vector"
using namespace std;

class HttpRequest {
private:
    string header;
    string firstLine;
    string method;
    string uri;
    string host;
    string body;
    string version;
    string date;
    // todo: may add others like connection, useragent, and they can all parse from header

    void parseHeader() {
        vector<string> lines = split(header, "\r\n");
        for (string line : lines) {
            // 解析请求行
            if (line.find("GET") == 0 || line.find("POST") == 0) {
                firstLine = line;
                istringstream iss(line);
                string httpMethod, httpRequestUri, httpVersion;
                iss >> httpMethod >> httpRequestUri >> httpVersion;
                this->method = httpMethod;
                this->uri = httpRequestUri;
                this->version = httpVersion;
            } else if (line.find("CONNECT") == 0) {
                firstLine = line;
                istringstream iss(line);
                string port = ":443";
                string httpMethod, httpRequestUri, httpVersion;
                iss >> method >> httpRequestUri >> port >> httpVersion;
                this->method = method;
                this->uri = httpRequestUri;
                this->version = httpVersion;
            } else if (line.find("Host: ") == 0) {
                // start after host: tag, get rid of port
                this->host = split(line.substr(6), ":")[0]; 
            } else if (line.find("Date: ") == 0) {
                this->date = line.substr(6);
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
    HttpRequest(string message) {
        vector<string> parts = split(message, "\r\n\r\n");
        if (parts.size() > 0) {
            header = parts[0];
        }
        // method like post can have body
        if (parts.size() > 1) {
            body = parts[1];
        } 
        parseHeader();
    }

    string getHeader() {
        return header;
    }

    string getMethod() {
        return method;
    }

    string getUri() {
        return uri;
    }

    string getHost() {
        return host;
    }

    string getBody() {
        return body;
    }

    string getFirstLine() {
        return firstLine;
    }

    string getDate() {
        return date;
    }
};
#endif
