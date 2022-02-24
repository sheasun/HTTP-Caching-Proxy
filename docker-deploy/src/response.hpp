#include <vector>
#include <time.h>
#include <iostream>
#include "log_manager.hpp"

using namespace std;

class Response {
public:
    std::string code;
    std::string date;
    std::string response;
    std::string cache_control;

    Response() : response() {}
    Response(vector<char> vec);
    Response(string res) : response(res) {
        parseCode();
        parseCacheControl();
    }

    void parseDate();
    void parseCode();
    void parseCacheControl();
  
    bool isNoCache();
    bool isNoStore();
    bool isPrivate();
    bool isChunked();

    // check the current time is valid or not
    bool isUpdate(int thread_id, LogManager * log_manager);
    bool needRevalidate(int thread_id, LogManager * log_manager);

    // calculate the time
    double getDiffTime();

    string getDate() { return date; }
    string getCode() { return code; }
    string getResponse() { return response; }
    string getCacheControl() { return cache_control; }

    string getMaxAge();
    string getFirstLine();
    
    string getField(string key);
    string getEtag() { return getField("ETag: "); }
    string getExpires() { return getField("Expires: "); }
    string getLastModified() { return getField("Last-Modified: "); }

    time_t parseTime(string t);
    
    int getContentLength();
};