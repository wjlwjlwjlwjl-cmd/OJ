#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <mutex>

enum LogLevel { DEBUG, INFO, WARN, ERROR };

class Logger {
public:
    static void init(LogLevel level = DEBUG);
    static void setLevel(LogLevel level);

    static void debug(const std::string& msg);
    static void info(const std::string& msg);
    static void warn(const std::string& msg);
    static void error(const std::string& msg);

private:
    static LogLevel s_level;
    static std::mutex s_mutex;

    static void log(LogLevel level, const std::string& msg);
    static const char* levelStr(LogLevel level);
};

#endif
