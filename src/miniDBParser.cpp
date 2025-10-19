#include "miniDBParser.hpp"
#include <string>
#include <vector>
#include <cstdlib>

// example input:              *3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n

// parses array of bulk strings only

std::vector<std::string> MiniDBParser::parse(const std::string &input)
{
    std::vector<std::string> result;
    if (input.empty())
        return result;

    size_t index = 0;
    if (input[index] != '*')
        return result;
    index++;

    // read number of elements
    size_t crlfpos = input.find("\r\n", index); // position of '\r\n' after '*'

    if (crlfpos == std::string::npos)
        return result;

    std::string elementscount = input.substr(index, crlfpos - index);
    int elements = std::stoi(elementscount);
    index = crlfpos + 2;

    for (int i = 0; i < elements; i++)
    {
        if (index > input.size() || input[index] != '$')
            return result;
        index++;
        size_t endline = input.find("\r\n", index);
        if (endline == std::string::npos)
            return result;

        //finding length of bulk string
        std::string lenstring = input.substr(index, endline - index);
        int length = stoi(lenstring);
        index = endline + 2;

        if (index + length > input.size())
            return result;
        std::string str = input.substr(index, length);
        result.push_back(str);

        index = index + length + 2;
    }
    return result;
}
