#pragma once
#ifndef LOGGER
#define LOGGER

#include <string>
#include <fstream>
#include <mutex>

class Logger {
public:
    static void init(const std::string& filename);
    static void log(const std::string& message);
    static void error(const std::string& message);
    static void connection(const std::string& address, int port, const std::string& message);

private:
    static void write(const std::string& level, const std::string& message);

    static std::ofstream logFile;
    static std::mutex logMutex;
};

#endif