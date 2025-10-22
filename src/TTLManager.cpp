#include "TTLManager.hpp"



void TTLManager:: setTTL(const std::string &key, int ttl_seconds){
    auto now = std::chrono::steady_clock::now();
    auto expiry=now+std::chrono::seconds(ttl_seconds);
    ttlMap[key]=expiry;
    ttlQueue.push(TTLItem{key,expiry});
}

// Remove TTL for a key (e.g., if key deleted manually)
void TTLManager:: removeTTL(const std::string &key){
    ttlMap.erase(key);
}


// Cleanup expired keys and return list of keys to delete
std::vector<std::string> TTLManager:: cleanupExpired(){
    std::vector<std::string> expired;

    auto currtime = std::chrono::steady_clock::now();

    while (!ttlQueue.empty() && ttlQueue.top().expiry <= currtime) {
        TTLItem item = ttlQueue.top();
        ttlQueue.pop();
        auto it = ttlMap.find(item.key);
        // Only remove if the stored expiry is indeed due
        if (it != ttlMap.end() && it->second <= currtime) {
            ttlMap.erase(it);
            expired.push_back(item.key);
        }
    }

    return expired;
}



// Optional: get remaining TTL for a key
int TTLManager:: getTTL(const std::string &key){
    auto it = ttlMap.find(key);
    if (it == ttlMap.end()) {
        return -1; // no TTL set
    }
    auto now = std::chrono::steady_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(it->second - now);
    return static_cast<int>(diff.count());
}

