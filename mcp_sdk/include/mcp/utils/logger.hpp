#pragma once

#include <string>
#include <memory>
#include <functional>
#include <mutex>
#include <fstream>
#include <chrono>

namespace mcp {
namespace utils {

/**
 * @brief Log levels
 */
enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
};

/**
 * @brief Log entry
 */
struct LogEntry {
    LogLevel level;
    std::string message;
    std::string timestamp;
    std::string thread_id;
    std::string source_file;
    int source_line;
    std::string function_name;
};

/**
 * @brief Log formatter interface
 */
class LogFormatter {
public:
    virtual ~LogFormatter() = default;
    virtual std::string format(const LogEntry& entry) = 0;
};

/**
 * @brief Default log formatter
 */
class DefaultLogFormatter : public LogFormatter {
public:
    std::string format(const LogEntry& entry) override;
};

/**
 * @brief JSON log formatter
 */
class JsonLogFormatter : public LogFormatter {
public:
    std::string format(const LogEntry& entry) override;
};

/**
 * @brief Log handler interface
 */
class LogHandler {
public:
    virtual ~LogHandler() = default;
    virtual void handle(const LogEntry& entry) = 0;
};

/**
 * @brief Console log handler
 */
class ConsoleLogHandler : public LogHandler {
private:
    std::mutex mutex_;
    bool color_enabled_;
    
public:
    ConsoleLogHandler(bool colorEnabled = true);
    void handle(const LogEntry& entry) override;
};

/**
 * @brief File log handler
 */
class FileLogHandler : public LogHandler {
private:
    std::string filename_;
    std::ofstream file_;
    std::mutex mutex_;
    
public:
    FileLogHandler(const std::string& filename);
    ~FileLogHandler();
    void handle(const LogEntry& entry) override;
};

/**
 * @brief Rotating file log handler
 */
class RotatingFileLogHandler : public LogHandler {
private:
    std::string base_filename_;
    size_t max_file_size_;
    size_t max_files_;
    std::ofstream current_file_;
    size_t current_file_size_;
    std::mutex mutex_;
    
public:
    RotatingFileLogHandler(const std::string& baseFilename, 
                          size_t maxFileSize = 10 * 1024 * 1024, // 10MB
                          size_t maxFiles = 5);
    ~RotatingFileLogHandler();
    void handle(const LogEntry& entry) override;
    
private:
    void rotateFile();
};

/**
 * @brief Custom log handler
 */
class CustomLogHandler : public LogHandler {
private:
    std::function<void(const LogEntry&)> handler_func_;
    
public:
    CustomLogHandler(std::function<void(const LogEntry&)> handlerFunc);
    void handle(const LogEntry& entry) override;
};

/**
 * @brief MCP Logger
 */
class Logger {
private:
    static std::unique_ptr<Logger> instance_;
    static std::mutex instance_mutex_;
    
    LogLevel level_;
    std::unique_ptr<LogFormatter> formatter_;
    std::vector<std::unique_ptr<LogHandler>> handlers_;
    std::mutex handlers_mutex_;
    
    Logger();
    
public:
    ~Logger() = default;
    
    // Singleton access
    static Logger& getInstance();
    
    // Configuration
    void setLevel(LogLevel level);
    LogLevel getLevel() const;
    
    void setFormatter(std::unique_ptr<LogFormatter> formatter);
    
    void addHandler(std::unique_ptr<LogHandler> handler);
    void removeAllHandlers();
    
    // Logging methods
    void log(LogLevel level, const std::string& message, 
            const std::string& sourceFile = "", int sourceLine = 0, 
            const std::string& functionName = "");
    
    void trace(const std::string& message, const std::string& sourceFile = "", 
              int sourceLine = 0, const std::string& functionName = "");
    
    void debug(const std::string& message, const std::string& sourceFile = "", 
              int sourceLine = 0, const std::string& functionName = "");
    
    void info(const std::string& message, const std::string& sourceFile = "", 
             int sourceLine = 0, const std::string& functionName = "");
    
