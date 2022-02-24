#ifndef _THREAD_INFO_HPP__
#define _THREAD_INFO_HPP__
#include <iostream>
#include "cache.hpp"
#include "log_manager.hpp"

using namespace std;

class ThreadInfo {
public:
    Cache * cache;
    string ip_addr;
    LogManager * log_manager;
    int thread_id;
    int browser_fd;

    ThreadInfo(Cache * _cache, string _ip_addr, LogManager * _log_manager, int _thread_id, int _browser_fd) : cache(_cache), ip_addr(_ip_addr), log_manager(_log_manager), thread_id(_thread_id), browser_fd(_browser_fd) {}
};

#endif