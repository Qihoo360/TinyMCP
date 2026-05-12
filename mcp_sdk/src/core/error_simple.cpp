#include "mcp/error_simple.hpp"
#include <sstream>

namespace mcp {

MCPError::MCPError(ErrorCode code, const std::string& message, const SimpleJson& data)
    : std::runtime_error(message), code_(code), data_(data) {}

MCPError::MCPError(ErrorCode code, const std::string& message)
    : std::runtime_error(message), code_(code), data_(SimpleJson()) {}

ErrorCode MCPError::getCode() const {
    return code_;
}

SimpleJson MCPError::getData() const {
    return data_;
}

std::string MCPError::getCodeString() const {
    switch (code_) {
        case ErrorCode::ParseError: return "ParseError";
        case ErrorCode::InvalidRequest: return "InvalidRequest";
        case ErrorCode::MethodNotFound: return "MethodNotFound";
        case ErrorCode::InvalidParams: return "InvalidParams";
        case ErrorCode::InternalError: return "InternalError";
        case ErrorCode::InvalidMethod: return "InvalidMethod";
        case ErrorCode::ResourceNotFound: return "ResourceNotFound";
        case ErrorCode::ResourceNotAvailable: return "ResourceNotAvailable";
        case ErrorCode::ResourceExhausted: return "ResourceExhausted";
        case ErrorCode::ContentModified: return "ContentModified";
        case ErrorCode::UnknownErrorCode: return "UnknownErrorCode";
        default: return "UnknownError";
    }
}

SimpleJson MCPError::toJson() const {
    SimpleJson error;
    error["code"] = static_cast<double>(code_);
    error["message"] = std::string(what());
    if (!data_.isNull()) {
        error["data"] = data_;
    }
    return error;
}

// Constants are defined in the header file

ErrorCode errorCodeFromInt(int code) {
    switch (code) {
        case JSON_RPC_PARSE_ERROR: return ErrorCode::ParseError;
        case JSON_RPC_INVALID_REQUEST: return ErrorCode::InvalidRequest;
        case JSON_RPC_METHOD_NOT_FOUND: return ErrorCode::MethodNotFound;
        case JSON_RPC_INVALID_PARAMS: return ErrorCode::InvalidParams;
        case JSON_RPC_INTERNAL_ERROR: return ErrorCode::InternalError;
        case MCP_INVALID_METHOD: return ErrorCode::InvalidMethod;
        case MCP_RESOURCE_NOT_FOUND: return ErrorCode::ResourceNotFound;
        case MCP_RESOURCE_NOT_AVAILABLE: return ErrorCode::ResourceNotAvailable;
        case MCP_RESOURCE_EXHAUSTED: return ErrorCode::ResourceExhausted;
        case MCP_CONTENT_MODIFIED: return ErrorCode::ContentModified;
        default: return ErrorCode::UnknownErrorCode;
    }
}

int errorCodeToInt(ErrorCode code) {
    switch (code) {
        case ErrorCode::ParseError: return JSON_RPC_PARSE_ERROR;
        case ErrorCode::InvalidRequest: return JSON_RPC_INVALID_REQUEST;
        case ErrorCode::MethodNotFound: return JSON_RPC_METHOD_NOT_FOUND;
        case ErrorCode::InvalidParams: return JSON_RPC_INVALID_PARAMS;
        case ErrorCode::InternalError: return JSON_RPC_INTERNAL_ERROR;
        case ErrorCode::InvalidMethod: return MCP_INVALID_METHOD;
        case ErrorCode::ResourceNotFound: return MCP_RESOURCE_NOT_FOUND;
        case ErrorCode::ResourceNotAvailable: return MCP_RESOURCE_NOT_AVAILABLE;
        case ErrorCode::ResourceExhausted: return MCP_RESOURCE_EXHAUSTED;
        case ErrorCode::ContentModified: return MCP_CONTENT_MODIFIED;
        default: return -32000; // Unknown error
    }
}

} // namespace mcp