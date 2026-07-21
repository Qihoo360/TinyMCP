#pragma once
#include <stdexcept>
#include <string>
#include "types_simple.hpp"

namespace mcp {

enum class ErrorCode {
    // JSON-RPC 2.0 standard errors
    ParseError = -32700,
    InvalidRequest = -32600,
    MethodNotFound = -32601,
    InvalidParams = -32602,
    InternalError = -32603,
    
    // MCP-specific errors
    InvalidMethod = -32604,
    ResourceNotFound = -32606,
    ResourceNotAvailable = -32607,
    ResourceExhausted = -32608,
    ContentModified = -32609,
    
    // Unknown error
    UnknownErrorCode = -32000
};

class MCPError : public std::runtime_error {
private:
    ErrorCode code_;
    SimpleJson data_;

public:
    explicit MCPError(ErrorCode code, const std::string& message, const SimpleJson& data);
    explicit MCPError(ErrorCode code, const std::string& message);
    
    ErrorCode getCode() const;
    SimpleJson getData() const;
    std::string getCodeString() const;
    SimpleJson toJson() const;
};

// Standard JSON-RPC error codes
const int JSON_RPC_PARSE_ERROR = -32700;
const int JSON_RPC_INVALID_REQUEST = -32600;
const int JSON_RPC_METHOD_NOT_FOUND = -32601;
const int JSON_RPC_INVALID_PARAMS = -32602;
const int JSON_RPC_INTERNAL_ERROR = -32603;

// MCP-specific error codes
const int MCP_INVALID_METHOD = -32604;
const int MCP_INVALID_PARAMS = -32605;
const int MCP_RESOURCE_NOT_FOUND = -32606;
const int MCP_RESOURCE_NOT_AVAILABLE = -32607;
const int MCP_RESOURCE_EXHAUSTED = -32608;
const int MCP_CONTENT_MODIFIED = -32609;

ErrorCode errorCodeFromInt(int code);
int errorCodeToInt(ErrorCode code);

} // namespace mcp