    void warn(const std::string& message, const std::string& sourceFile = "", 
             int sourceLine = 0, const std::string& functionName = "");
    
    void error(const std::string& message, const std::string& sourceFile = "", 
              int sourceLine = 0, const std::string& functionName = "");
    
    void fatal(const std::string& message, const std::string& sourceFile = "", 
              int sourceLine = 0, const std::string& functionName = "");

private:
    std::string getCurrentTimestamp();
    std::string getThreadId();
    bool shouldLog(LogLevel level) const;
};

/**
 * @brief Logging macros
 */
#define MCP_LOG_TRACE(message) \
    mcp::utils::Logger::getInstance().trace(message, __FILE__, __LINE__, __FUNCTION__)

#define MCP_LOG_DEBUG(message) \
    mcp::utils::Logger::getInstance().debug(message, __FILE__, __LINE__, __FUNCTION__)

#define MCP_LOG_INFO(message) \
    mcp::utils::Logger::getInstance().info(message, __FILE__, __LINE__, __FUNCTION__)

#define MCP_LOG_WARN(message) \
    mcp::utils::Logger::getInstance().warn(message, __FILE__, __LINE__, __FUNCTION__)

#define MCP_LOG_ERROR(message) \
    mcp::utils::Logger::getInstance().error(message, __FILE__, __LINE__, __FUNCTION__)

#define MCP_LOG_FATAL(message) \
    mcp::utils::Logger::getInstance().fatal(message, __FILE__, __LINE__, __FUNCTION__)

/**
 * @brief Logger configuration
 */
class LoggerConfig {
public:
    static void configureDefault();
    static void configureForDevelopment();
    static void configureForProduction();
    static void configureMinimal();
    static void configureVerbose();
    
    static void setConsoleOutput(bool enabled);
    static void setFileOutput(const std::string& filename);
    static void setRotatingFileOutput(const std::string& baseFilename, 
                                    size_t maxFileSize = 10 * 1024 * 1024,
                                    size_t maxFiles = 5);
    static void setCustomOutput(std::function<void(const LogEntry&)> handler);
    
    static void setJsonFormat(bool enabled);
    static void setColorOutput(bool enabled);
    static void setLogLevel(LogLevel level);
};

/**
 * @brief Structured logging
 */
class StructuredLogger {
private:
    std::string component_;
    std::string operation_;
    std::unordered_map<std::string, std::string> context_;
    
public:
    StructuredLogger(const std::string& component, const std::string& operation = "");
    
    StructuredLogger& withContext(const std::string& key, const std::string& value);
    StructuredLogger& withOperation(const std::string& operation);
    
    void log(LogLevel level, const std::string& message);
    void trace(const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);
    
private:
    std::string formatContext() const;
};

/**
 * @brief Performance logger
 */
class PerformanceLogger {
private:
    std::string operation_;
    std::chrono::high_resolution_clock::time_point start_time_;
    std::unordered_map<std::string, std::string> context_;
    
public:
    PerformanceLogger(const std::string& operation);
    
    PerformanceLogger& withContext(const std::string& key, const std::string& value);
    
    void finish();
    void finishWithResult(const std::string& result);
    void finishWithError(const std::string& error);
    
private:
    std::chrono::milliseconds getElapsedTime() const;
};

/**
 * @brief RAII performance logger
 */
class ScopedPerformanceLogger {
private:
    std::unique_ptr<PerformanceLogger> logger_;
    
public:
    ScopedPerformanceLogger(const std::string& operation);
    ScopedPerformanceLogger(const std::string& operation, const std::string& key, const std::string& value);
    ~ScopedPerformanceLogger();
    
    ScopedPerformanceLogger& withContext(const std::string& key, const std::string& value);
};

#define MCP_PERF_LOG(operation) \
    mcp::utils::ScopedPerformanceLogger _perf_logger(operation)

#define MCP_PERF_LOG_WITH_CONTEXT(operation, key, value) \
    mcp::utils::ScopedPerformanceLogger _perf_logger(operation, key, value)

} // namespace utils
} // namespace mcp