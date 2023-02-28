#include "Cache.h"

std::mutex copy_mutex;
std::mutex cache_mutex;
// ？ max age在哪里实现的

Cache* Cache::instance = nullptr;
string Cache::get(string key){
    std::lock_guard<std::mutex> lck(cache_mutex);
    //if key does not exist, return NULL
    if (key == "wildcard") {
        string message = "Hello, cache world!\n";
                        string response = "HTTP/1.1 200 OK\r\n"
                                        "Content-Type: text/plain\r\n"
                                        "Content-Length: " + to_string(message.length()) + "\r\n"
                                        "\r\n"
                                        + message;

        return response;
    }
    if (!Cache::instance->my_cache.count(key)) {
        return "";
    }
    //if it exists, get node then move it to head
    Node* node = Cache::instance->my_cache[key];
    moveToHead(node);
    return node->value;
}

//put key and value pair into cache
void Cache::put(string key, string value) {
    std::lock_guard<std::mutex> lck(cache_mutex);
    if (!Cache::instance->my_cache.count(key)) {
        // if key does not exist, create a new node
        Node* node = new Node(key, value);
        // add into hashmap
        Cache::instance->my_cache[key] = node;
        // add to head
        addToHead(node);
        ++Cache::instance->size;
        if (Cache::instance->size > Cache::instance->capacity) {
            // if over capacity, remove tail node
            removeTail();
        }
    }
    else {
        // if key exist, get node and edit value then move it to head
        Node* node = Cache::instance->my_cache[key];
        node->value = value;
        moveToHead(node);
    }
}

//add node to head
void Cache::addToHead(Node *node){
    node->prev = Cache::instance->head;
    node->next = Cache::instance->head->next;
    Cache::instance->head->next->prev = node;
    Cache::instance->head->next = node;
}

//remove the given node
void Cache::removeNode(Node *node){
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

//move the node to head
void Cache::moveToHead(Node *node){
    removeNode(node);
    addToHead(node);
}

//remove tail node and delete it
void Cache::removeTail(){
    Node* node = Cache::instance->tail->prev;
    removeNode(node);
    Cache::instance->my_cache.erase(node->key);
    delete node;
    --Cache::instance->size;
}

//destructor
Cache::~Cache(){
    while(Cache::instance->head!=NULL){
        Node* temp = Cache::instance->head->next;
        delete Cache::instance->head;
        Cache::instance->head = temp;
    }
}



string Cache::storeResponse(string uri, string rspStr) {
    HttpResponse rsp(rspStr);
    string code = rsp.getStatusCode();
    string cacheControl = rsp.getCacheControl();
    if (code == "200" && rsp.canCache()) {
        put(uri, rspStr);
        string expire = rsp.getExpire();
        string message;
        if(expire != "") {
            //message = generateLogMsg(id,"cached, expires at " + expire);
        }
        else {
            //message = generateLogMsg(id, "cached, but requires re-validation");
        }
        return "cached";
    } else if (code != "200") {
        return "response is not 200/OK";
    } else if (cacheControl == "") {
        return "does not support cache control";
    } else if (cacheControl.find("no-cache") != string::npos) {
        return "cache-control: no-cache";
    } else if (cacheControl.find("no-store") != string::npos) {
            return "cache-control: no-store";
    } else if (cacheControl.find("private") != string::npos) {
        return "cache-control: private";
    } 
    return "cache-policy forbidden";
}




// string Cache::revalidate(HttpRequest request, HttpResponse response, int socket, int id){
//     string etag = response.getEtag();
//     string lastModify = response.getLastModify();
//     //if has e-tag, send if-none-match
//     if(etag != ""){
//         cout << "use etag" << endl;
//         return checkIfNoneMatch(request, response, socket, etag, id);
//     }
//     //if has last-modify, send if-modified-since
//     else if(lastModify != ""){
//         cout << "use last modify" << endl;
//         return checkIfModifiedSince(request, response, socket, lastModify, id);
//     }
//     //send all request
//     else{
//         return reSendRequest(request, socket, id);
//     }
// }


// string Cache::checkIfNoneMatch(HttpRequest request, HttpResponse response, int socket, string etag, int id){
//     return checkValidate(request, response, socket,"If-None-Match: ", etag, id);
// }


// string Cache::checkIfModifiedSince(HttpRequest request, HttpResponse response, int socket, string lastModify, int id){
//     return checkValidate(request, response, socket, "If-Modified-Since: ", lastModify, id);
// }


// // 删掉 自己在外面写
// string Cache::reSendRequest(HttpRequest request, int socket, int id){
//     cout << "resend" << endl;
//     string origin = request.getHeader();

//     send(socket, origin.data(), origin.size() + 1, 0);

//     vector<char> v;
//     my_recvFrom(socket,v);
//     HttpResponse newResponse(v);
//     storeResponse(request.getUri(),newResponse, id);
//     return newResponse.getHeader();
// }



// string Cache::checkValidate(HttpRequest request, HttpResponse response, int socket, string type, string content, int id){
//     //send new request
//     cout << "re validate" << endl;
//     string origin = request.getHeader();
//     string newRequest = origin + "\r\n" + type + content + "\r\n\r\n";

//     send(socket, newRequest.data(), newRequest.size() + 1, 0);

//     //receive new response
//     vector<char> v;
//     my_recvFrom(socket, v);
//     HttpResponse newResponse(v);
//     if(newResponse.getCode() == "304"){
//         return response.getHeader();
//     }
//     else{
//         storeResponse(request.getUri(),newResponse, id);
//         return newResponse.getHeader();
//     }
// }


// //删掉外面写
// void my_recvFrom(int fd, vector<char> &v){
//     ssize_t index = 0;
//     v.resize(65536);
//     ssize_t msg_len = recv(fd,&v.data()[index],v.size(),0);
//     checkMsgLen(msg_len);
//     index += msg_len;
//     HttpResponse r = HttpResponse(v);
//     if(r.getStatusCode() == "304"){
//         return;
//     }

//     if(r.getIsChuncked()){
//         cout << "is chunked" << endl;
//         string res;
//         res.insert(res.begin(),v.begin(),v.end());
//         while(res.find("0\r\n\r\n") == string::npos){
//             v.resize(index + 65536);
//             msg_len = recv(fd, &v.data()[index], 65536, 0);

//             if(msg_len <= 0){
//                 break;
//             }
//             res = "";
//             res.insert(res.begin(),v.begin() + index,v.begin() + index + msg_len);
//             index += msg_len;
//         }
//         return;
//     }

//     else{
//         int len = r.getContentLength();
//         cout << "not chunked, content-length is "<< len << endl;
//         while(index < len){
//             v.resize(index + 65536);
//             msg_len = recv(fd, &v.data()[index], v.size(), 0);

//             if(msg_len <= 0){
//                 break;
//             }
//             index += msg_len;
//         }
//     }
// }

// void checkMsgLen(int msg_len){
//     if(msg_len == 0){
//         throw MyException("Received nothing", "");
//     }
//     if(msg_len==-1){
//         throw MyException("Error while receiving", "");
//     }
// }