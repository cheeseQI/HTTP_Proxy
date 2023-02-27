#include <map>
#include <algorithm>
#include <mutex>

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "sstream"
#include "string"
#include "vector"
using namespace std;

class Cache{
private:
    class Node{
    public:
        string key; // uri
        string value; // 存header + body
        Node* prev;
        Node* next;
        Node(): key(), value(), prev(nullptr), next(nullptr){}
        Node(string k,string v): key(k), value(v), prev(nullptr), next(nullptr){}
    };
    map<string,Node*> my_cache = {}; // uri, Node
    Node* head = nullptr;
    Node* tail = nullptr;
    int capacity = 0;
    int size = 0;

    // Singleton
    // private constructor to prevent instantiation
    Cache(){}
    Cache(int c): my_cache(), capacity(c), size(0){
        head = new Node();
        tail = new Node();
        head->next=tail;
        tail->prev=head;
    }
    // the only instance of Singleton
    static Cache* instance;



public:
    static Cache* getInstance() {
        if (instance == nullptr){
            instance = new Cache();
        }
        return instance;
    }

    // Disable copying and operator
    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;
    ~Cache();



    string get(string key);
    void put(string key, string value);
    void addToHead(Node *node);
    void removeNode(Node *node);
    void moveToHead(Node *node);
    void removeTail();


    bool storeResponse(string uri, HttpResponse rsp, int id);
    //string revalidate(HttpRequest request, HttpResponse response, int socket,int id);
    //string checkIfNoneMatch(HttpRequest request, HttpResponse response, int socket, string etag, int id);
    //string checkIfModifiedSince(HttpRequest request, HttpResponse response, int socket, string lastModify, int id);
    //string reSendRequest(HttpRequest request, int socket, int id);
    //string checkValidate(HttpRequest request, HttpResponse response, int socket, string type, string content, int id);
};

// // initialize static field
// map<string, Cache::Node*> Cache::my_cache = {};
// Cache::Node* Cache::head = nullptr;
// Cache::Node* Cache::tail = nullptr;
// int Cache::capacity = 0;
// int Cache::size = 0;
// Cache* Cache::instance = nullptr;
