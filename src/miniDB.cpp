#include "miniDB.hpp"
#include "miniDBParser.hpp"
#include <algorithm>
#include <string>

std::string MiniDB::executeCommand(const std::string& input) {
    MiniDBParser parser;
    std::vector<std::string> cleaninput = parser.parse(input);

    if (cleaninput.empty()) return "-ERR malformed command\r\n";

    std::string command = cleaninput[0];
    for(int i=0;i<command.size();i++){
        command[i]=toupper(command[i]);
    }

    if (command == "PING") return processPing();
    if (command == "SET")  return processSetCommand(cleaninput);
    if (command == "GET")  return processGetCommand(cleaninput);
    if (command == "DEL")  return processDeleteCommand(cleaninput);
    if (command=="EXISTS") return processExistsCommand(cleaninput);
    if (command=="EXPIRE") return processExpireCommand(cleaninput);
    if (command=="TTL") return processTTLCommand(cleaninput);

    return "-ERR unknown command\r\n";
}

std::string MiniDB::processPing() {
    return "+PONG\r\n";
}

std::string MiniDB::processSetCommand(const std::vector<std::string>& cleaninput) {
    if (cleaninput.size() < 3) return "-ERR wrong number of arguments for 'SET'\r\n";
    store.put(cleaninput[1], cleaninput[2]);
    // No need to access cleaninput[3] when size is 3
    if(cleaninput.size()==5){
        std::string fourthArg = cleaninput[3];
        for(auto &ch : fourthArg){
            ch = toupper(ch);
        }
        if(fourthArg != "EX"){
            return "-ERR syntax error\r\n";
        }
        int ttlTime = 0;
        try {
            ttlTime = std::stoi(cleaninput[4]);
        } catch (const std::exception&) {
            return "-ERR value is not an integer or out of range\r\n";
        }
        if(ttlTime>0){
            ttlmanager.setTTL(cleaninput[1],ttlTime);
        }
        else{
            return "-ERR invalid expire time\r\n";
        }
    }
    else if(cleaninput.size() == 4) {
        std::string thirdArg = cleaninput[3];
        for(auto &ch : thirdArg) {
            ch = toupper(ch);
        }
        if(thirdArg != "EX") {
            return "-ERR syntax error\r\n";
        }
    }
    else if(cleaninput.size() > 5) {
        return "-ERR wrong number of arguments for 'SET'\r\n";
    }
    return "+OK\r\n";
}

std::string MiniDB::processGetCommand(const std::vector<std::string>& cleaninput) {
    if (cleaninput.size() < 2) return "-ERR wrong number of arguments for 'GET'\r\n";

    const std::string& key = cleaninput[1];
    
    // Check if expired
    int ttl = ttlmanager.getTTL(key);
    if (ttl != -1 && ttl <= 0) { // expired
        store.erase(key);
        ttlmanager.removeTTL(key);
        return "$-1\r\n";
    }
    std::string value;
    if (!store.get(key, value)) return "$-1\r\n";
    return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
}

std::string MiniDB::processDeleteCommand(const std::vector<std::string>& cleaninput) {
    if (cleaninput.size() < 2) return "-ERR wrong number of arguments for 'DEL'\r\n";

    const std::string& key = cleaninput[1];
    bool removed = store.erase(key);
    if(removed) {
        ttlmanager.removeTTL(key);
    }
    return ":" + std::to_string(removed ? 1 : 0) + "\r\n";
}

std::string MiniDB::processExistsCommand(const std::vector<std::string>& cleaninput){
    if(cleaninput.size()<2) return "-ERR wrong number of arguments for 'EXISTS'\r\n";

    int exists = store.contains(cleaninput[1]) ? 1 : 0;
    return ":" + std::to_string(exists) + "\r\n";
}

std::string MiniDB::processExpireCommand(const std::vector<std::string>& cleaninput){
    if(cleaninput.size()<3) return "-ERR wrong number of arguments for 'EXPIRE'\r\n";

    std::string key=cleaninput[1];
    
    int ttl_seconds;
    try {
        ttl_seconds = std::stoi(cleaninput[2]);
    } catch (const std::exception&) {
        return "-ERR value is not an integer or out of range\r\n";
    }
    if(ttl_seconds <= 0) {
        return "-ERR invalid expire time\r\n";
    }

    // Lazy expire: if key has TTL and it's due, treat as not existing
    int ttl = ttlmanager.getTTL(key);
    if (ttl != -1 && ttl <= 0) {
        store.erase(key);
        ttlmanager.removeTTL(key);
        return ":0\r\n";
    }

    // Only set TTL if key exists
    if (!store.contains(key)) {
        return ":0\r\n";
    }

    ttlmanager.setTTL(key, ttl_seconds);
    return ":1\r\n"; // Indicate success
}

TTLManager& MiniDB:: getTTLManager(){
    return ttlmanager;
}

std::string MiniDB::processTTLCommand(const std::vector<std::string>& cleaninput){
    if(cleaninput.size()<2) return "-ERR wrong number of arguments for 'TTL'\r\n";
    
    const std::string& key = cleaninput[1];
    
    // Check if key exists
    if(!store.contains(key)) {
        return ":-2\r\n"; // Key doesn't exist
    }
    
    int ttl = ttlmanager.getTTL(key);
    if(ttl == -1) {
        return ":-1\r\n"; 
    }
    if(ttl <= 0) {
        store.erase(key);
        ttlmanager.removeTTL(key);
        return ":-2\r\n"; // expired
    }
    
    return ":" + std::to_string(ttl) + "\r\n";
}

void MiniDB:: purgeExpiredKeys() {
        auto expired = ttlmanager.cleanupExpired();
        for (const auto& key : expired) {
            store.erase(key);
        }
    }


