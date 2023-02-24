#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H    
#include "sstream"
#include "string"
#include "vector"
using namespace std;

class HttpRequest {
private:
    string header;
    string method;
    string uri;
    string host;
    string body;
    string version;
    // todo: may add others like connection, useragent, and they can all parse from header

    void parseHeader() {
        vector<string> lines = split(header, "\r\n");
        for (string line : lines) {
            // 解析请求行
            if (line.find("GET") == 0 || line.find("POST") == 0) {
                istringstream iss(line);
                string httpMethod, httpRequestUri, httpVersion;
                iss >> method >> httpRequestUri >> httpVersion;
                this->method = method;
                this->uri = httpRequestUri;
                this->version = httpVersion;
            } else if (line.find("CONNECT") == 0) {
                istringstream iss(line);
                string port = ":443";
                string httpMethod, httpRequestUri, httpVersion;
                iss >> method >> httpRequestUri >> port >> httpVersion;
                this->method = method;
                this->uri = httpRequestUri;
                this->version = httpVersion;
            } else if (line.find("Host: ") == 0) {
                // start after host: tag
                this->host = line.substr(6); 
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
};
#endif
