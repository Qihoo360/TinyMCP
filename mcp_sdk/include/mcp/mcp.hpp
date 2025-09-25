#pragma once

/**
 * @file mcp.hpp
 * @brief Model Context Protocol (MCP) C++ SDK
 * 
 * This is a comprehensive C++ SDK for the Model Context Protocol (MCP),
 * providing both client and server implementations with full protocol support.
 * 
 * @version 1.0.0
 * @author TinyMCP Team
 * @date 2025
 */

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <variant>
#include <optional>
#include <chrono>

namespace mcp {

/**
 * @brief MCP Protocol version
 */
inline constexpr const char* PROTOCOL_VERSION = "2024-11-05";

/**
 * @brief JSON-RPC version
 */
inline constexpr const char* JSON_RPC_VERSION = "2.0";

/**
 * @brief Core MCP types and forward declarations
 */
using RequestId = std::variant<std::string, int64_t, std::nullptr_t>;
using JsonValue = std::variant<std::string, int64_t, double, bool, std::nullptr_t, 
                              std::vector<JsonValue>, std::unordered_map<std::string, JsonValue>>;

/**
 * @brief MCP Error codes
 */
enum class ErrorCode : int {
    // JSON-RPC 2.0 standard errors
    PARSE_ERROR = -32700,
    INVALID_REQUEST = -32600,
    METHOD_NOT_FOUND = -32601,
    INVALID_PARAMS = -32602,
    INTERNAL_ERROR = -32603,
    
    // MCP-specific errors
    SERVER_ERROR_FIRST = -32000,
    INVALID_RESPONSE = -32001,
    INVALID_NOTIFICATION = -32002,
    TRANSPORT_ERROR = -32003,
    AUTHENTICATION_ERROR = -32004,
    AUTHORIZATION_ERROR = -32005,
    RESOURCE_NOT_FOUND = -32006,
    TOOL_EXECUTION_ERROR = -32007,
    SERVER_ERROR_LAST = -32099
};

/**
 * @brief MCP Message types
 */
enum class MessageType {
    REQUEST,
    RESPONSE,
    NOTIFICATION
};

/**
 * @brief MCP Content types
 */
enum class ContentType {
    TEXT,
    IMAGE,
    RESOURCE
};

/**
 * @brief MCP Tool parameter types
 */
enum class ParameterType {
    STRING,
    NUMBER,
    BOOLEAN,
    OBJECT,
    ARRAY,
    NULL_TYPE
};

/**
 * @brief MCP Transport types
 */
enum class TransportType {
    STDIO,
    HTTP,
    WEBSOCKET,
    SSE
};

} // namespace mcp

// Core protocol components
#include "mcp/types.hpp"
#include "mcp/message.hpp"
#include "mcp/error.hpp"
#include "mcp/transport.hpp"
#include "mcp/client.hpp"
#include "mcp/server.hpp"
#include "mcp/tools.hpp"
#include "mcp/resources.hpp"
#include "mcp/prompts.hpp"
#include "mcp/capabilities.hpp"

// Utilities
#include "mcp/utils/json.hpp"
#include "mcp/utils/logger.hpp"
#include "mcp/utils/thread_pool.hpp"

// Examples and helpers
#include "mcp/examples/basic_server.hpp"
#include "mcp/examples/basic_client.hpp"