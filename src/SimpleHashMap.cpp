#include "SimpleHashMap.hpp"

SimpleHashMap::SimpleHashMap() : buckets(16, nullptr), count(0) {}

SimpleHashMap::~SimpleHashMap() {
    clear();
}

unsigned long long SimpleHashMap::hashKey(const std::string &key) {
    unsigned long long h = 0;
    for (unsigned char c : key) {
        h = h * 131ULL + static_cast<unsigned long long>(c);
    }
    return h;
}

std::size_t SimpleHashMap::indexFor(const std::string &key) {
    if (buckets.empty()) return 0;
    return static_cast<std::size_t>(hashKey(key) % buckets.size());
}

void SimpleHashMap::put(const std::string &key, const std::string &value) {
    if (count + 1 > buckets.size() * 3 / 4) {
        rehash(buckets.size() * 2);
    }

    std::size_t idx = indexFor(key);
    Node *cur = buckets[idx];
    while (cur) {
        if (cur->key == key) {
            cur->value = value;
            return;
        }
        cur = cur->next;
    }

    Node *n = new Node{key, value, buckets[idx]};
    buckets[idx] = n;
    count++;
}

bool SimpleHashMap::get(const std::string &key, std::string &out) {
    std::size_t idx = indexFor(key);
    Node *cur = buckets[idx];
    while (cur) {
        if (cur->key == key) {
            out = cur->value;
            return true;
        }
        cur = cur->next;
    }
    return false;
}

bool SimpleHashMap::contains(const std::string &key) {
    std::size_t idx = indexFor(key);
    Node *cur = buckets[idx];
    while (cur) {
        if (cur->key == key) return true;
        cur = cur->next;
    }
    return false;
}

bool SimpleHashMap::erase(const std::string &key) {
    std::size_t idx = indexFor(key);
    Node *cur = buckets[idx];
    Node *prev = nullptr;
    while (cur) {
        if (cur->key == key) {
            if (prev) prev->next = cur->next; else buckets[idx] = cur->next;
            delete cur;
            count--;
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}

void SimpleHashMap::clear() {
    for (std::size_t i = 0; i < buckets.size(); i++) {
        Node *cur = buckets[i];
        while (cur) {
            Node *nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        buckets[i] = nullptr;
    }
    count = 0;
}

void SimpleHashMap::rehash(std::size_t newCapacity) {
    if (newCapacity < 16) newCapacity = 16;
    std::vector<Node*> newBuckets(newCapacity, nullptr);

    for (std::size_t i = 0; i < buckets.size(); i++) {
        Node *cur = buckets[i];
        while (cur) {
            Node *next = cur->next;
            unsigned long long h = hashKey(cur->key);
            std::size_t idx = static_cast<std::size_t>(h % newCapacity);
            cur->next = newBuckets[idx];
            newBuckets[idx] = cur;
            cur = next;
        }
        buckets[i] = nullptr;
    }

    buckets.swap(newBuckets);
}
