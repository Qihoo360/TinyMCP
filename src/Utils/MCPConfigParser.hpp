#pragma once
#include "../Public/Core.hpp"
#include <json/json.h>
#include <string>
#include <vector>
#include <unordered_map>

namespace TinyMCP {
namespace Utils {

/**
 * @brief MCP Configuration Parser
 * Handles parsing and validation of MCP server configurations
 */
class MCPConfigParser {
private:
    Json::Value config_json_;
    bool is_valid_;
    std::string validation_error_;

public:
    MCPConfigParser();
    virtual ~MCPConfigParser() = default;

    /**
     * @brief Parse configuration from JSON string
     * @param jsonString JSON configuration string
     * @return true if parsing successful, false otherwise
     */
    bool parseFromString(const std::string& jsonString);

    /**
     * @brief Parse configuration from file
     * @param filePath Path to configuration file
     * @return true if parsing successful, false otherwise
     */
    bool parseFromFile(const std::string& filePath);

    /**
     * @brief Validate the parsed configuration
     * @return true if configuration is valid, false otherwise
     */
    bool validate();

    /**
     * @brief Get validation error message
     * @return Error message if validation failed
     */
    std::string getValidationError() const;

    /**
     * @brief Get parsed configuration as Core::MCPConfiguration
     * @return MCP configuration structure
     */
    Core::MCPConfiguration getConfiguration() const;

    /**
     * @brief Get server name from configuration
     * @return Server name
     */
    std::string getServerName() const;

    /**
     * @brief Get server version from configuration
     * @return Server version
     */
    std::string getServerVersion() const;

    /**
     * @brief Get server description from configuration
     * @return Server description
     */
    std::string getServerDescription() const;

    /**
     * @brief Get list of tools from configuration
     * @return Vector of tool names
     */
    std::vector<std::string> getTools() const;

    /**
     * @brief Get list of resources from configuration
     * @return Vector of resource names
     */
    std::vector<std::string> getResources() const;

    /**
     * @brief Get list of prompts from configuration
     * @return Vector of prompt names
     */
    std::vector<std::string> getPrompts() const;

    /**
     * @brief Get environment variables from configuration
     * @return Map of environment variables
     */
    std::unordered_map<std::string, std::string> getEnvironment() const;

    /**
     * @brief Get settings from configuration
     * @return Map of settings
     */
    std::unordered_map<std::string, std::string> getSettings() const;

    /**
     * @brief Check if configuration has specific tool
     * @param toolName Tool name to check
     * @return true if tool exists, false otherwise
     */
    bool hasTool(const std::string& toolName) const;

    /**
     * @brief Check if configuration has specific resource
     * @param resourceName Resource name to check
     * @return true if resource exists, false otherwise
     */
    bool hasResource(const std::string& resourceName) const;

    /**
     * @brief Get tool configuration
     * @param toolName Tool name
     * @return Tool configuration as JSON
     */
    Json::Value getToolConfig(const std::string& toolName) const;

    /**
     * @brief Get resource configuration
     * @param resourceName Resource name
     * @return Resource configuration as JSON
     */
    Json::Value getResourceConfig(const std::string& resourceName) const;

    /**
     * @brief Generate configuration template
     * @param serverName Server name
     * @param serverVersion Server version
     * @return JSON configuration template
     */
    static Json::Value generateTemplate(const std::string& serverName,
                                      const std::string& serverVersion = "1.0.0");

    /**
     * @brief Save configuration to file
     * @param filePath Path to save configuration
     * @return true if successful, false otherwise
     */
    bool saveToFile(const std::string& filePath) const;

    /**
     * @brief Convert configuration to JSON string
     * @param prettyPrint Whether to pretty print the JSON
     * @return JSON string representation
     */
    std::string toJsonString(bool prettyPrint = true) const;

private:
    bool validateRequiredFields();
    bool validateTools();
    bool validateResources();
    bool validatePrompts();
    bool validateEnvironment();
    std::vector<std::string> parseStringArray(const Json::Value& json) const;
    std::unordered_map<std::string, std::string> parseStringMap(const Json::Value& json) const;
    std::string getStringValue(const Json::Value& json, const std::string& key, const std::string& defaultValue = "") const;
};

/**
 * @brief MCP Configuration Manager
 * Handles multiple configuration files and provides management utilities
 */
class MCPConfigurationManager {
private:
    std::string config_directory_;
    std::unordered_map<std::string, std::unique_ptr<MCPConfigParser>> configurations_;

public:
    MCPConfigurationManager(const std::string& configDir = "./mcp_configs");
    virtual ~MCPConfigurationManager() = default;

    /**
     * @brief Load configuration for a server
     * @param serverName Server name
     * @return true if loaded successfully, false otherwise
     */
    bool loadServerConfiguration(const std::string& serverName);

    /**
     * @brief Save configuration for a server
     * @param serverName Server name
     * @param config Configuration parser
     * @return true if saved successfully, false otherwise
     */
    bool saveServerConfiguration(const std::string& serverName, const MCPConfigParser& config);

    /**
     * @brief Get configuration for a server
     * @param serverName Server name
     * @return Configuration parser (nullptr if not found)
     */
    MCPConfigParser* getServerConfiguration(const std::string& serverName);

    /**
     * @brief List all available server configurations
     * @return Vector of server names
     */
    std::vector<std::string> listServerConfigurations() const;

    /**
     * @brief Remove server configuration
     * @param serverName Server name
     * @return true if removed successfully, false otherwise
     */
    bool removeServerConfiguration(const std::string& serverName);

    /**
     * @brief Validate all configurations
     * @return Map of server names to validation results
     */
    std::unordered_map<std::string, bool> validateAllConfigurations();

    /**
     * @brief Get configuration file path for server
     * @param serverName Server name
     * @return File path
     */
    std::string getConfigFilePath(const std::string& serverName) const;

private:
    void loadAllConfigurations();
    std::string getServerConfigFileName(const std::string& serverName) const;
};

} // namespace Utils
} // namespace TinyMCP