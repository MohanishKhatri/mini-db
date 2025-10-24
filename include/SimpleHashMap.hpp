#pragma once
#include <string>
#include <vector>

class SimpleHashMap {
public:
    SimpleHashMap();
    ~SimpleHashMap();

    // no copy
    SimpleHashMap(const SimpleHashMap&) = delete;
    SimpleHashMap& operator=(const SimpleHashMap&) = delete;

    void put(const std::string &key, const std::string &value);
    bool get(const std::string &key, std::string &out);
    bool contains(const std::string &key);
    bool erase(const std::string &key);
    void clear();

private:
    struct Node {
        std::string key;
        std::string value;
        Node *next;
    };

    std::vector<Node*> buckets;
    std::size_t count;

    std::size_t indexFor(const std::string &key);
    unsigned long long hashKey(const std::string &key);
    void rehash(std::size_t newCapacity);
};
