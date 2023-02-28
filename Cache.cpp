#include "Cache.h"

std::mutex copy_mutex;
std::mutex cache_mutex;

Cache* Cache::instance = nullptr;
string Cache::get(string key){
    std::lock_guard<std::mutex> lck(cache_mutex);
    //if key does not exist, return NULL
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