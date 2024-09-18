#ifndef REDIS_PARSER_HPP
#define REDIS_PARSER_HPP

#include <vector>
#include <string>

class RedisParser {
public:
    std::vector<std::string> parseCommand(const std::string& command);
};

#endif // REDIS_PARSER_HPP