#include "logger.h"
#include <cstdio>
#include <cstdarg>

namespace ethercat_sim::framework::logger
{

// Static member definitions
LogLevel Logger::current_level_ = LogLevel::DEBUG;
std::string Logger::current_component_ = "default";
std::ostream* Logger::output_stream_ = &std::cout;
bool Logger::timestamp_enabled_ = true;
std::mutex Logger::log_mutex_;

void Logger::setLevel(LogLevel level)
{
    std::lock_guard<std::mutex> lock(log_mutex_);
    current_level_ = level;
}

void Logger::setComponent(const std::string& component)
{
    std::lock_guard<std::mutex> lock(log_mutex_);
    current_component_ = component;
}

void Logger::setOutput(std::ostream& stream)
{
    std::lock_guard<std::mutex> lock(log_mutex_);
    output_stream_ = &stream;
}

void Logger::setTimestampEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(log_mutex_);
    timestamp_enabled_ = enabled;
}

void Logger::debug(const std::string& message)
{
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message)
{
    log(LogLevel::INFO, message);
}

void Logger::warn(const std::string& message)
{
    log(LogLevel::WARN, message);
}

void Logger::error(const std::string& message)
{
    log(LogLevel::ERROR, message);
}


void Logger::log(LogLevel level, const std::string& message)
{
    if (level < current_level_)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(log_mutex_);

    std::ostringstream oss;

    // Add timestamp if enabled (more compact format)
    if (timestamp_enabled_)
    {
        oss << formatTimestamp() << " ";
    }

    // Add log level with padding
    std::string level_str = levelToString(level);
    oss << "[" << std::setw(5) << std::left << level_str << "] ";

    // Add component with padding
    oss << "[" << std::setw(6) << std::left << current_component_ << "] ";

    // Add message
    oss << message;

    // Output to stream with consistent formatting
    *output_stream_ << oss.str() << "\n";
    output_stream_->flush();
}

std::string Logger::formatTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string Logger::levelToString(LogLevel level)
{
    switch (level)
    {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}


} // namespace ethercat_sim::framework::logger