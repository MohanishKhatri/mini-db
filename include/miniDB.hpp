#pragma once
#include <string>
#include <vector>
#include "TTLManager.hpp"
#include "SimpleHashMap.hpp"

class MiniDB {
public:
    TTLManager& getTTLManager();
    std::string executeCommand(const std::string& input);
    void purgeExpiredKeys();
    
    private:
    SimpleHashMap store;
    TTLManager ttlmanager;

    std::string processPing();
    std::string processSetCommand(const std::vector<std::string>& cleaninput);
    std::string processGetCommand(const std::vector<std::string>& cleaninput);
    std::string processExistsCommand(const std::vector<std::string>& cleaninput);
    std::string processExpireCommand(const std::vector<std::string>& cleaninput);//set expire time
    std::string processTTLCommand(const std::vector<std::string>& cleaninput);//get remaining TTL
    std::string processDeleteCommand(const std::vector<std::string>& cleaninput);
};
