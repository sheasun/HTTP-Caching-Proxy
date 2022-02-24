#include "cache.hpp"

string Cache::get(string key) {
    lock_guard<mutex> guard(lock);
    if (!cacheMap.count(key)) {
        return "";
    }
    else {
        list<pair<string, string> >::iterator it = cacheMap[key];
        /* Move current entry to the front of the list. */
        cache.splice(cache.begin(), cache, it);
        return it->second;
    }
}

void Cache::put(string key, string value) {
    lock_guard<mutex> guard(lock);
    if (cacheMap.count(key)) {
        cache.erase(cacheMap[key]);
        cacheMap.erase(key);
    }
    cache.push_front(make_pair(key, value));
    cacheMap[key] = cache.begin();
    /* Remove the least recently used entry. */
    if (cache.size() > capacity) {
        list<pair<string, string> >::iterator it = cache.end();
        it ++;
        cacheMap.erase(it->first);
        cache.pop_back();
    }
}