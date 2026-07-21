#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <optional>
#include <chrono>

namespace mcp {

/**
 * @brief MCP JSON value type
 */
using JsonValue = std::variant<
    std::string,
    int64_t,
    double,
    bool,
    std::nullptr_t,
    std::vector<JsonValue>,
    std::unordered_map<std::string, JsonValue>
>;

/**
 * @brief MCP Request ID type
 */
using RequestId = std::variant<std::string, int64_t, std::nullptr_t>;

/**
 * @brief MCP URI type
 */
using Uri = std::string;

/**
 * @brief MCP MIME type
 */
using MimeType = std::string;

/**
 * @brief MCP Name type
 */
using Name = std::string;

/**
 * @brief MCP Description type
 */
using Description = std::string;

/**
 * @brief MCP Version type
 */
using Version = std::string;

/**
 * @brief MCP Timestamp type
 */
using Timestamp = std::chrono::system_clock::time_point;

/**
 * @brief MCP Client information
 */
struct ClientInfo {
    Name name;
    Version version;
    std::optional<Description> description;
    std::optional<std::string> vendor;
    std::optional<std::unordered_map<std::string, JsonValue>> metadata;
};

/**
 * @brief MCP Server information
 */
struct ServerInfo {
    Name name;
    Version version;
    std::optional<Description> description;
    std::optional<std::string> vendor;
    std::optional<std::unordered_map<std::string, JsonValue>> metadata;
};

/**
 * @brief MCP Protocol version information
 */
struct ProtocolVersion {
    std::string protocol_version;
    std::vector<std::string> capabilities;
    std::optional<std::unordered_map<std::string, JsonValue>> server_info;
};

/**
 * @brief MCP Content item
 */
struct Content {
    ContentType type;
    std::optional<std::string> text;
    std::optional<std::string> data; // Base64 encoded for binary content
    std::optional<MimeType> mimeType;
    std::optional<Uri> uri;
};

/**
 * @brief MCP Tool parameter schema
 */
struct ParameterSchema {
    std::string name;
    std::string description;
    ParameterType type;
    bool required = false;
    std::optional<JsonValue> default_value;
    std::optional<std::vector<JsonValue>> enum_values;
    std::optional<JsonValue> items; // For array types
    std::optional<std::unordered_map<std::string, JsonValue>> properties; // For object types
};

/**
 * @brief MCP Tool definition
 */
struct Tool {
    Name name;
    Description description;
    std::optional<std::unordered_map<std::string, JsonValue>> inputSchema;
    std::optional<std::unordered_map<std::string, JsonValue>> metadata;
};

/**
 * @brief MCP Resource definition
 */
struct Resource {
    Uri uri;
    Name name;
    Description description;
    std::optional<MimeType> mimeType;
    std::optional<std::unordered_map<std::string, JsonValue>> metadata;
};

/**
 * @brief MCP Prompt argument
 */
struct PromptArgument {
    Name name;
    Description description;
    bool required = false;
    std::optional<JsonValue> default_value;
};

/**
 * @brief MCP Prompt definition
 */
struct Prompt {
    Name name;
    Description description;
    std::optional<std::vector<PromptArgument>> arguments;
    std::optional<std::unordered_map<std::string, JsonValue>> metadata;
};

/**
 * @brief MCP Capabilities
 */
struct Capabilities {
    bool experimental = false;
    bool logging = false;
    bool prompts = false;
    bool resources = false;
    bool tools = false;
    std::optional<std::unordered_map<std::string, JsonValue>> additional;
};

/**
 * @brief MCP Initialize request parameters
 */
struct InitializeParams {
    ProtocolVersion protocolVersion;
    std::optional<ClientInfo> clientInfo;
    std::optional<Capabilities> capabilities;
    std::optional<std::unordered_map<std::string, JsonValue>> metadata;
};

/**
 * @brief MCP Initialize response
 */
struct InitializeResult {
    ProtocolVersion protocolVersion;
    std::optional<ServerInfo> serverInfo;
    Capabilities capabilities;
    std::optional<std::unordered_map<std::string, JsonValue>> metadata;
};

/**
 * @brief MCP List tools response
 */
struct ListToolsResult {
    std::vector<Tool> tools;
    std::optional<std::unordered_map<std::string, JsonValue>> metadata;
};

/**
 * @brief MCP Call tool request parameters
 */
struct CallToolParams {
    Name name;
    std::optional<std::unordered_map<std::string, JsonValue>> arguments;
};

/**
 * @brief MCP Call tool response
 */
struct CallToolResult {
    std::vector<Content> content;
    std::optional<bool> isError;
    std::optional<std::unordered_map<std::string, JsonValue>> metadata;
};

/**
 * @brief MCP List resources response
 */
struct ListResourcesResult {
    std::vector<Resource> resources;
    std::optional<std::unordered_map<std::string, JsonValue>> metadata;
};

/**
 * @brief MCP Read resource request parameters
 */
struct ReadResourceParams {
    Uri uri;
};

/**
 * @brief MCP Read resource response
 */
struct ReadResourceResult {
    std::vector<Content> contents;
    std::optional<std::unordered_map<std::string, JsonValue>> metadata;
};

/**
 * @brief MCP List prompts response
 */
struct ListPromptsResult {
    std::vector<Prompt> prompts;
    std::optional<std::unordered_map<std::string, JsonValue>> metadata;
};

/**
 * @brief MCP Get prompt request parameters
 */
struct GetPromptParams {
    Name name;
    std::optional<std::unordered_map<std::string, JsonValue>> arguments;
};

/**
 * @brief MCP Get prompt response
 */
struct GetPromptResult {
    std::vector<Content> description;
    std::vector<Content> messages;
    std::optional<std::unordered_map<std::string, JsonValue>> metadata;
};

/**
 * @brief MCP Progress notification parameters
 */
struct ProgressParams {
    std::string progressToken;
    std::optional<double> progress;
    std::optional<double> total;
};

/**
 * @brief MCP Log notification parameters
 */
struct LogParams {
    std::optional<std::string> level;
    std::optional<std::string> data;
    std::optional<Uri> logger;
};

/**
 * @brief MCP Error details
 */
struct ErrorDetails {
    ErrorCode code;
    std::string message;
    std::optional<JsonValue> data;
};

} // namespace mcp