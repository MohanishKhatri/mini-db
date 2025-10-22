#pragma once
#include <string>
#include <unordered_map>
#include <queue>
#include <chrono>

struct TTLItem {
    std::string key;
    std::chrono::steady_clock::time_point expiry;
};

struct TTLcomparator {
    // return true if a should come after b in a min-heap sense
    bool operator()(const TTLItem& a, const TTLItem& b) const {
        return a.expiry > b.expiry; // later expiry comes after earlier expiry
    }
};

class TTLManager {
public:
    // Set TTL for a key (overwrite if exists)
    void setTTL(const std::string &key, int ttl_seconds);

    // Remove TTL for a key (e.g., if key deleted manually)
    void removeTTL(const std::string &key);

    // Cleanup expired keys and return list of keys to delete
    std::vector<std::string> cleanupExpired();

    // Optional: get remaining TTL for a key
    int getTTL(const std::string &key);

private:
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> ttlMap;
    std::priority_queue<TTLItem, std::vector<TTLItem>, TTLcomparator> ttlQueue;

};
