#pragma once

#include "types.hpp"
#include "message.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

namespace mcp {

/**
 * @brief Tool handler interface
 */
class ToolHandler {
public:
    virtual ~ToolHandler() = default;
    
    /**
     * @brief Handle tool list request
     */
    virtual ListToolsResult listTools() = 0;
    
    /**
     * @brief Handle tool call request
     */
    virtual CallToolResult callTool(const CallToolParams& params) = 0;
};

/**
 * @brief Default tool handler implementation
 */
class DefaultToolHandler : public ToolHandler {
private:
    std::unordered_map<std::string, Tool> tools_;
    std::unordered_map<std::string, std::function<CallToolResult(const CallToolParams&)>> tool_handlers_;
    std::mutex tools_mutex_;

public:
    /**
     * @brief Register a tool
     */
    void registerTool(const Tool& tool, std::function<CallToolResult(const CallToolParams&)> handler);
    
    /**
     * @brief Unregister a tool
     */
    void unregisterTool(const std::string& name);
    
    /**
     * @brief Check if tool is registered
     */
    bool hasTool(const std::string& name) const;
    
    /**
     * @brief Get registered tools
     */
    std::vector<Tool> getTools() const;
    
    // ToolHandler interface
    ListToolsResult listTools() override;
    CallToolResult callTool(const CallToolParams& params) override;
};

/**
 * @brief Tool registry for managing tools
 */
class ToolRegistry {
private:
    std::unordered_map<std::string, Tool> tools_;
    std::unordered_map<std::string, std::function<CallToolResult(const CallToolParams&)>> handlers_;
    std::mutex registry_mutex_;

public:
    /**
     * @brief Register a tool
     */
    void registerTool(const Tool& tool, std::function<CallToolResult(const CallToolParams&)> handler);
    
    /**
     * @brief Unregister a tool
     */
    void unregisterTool(const std::string& name);
    
    /**
     * @brief Check if tool is registered
     */
    bool hasTool(const std::string& name) const;
    
    /**
     * @brief Get tool by name
     */
    std::optional<Tool> getTool(const std::string& name) const;
    
    /**
     * @brief Get all registered tools
     */
    std::vector<Tool> getAllTools() const;
    
    /**
     * @brief Get tool handler
     */
    std::optional<std::function<CallToolResult(const CallToolParams&)>> getHandler(const std::string& name) const;
    
    /**
     * @brief Clear all tools
     */
    void clear();
    
    /**
     * @brief Get tool count
     */
    size_t size() const;
};

/**
 * @brief Built-in tools
 */
namespace builtin {

/**
 * @brief Echo tool - echoes back input
 */
CallToolResult echoTool(const CallToolParams& params);

/**
 * @brief Calculator tool - performs basic arithmetic
 */
CallToolResult calculatorTool(const CallToolParams& params);

/**
 * @brief File reader tool - reads file contents
 */
CallToolResult fileReaderTool(const CallToolParams& params);

/**
 * @brief File writer tool - writes content to file
 */
CallToolResult fileWriterTool(const CallToolParams& params);

/**
 * @brief System info tool - returns system information
 */
CallToolResult systemInfoTool(const CallToolParams& params);

/**
 * @brief Date/time tool - returns current date and time
 */
CallToolResult dateTimeTool(const CallToolParams& params);

/**
 * @brief Random number tool - generates random numbers
 */
CallToolResult randomNumberTool(const CallToolParams& params);

/**
 * @brief Hash tool - generates hashes of input
 */
CallToolResult hashTool(const CallToolParams& params);

/**
 * @brief Base64 encode/decode tool
 */
CallToolResult base64Tool(const CallToolParams& params);

/**
 * @brief JSON validator tool
 */
CallToolResult jsonValidatorTool(const CallToolParams& params);

/**
 * @brief Register all built-in tools
 */
void registerBuiltinTools(ToolRegistry& registry);

} // namespace builtin

/**
 * @brief Tool parameter validation
 */
class ToolParameterValidator {
public:
    /**
     * @brief Validate tool parameters
     */
    static bool validateParameters(const Tool& tool, const std::optional<std::unordered_map<std::string, JsonValue>>& arguments);
    
    /**
     * @brief Validate required parameters
     */
    static bool validateRequiredParameters(const std::unordered_map<std::string, JsonValue>& schema,
                                          const std::unordered_map<std::string, JsonValue>& arguments);
    
    /**
     * @brief Validate parameter types
     */
    static bool validateParameterTypes(const std::unordered_map<std::string, JsonValue>& schema,
                                      const std::unordered_map<std::string, JsonValue>& arguments);
    
    /**
     * @brief Get validation error message
     */
    static std::string getValidationError(const Tool& tool, 
                                        const std::optional<std::unordered_map<std::string, JsonValue>>& arguments);
};

/**
 * @brief Tool execution context
 */
struct ToolExecutionContext {
    std::string tool_name;
    std::unordered_map<std::string, JsonValue> arguments;
    std::optional<std::string> user_id;
    std::optional<std::string> session_id;
    std::optional<std::unordered_map<std::string, JsonValue>> metadata;
};

/**
 * @brief Tool execution result
 */
struct ToolExecutionResult {
    bool success;
    std::vector<Content> content;
    std::optional<std::string> error_message;
    std::optional<std::unordered_map<std::string, JsonValue>> metadata;
    std::optional<std::chrono::milliseconds> execution_time;
};

/**
 * @brief Tool executor interface
 */
class ToolExecutor {
public:
    virtual ~ToolExecutor() = default;
    
    /**
     * @brief Execute a tool
     */
    virtual ToolExecutionResult execute(const ToolExecutionContext& context) = 0;
    
    /**
     * @brief Check if tool can be executed
     */
    virtual bool canExecute(const std::string& toolName) = 0;
    
    /**
     * @brief Get tool execution time limit
     */
    virtual std::chrono::milliseconds getExecutionTimeLimit() const = 0;
};

/**
 * @brief Default tool executor implementation
 */
class DefaultToolExecutor : public ToolExecutor {
private:
    ToolRegistry& registry_;
    std::chrono::milliseconds time_limit_;

public:
    DefaultToolExecutor(ToolRegistry& registry, std::chrono::milliseconds timeLimit = std::chrono::seconds(30));
    
    ToolExecutionResult execute(const ToolExecutionContext& context) override;
    bool canExecute(const std::string& toolName) override;
    std::chrono::milliseconds getExecutionTimeLimit() const override;
    
    /**
     * @brief Set execution time limit
     */
    void setExecutionTimeLimit(std::chrono::milliseconds limit);
};

} // namespace mcp