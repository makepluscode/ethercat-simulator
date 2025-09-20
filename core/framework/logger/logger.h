#pragma once

#include <string>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>

namespace ethercat_sim::framework::logger
{

enum class LogLevel
{
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3
};

class Logger
{
public:
    // Configuration methods
    static void setLevel(LogLevel level);
    static void setComponent(const std::string& component);
    static void setOutput(std::ostream& stream);
    static void setTimestampEnabled(bool enabled);

    // Logging methods
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);

    // Formatted logging (printf-style)
    template<typename... Args>
    static void debug(const char* format, Args... args)
    {
        if (current_level_ <= LogLevel::DEBUG)
        {
            char buffer[1024];
            std::snprintf(buffer, sizeof(buffer), format, args...);
            log(LogLevel::DEBUG, std::string(buffer));
        }
    }
    
    template<typename... Args>
    static void info(const char* format, Args... args)
    {
        if (current_level_ <= LogLevel::INFO)
        {
            char buffer[1024];
            std::snprintf(buffer, sizeof(buffer), format, args...);
            log(LogLevel::INFO, std::string(buffer));
        }
    }
    
    template<typename... Args>
    static void warn(const char* format, Args... args)
    {
        if (current_level_ <= LogLevel::WARN)
        {
            char buffer[1024];
            std::snprintf(buffer, sizeof(buffer), format, args...);
            log(LogLevel::WARN, std::string(buffer));
        }
    }
    
    template<typename... Args>
    static void error(const char* format, Args... args)
    {
        if (current_level_ <= LogLevel::ERROR)
        {
            char buffer[1024];
            std::snprintf(buffer, sizeof(buffer), format, args...);
            log(LogLevel::ERROR, std::string(buffer));
        }
    }

private:
    static void log(LogLevel level, const std::string& message);
    static std::string formatTimestamp();
    static std::string levelToString(LogLevel level);

    static LogLevel current_level_;
    static std::string current_component_;
    static std::ostream* output_stream_;
    static bool timestamp_enabled_;
    static std::mutex log_mutex_;
};

// Convenience macros for component-specific logging
#define LOG_DEBUG(msg) ethercat_sim::framework::logger::Logger::debug(msg)
#define LOG_INFO(msg) ethercat_sim::framework::logger::Logger::info(msg)
#define LOG_WARN(msg) ethercat_sim::framework::logger::Logger::warn(msg)
#define LOG_ERROR(msg) ethercat_sim::framework::logger::Logger::error(msg)

#define LOG_DEBUG_FMT(fmt, ...) ethercat_sim::framework::logger::Logger::debug(fmt, __VA_ARGS__)
#define LOG_INFO_FMT(fmt, ...) ethercat_sim::framework::logger::Logger::info(fmt, __VA_ARGS__)
#define LOG_WARN_FMT(fmt, ...) ethercat_sim::framework::logger::Logger::warn(fmt, __VA_ARGS__)
#define LOG_ERROR_FMT(fmt, ...) ethercat_sim::framework::logger::Logger::error(fmt, __VA_ARGS__)

} // namespace ethercat_sim::framework::logger
