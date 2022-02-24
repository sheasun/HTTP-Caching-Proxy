#include "request.hpp"

Request::Request(vector<char> v) {
    request.insert(request.begin(), v.begin(), v.end());
    parseRequest();
}

void Request::parseRequest() {
    string s = "HTTP/1.1";
    size_t line_end = request.find(s) + s.size();
    line = request.substr(0, line_end);

    size_t method_end = request.find(" ");
    method = request.substr(0, method_end);
    port = method == "CONNECT" ? "443" : "80";

    string temp = request.substr(method_end + 1);
    size_t uri_end = temp.find(" ");
    uri = temp.substr(0, uri_end);
    size_t end = uri.find("//");
    end = end == string::npos ? 0 : end + 2;

    size_t end2 = uri.find(":", end);
    size_t end3 = uri.find("/", end);
    end3 = end3 == string::npos ? uri.size() + 1 : end3;
    if (end2 != string::npos) {
        host = uri.substr(end, end2 - end);
        port = uri.substr(end2 + 1, end3 - end2 - 1);
    }
    else {
        host = uri.substr(end, end3 - end);
    }
}

int Request::getContentLen() {
    size_t start = request.find("Content-Length: ");
    if(start != string::npos) {
        start += 16;
        string content = request.substr(start);
        size_t end = content.find_first_of("\r\n");
        content = content.substr(0, end);
        size_t content_len = atoi(content.c_str());
        return content_len;
    }
    return -1;
}
