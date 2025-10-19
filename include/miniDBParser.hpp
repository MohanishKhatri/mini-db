#pragma once
#include <string>
#include <vector>

// bulk-string parser
class MiniDBParser {
public:
    std::vector<std::string> parse(const std::string& input);
};
