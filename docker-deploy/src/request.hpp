#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <string.h>
#include <vector>

using namespace std;

class Request {
private:
    string uri;
    string line;
    string host;
    string port;
    string method;
    string request;
    void parseRequest();
public:
    Request() : request() {}
    Request(vector<char> v);
    Request(string req) : request(req) { parseRequest(); }

    string getURI() { return uri; }
    string getLine() { return line; }
    string getHost() { return host; }
    string getPort() { return port; }
    string getRequest() { return request; }
    string getMethod() { return method; }
    string getHeader() {
        size_t end = request.find("\r\n\r\n");
        return request.substr(0, end);
    }

    int getContentLen();
    
    bool check() { return method == "GET" || method == "POST" || method == "CONNECT"; }
};