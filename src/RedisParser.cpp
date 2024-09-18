#include "RedisParser.hpp"

std::vector<std::string> RedisParser::parseCommand(const std::string& command) {
    std::vector<std::string> result;
    size_t i = 0;
    int arrLen = 0;

    if (command[0] == '*') {
        i = 1;
        while (command[i] != '\r' && command[i+1] != '\n') {
            arrLen = arrLen * 10 + (command[i] - '0');
            i++;
        }
        i += 2;  // Skip \r\n

        while (arrLen > 0) {
            if (command[i] == '$') {
                i++;  // Skip $
                int len = 0;
                while (command[i] != '\r') {
                    len = len * 10 + (command[i] - '0');
                    i++;
                }
                i += 2;  // Skip \r\n
                result.push_back(command.substr(i, len));
                i += len + 2;  // Skip content and \r\n
                arrLen--;
            }
        }
    }

    return result;
}