#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H    
#include "sstream"
#include "string"
#include "vector"
#include "iostream"
#include "chrono"
#include "ctime"
#include "iomanip"
using namespace std;

class HttpResponse {
private:
    string header;
    string firstLine;
    string version;
    string statusCode;
    string statusText;
    string body;
    string date;
    string cacheControl;
    string maxAge;
    string expire;
    time_t maxEndTime;
    time_t expiredTime;
    string etag;
    string lastModified;
    bool isChunked;
    bool revalidate;
    int contentLength;
    // may add other element in headers

    void parseHeader() {
        vector<string> lines = split(header, "\r\n");
        for (string line : lines) {
            if (line.find("HTTP") == 0) {
                firstLine = line;
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
                for (string seg: split(line, ",")) {
                    if (seg.find("max-age=") != string::npos) {
                        seg = seg.substr(seg.find("max-age="));
                        maxAge = seg.substr(8);
                    } else if (seg.find("must-revalidate") != string::npos) {
                        revalidate = true; 
                    }
                }
            } else if (line.find("Expire: ") == 0) {
                this->expire = line.substr(8);
            } else if (line.find("Etag: ") == 0) {
                this->etag = line.substr(6);
            } else if (line.find("Last-Modified: ") == 0) {
                this->lastModified = line.substr(15);
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
        this->contentLength = -1;
        this->date = "";
        this->cacheControl = "";
        this->expire = "";
        this->maxAge = "";
        this->etag = "";
        this->lastModified = "";
        this->isChunked = false;
        this->revalidate = false;
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
    
    string getMaxAge() {
        return maxAge;
    }

    string getExpire() {
        return expire;
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

    string getFirstLine() {
        return firstLine;
    }

    time_t getMaxEndTime() {
        time_t respTime = getConvertedDate(date);
        auto expireTime = chrono::system_clock::from_time_t(respTime) + chrono::seconds(stoi(maxAge));
        return chrono::system_clock::to_time_t(expireTime);
    }

    time_t getExpiredTime() {
        return getConvertedDate(expire);
    }

    time_t getConvertedDate(string dateStr) {
        istringstream iss(dateStr);
        tm tm = {};
        iss >> get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
        chrono::system_clock::time_point tp = chrono::system_clock::from_time_t(mktime(&tm));
        return chrono::system_clock::to_time_t(tp);
    }

    bool isRevalidate() {
        return revalidate;
    }

    string getEtag() {
        return etag;
    }

    string getLastModified() {
        return lastModified;
    }

};
#endif
