#include "mcp/utils/logger.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <mutex>

namespace mcp {
namespace utils {

static std::mutex log_mutex;

Logger::Logger(LogLevel level) : level_(level) {}

void Logger::setLevel(LogLevel level) {
    level_ = level;
}

LogLevel Logger::getLevel() const {
    return level_;
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

std::string Logger::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

void Logger::log(LogLevel level, const std::string& message) const {
    if (level < level_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(log_mutex);
    
    std::cout << "[" << getTimestamp() << "] "
              << "[" << levelToString(level) << "] "
              << message << std::endl;
}

void Logger::debug(const std::string& message) const {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) const {
    log(LogLevel::INFO, message);
}

void Logger::warn(const std::string& message) const {
    log(LogLevel::WARN, message);
}

void Logger::error(const std::string& message) const {
    log(LogLevel::ERROR, message);
}

// Global logger instance
static Logger global_logger(LogLevel::INFO);

Logger& getLogger() {
    return global_logger;
}

void setGlobalLogLevel(LogLevel level) {
    global_logger.setLevel(level);
}

} // namespace utils
} // namespace mcp