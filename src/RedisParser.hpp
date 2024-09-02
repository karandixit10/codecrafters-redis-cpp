#ifndef REDISPARSER_HPP
#define REDISPARSER_HPP

#include <string>
#include <vector>

class RedisParser
{
public:
    RedisParser(const std::string& input);
    std::vector<std::string> parseCommand();

private:
    std::string input;
};

#endif // REDISPARSER_HPP
