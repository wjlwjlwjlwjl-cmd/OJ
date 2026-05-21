#include "logger.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

LogLevel Logger::s_level = DEBUG;
std::mutex Logger::s_mutex;

void Logger::init(LogLevel level) {
    s_level = level;
}

void Logger::setLevel(LogLevel level) {
    s_level = level;
}

void Logger::debug(const std::string& msg) {
    log(DEBUG, msg);
}

void Logger::info(const std::string& msg) {
    log(INFO, msg);
}

void Logger::warn(const std::string& msg) {
    log(WARN, msg);
}

void Logger::error(const std::string& msg) {
    log(ERROR, msg);
}

void Logger::log(LogLevel level, const std::string& msg) {
    if (level < s_level) return;

    std::lock_guard<std::mutex> lock(s_mutex);

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << "[" << levelStr(level) << "] "
        << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
        << " " << msg;

    auto& out = (level >= ERROR) ? std::cerr : std::cout;
    out << oss.str() << std::endl;
}

const char* Logger::levelStr(LogLevel level) {
    switch (level) {
        case DEBUG: return "DEBUG";
        case INFO:  return "INFO";
        case WARN:  return "WARN";
        case ERROR: return "ERROR";
        default:    return "UNKNOWN";
    }
}
