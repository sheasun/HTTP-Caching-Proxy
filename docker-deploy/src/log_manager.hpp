#ifndef _LOG_MANAGER_HPP__
#define _LOG_MANAGER_HPP__
#include <thread>
#include <mutex>
#include <sstream>
#include <vector>
#include <fstream>
#include <iostream>

using namespace std;

class LogManager {
public:
    mutex mtx;
    string path;
    ofstream file;
    
    LogManager(string _path) : path(_path) {
        file.open(path, std::ostream::out);
        if (!file.is_open()) {
	    cout << "(no-id): WARNING cannot open file\n";
	}
    }

    ~LogManager() {
        file.close();
    }

    void writeLog(string log) {
        lock_guard<mutex> lck(mtx);
        file << log;
        file.flush();
    }
};
#endif
