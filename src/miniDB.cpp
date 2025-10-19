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

    return "-ERR unknown command\r\n";
}

std::string MiniDB::processPing() {
    return "+PONG\r\n";
}

std::string MiniDB::processSetCommand(const std::vector<std::string>& cleaninput) {
    if (cleaninput.size() < 3) return "-ERR wrong number of arguments for 'SET'\r\n";
    store[cleaninput[1]] = cleaninput[2];
    return "+OK\r\n";
}

std::string MiniDB::processGetCommand(const std::vector<std::string>& cleaninput) {
    if (cleaninput.size() < 2) return "-ERR wrong number of arguments for 'GET'\r\n";

    auto it = store.find(cleaninput[1]);
    if (it == store.end()) return "$-1\r\n"; 

    return "$" + std::to_string(it->second.size()) + "\r\n" + it->second + "\r\n";
}

std::string MiniDB::processDeleteCommand(const std::vector<std::string>& cleaninput) {
    if (cleaninput.size() < 2) return "-ERR wrong number of arguments for 'DEL'\r\n";

    int removed = store.erase(cleaninput[1]);
    return ":" + std::to_string(removed) + "\r\n";
}


