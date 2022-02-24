#ifndef _CACHE_HPP__
#define _CACHE_HPP__
#include <map>
#include <list>
#include <mutex>
#include <iostream>
#include <exception>

using namespace std;

class Cache {
private:
    size_t capacity;
    mutex lock;
    list<pair<string, string> > cache;
    /* Supprt O(1) time lookup. */
    map<string, list<pair<string, string> >::iterator> cacheMap;
public:
    /* Set default value for capacity. */
    Cache() : capacity(10) {};
    Cache(size_t _capacity) : capacity(_capacity) {};
    /* Given key, return corresponding value. */
    string get(string key);
    /* Store a new key-value pair. */
    void put(string key, string value);
};
#endif