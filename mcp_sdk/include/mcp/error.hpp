#pragma once

#include "types.hpp"
#include <string>
#include <exception>

namespace mcp {

/**
 * @brief Base MCP exception class
 */
class McpException : public std::exception {
private:
    ErrorCode code_;
    std::string message_;
    std::optional<JsonValue> data_;

public:
    McpException(ErrorCode code, const std::string& message, 
                const std::optional<JsonValue>& data = std::nullopt)
        : code_(code), message_(message), data_(data) {}
    
    ErrorCode getCode() const { return code_; }
    const char* what() const noexcept override { return message_.c_str(); }
    std::optional<JsonValue> getData() const { return data_; }
    
    /**
     * @brief Convert to ErrorDetails
     */
    ErrorDetails toErrorDetails() const;
};

/**
 * @brief JSON-RPC parse error
 */
class ParseError : public McpException {
public:
    ParseError(const std::string& message = "Parse error")
        : McpException(ErrorCode::PARSE_ERROR, message) {}
};

/**
 * @brief Invalid request error
 */
class InvalidRequestError : public McpException {
public:
    InvalidRequestError(const std::string& message = "Invalid Request")
        : McpException(ErrorCode::INVALID_REQUEST, message) {}
};

/**
 * @brief Method not found error
 */
class MethodNotFoundError : public McpException {
public:
    MethodNotFoundError(const std::string& method)
        : McpException(ErrorCode::METHOD_NOT_FOUND, "Method not found: " + method) {}
};

/**
 * @brief Invalid parameters error
 */
class InvalidParamsError : public McpException {
public:
    InvalidParamsError(const std::string& message = "Invalid params")
        : McpException(ErrorCode::INVALID_PARAMS, message) {}
};

/**
 * @brief Internal error
 */
class InternalError : public McpException {
public:
    InternalError(const std::string& message = "Internal error")
        : McpException(ErrorCode::INTERNAL_ERROR, message) {}
};

/**
 * @brief Transport error
 */
class TransportError : public McpException {
public:
    TransportError(const std::string& message = "Transport error")
        : McpException(ErrorCode::TRANSPORT_ERROR, message) {}
};

/**
 * @brief Authentication error
 */
class AuthenticationError : public McpException {
public:
    AuthenticationError(const std::string& message = "Authentication failed")
        : McpException(ErrorCode::AUTHENTICATION_ERROR, message) {}
};

/**
 * @brief Authorization error
 */
class AuthorizationError : public McpException {
public:
    AuthorizationError(const std::string& message = "Authorization failed")
        : McpException(ErrorCode::AUTHORIZATION_ERROR, message) {}
};

/**
 * @brief Resource not found error
 */
class ResourceNotFoundError : public McpException {
public:
    ResourceNotFoundError(const std::string& uri)
        : McpException(ErrorCode::RESOURCE_NOT_FOUND, "Resource not found: " + uri) {}
};

/**
 * @brief Tool execution error
 */
class ToolExecutionError : public McpException {
public:
    ToolExecutionError(const std::string& tool, const std::string& message)
        : McpException(ErrorCode::TOOL_EXECUTION_ERROR, "Tool execution failed: " + tool + " - " + message) {}
};

/**
 * @brief Error factory for creating standardized errors
 */
class ErrorFactory {
public:
    /**
     * @brief Create a parse error
     */
    static ErrorDetails createParseError(const std::string& message = "Parse error");
    
    /**
     * @brief Create an invalid request error
     */
    static ErrorDetails createInvalidRequestError(const std::string& message = "Invalid Request");
    
    /**
     * @brief Create a method not found error
     */
    static ErrorDetails createMethodNotFoundError(const std::string& method);
    
    /**
     * @brief Create an invalid params error
     */
    static ErrorDetails createInvalidParamsError(const std::string& message = "Invalid params");
    
    /**
     * @brief Create an internal error
     */
    static ErrorDetails createInternalError(const std::string& message = "Internal error");
    
    /**
     * @brief Create a transport error
     */
    static ErrorDetails createTransportError(const std::string& message = "Transport error");
    
    /**
     * @brief Create an authentication error
     */
    static ErrorDetails createAuthenticationError(const std::string& message = "Authentication failed");
    
    /**
     * @brief Create an authorization error
     */
    static ErrorDetails createAuthorizationError(const std::string& message = "Authorization failed");
    
    /**
     * @brief Create a resource not found error
     */
    static ErrorDetails createResourceNotFoundError(const std::string& uri);
    
    /**
     * @brief Create a tool execution error
     */
    static ErrorDetails createToolExecutionError(const std::string& tool, const std::string& message);
};

/**
 * @brief Error code to string conversion
 */
class ErrorCodeToString {
public:
    static std::string toString(ErrorCode code);
    static std::string getErrorMessage(ErrorCode code);
};

} // namespace mcp