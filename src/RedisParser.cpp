#include "RedisParser.hpp"
#include <sstream>
#include <stdexcept>

RedisParser::RedisParser(const std::string& input)
    : input(input) {}

std::vector<std::string> RedisParser::parseCommand()
{
    std::vector<std::string> result;
    std::istringstream iss(input);
    std::string line;

    // Handle RESP (REdis Serialization Protocol)
    while (std::getline(iss, line))
    {
        if (line.empty() || line[0] == '\r' || line[0] == '\n')
        {
            continue;
        }

        if (line[0] == '*') // Array of Bulk Strings
        {
            continue; // We just care about the actual command parts
        }
        else if (line[0] == '$') // Bulk String
        {
            std::string argument;
            std::getline(iss, argument);
            argument.erase(argument.find_last_not_of("\r\n") + 1); // Remove trailing CRLF
            result.push_back(argument);
        }
        else // Simple Strings (e.g., inline commands)
        {
            result.push_back(line);
        }
    }

    return result;
}
