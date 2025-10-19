#pragma once
#include <string>
#include <unordered_map>
#include <vector>

class MiniDB {
public:
    std::string executeCommand(const std::string& input);

private:
    std::unordered_map<std::string, std::string> store;

    std::string processPing();
    std::string processSetCommand(const std::vector<std::string>& cleaninput);
    std::string processGetCommand(const std::vector<std::string>& cleaninput);
    std::string processDeleteCommand(const std::vector<std::string>& cleaninput);
};
