#include "response.hpp"

// find the input key in the response, and then return the corresponding value 
string Response::getField(string key) {
    string value = "";
    size_t pos = response.find(key);
    if(pos != string::npos) {
        string temp = response.substr(pos);
        size_t start = temp.find(" ") + 1;
        size_t end = temp.find("\r\n");
        value = temp.substr(start, end - start);
        return value;
    }
    return value;
}

Response::Response(vector<char> vec) {
    response.insert(response.begin(), vec.begin(), vec.end());
}

// date
void Response::parseDate() {
    date = getField("Date: ");
}

// code
void Response::parseCode() {
    size_t pos = response.find(" ");
    string temp = response.substr(pos + 1);
    pos = temp.find(" ");
    code = temp.substr(0, pos);
}

// cache-control
void Response::parseCacheControl() {
    size_t pos = response.find("Cache-Control: ");
    if (pos != string::npos) {
        string temp = response.substr(pos);
        size_t start = temp.find(" ") + 1;
        size_t end = temp.find("\r\n");
        cache_control = temp.substr(start, end - start);
    }
}

// check if the value is no-cache
bool Response::isNoCache() {
    size_t pos = cache_control.find_first_of("no-cache");
    return pos != string::npos;
}

// check if the value is no-store
bool Response::isNoStore() {
    size_t pos = cache_control.find_first_of("no-store");
    return pos != string::npos;
}

// check the value is private
bool Response::isPrivate() {
    size_t pos = cache_control.find_first_of("private");
    return pos != string::npos;
}

// check if it is chunked
bool Response::isChunked() {
    std::string str = getField("Transfer-Encoding: ");
    size_t pos = str.find_first_of("chunked");
    return pos != string::npos;
}

// check the current time is valid or not
bool Response::isUpdate(int thread_id, LogManager * log_manager) {
    bool check = false;
    std::string max_age = getMaxAge();
    std::string expires = getExpires();
    std::string last_modified = getLastModified();
    if (expires != "") {
        time_t expire_time = parseTime(expires);
        // 5 hours later than GMT
        time_t now = time(NULL) + 18000;
        if(now >= expire_time) {
            string msg = to_string(thread_id) + ": in cache, but expires at " + to_string(expire_time) + "\n";
            cout << msg;
            log_manager->writeLog(msg);
            return check;
        }
        check = true;
        return check;
    }
    if(max_age != "") {
        long max_age_long = stol(max_age);
        if(max_age_long <= getDiffTime()) {
            string msg = to_string(thread_id) + ": in cache, requires validation\n";
            cout << msg;
            log_manager->writeLog(msg);
            return check;
        }
        check = true;
        return check;
    }
    if (last_modified != "") {
        time_t date_time = parseTime(date);
        time_t last_modified_time = parseTime(last_modified);
        double new_time = difftime(date_time, last_modified_time) / 10.0;
        if(getDiffTime() >= new_time) {
            return check;
        }
        check = true;
        return check;
    }
    return check;
}

string Response::getMaxAge() {
    string cacheControl = getCacheControl();
    if (cacheControl != "") {
        size_t p1 = cacheControl.find("max-age");
        if (p1 != string::npos) {
            size_t p2 = cacheControl.find(" ", p1 + 1);
            if(p2 == string::npos)  {
                p2 = cacheControl.find("\r\n", p1 + 1);
            }
            return cacheControl.substr(p1 + 8, p2 - p1 - 8);
        }
    }
    return "";
}

time_t Response::parseTime(string t) {
    struct tm timeinfo;
    size_t pos = t.find_first_of("GMT");
    if(pos != string::npos){
        t = t.substr(0, pos - 1);
    }
    strptime(t.c_str(), "%a, %d %b %Y %H:%M:%S", &timeinfo);
    time_t prev = mktime(&timeinfo) - 18000;
    return prev;
}

double Response::getDiffTime() {
  time_t prev = parseTime(date);
  time_t curr = time(NULL);
  double diff = difftime(curr, prev);
  return diff;
}

string Response::getFirstLine() {
    size_t pos = response.find("\n") - 1;
    return response.substr(0, pos);
}

int Response::getContentLength() {
    string con_len = getField("Content-Length: ");
    int len = con_len.length();
    return len == 0 ? -1 : len;
}

bool Response::needRevalidate(int thread_id, LogManager * log_manager) {
    return isNoCache() ? true : !isUpdate(thread_id, log_manager);
}