#include "Logger.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>

std::ofstream Logger::logFile;
std::mutex Logger::logMutex;

void Logger::init(const std::string& filename) {
    logFile.open(filename, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

void Logger::log(const std::string& message) {
    write("INFO", message);
}

void Logger::error(const std::string& message) {
    write("ERROR", message);
}

void Logger::connection(const std::string& address, int port, const std::string& message) {
    write("CONNECTION", address + ":" + std::to_string(port) + " - " + message);
}

void Logger::write(const std::string& level, const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto now_tm = *std::localtime(&now_c);
    
    logFile << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << " [" << level << "] " << message << std::endl;
    std::cout << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << " [" << level << "] " << message << std::endl;
}
