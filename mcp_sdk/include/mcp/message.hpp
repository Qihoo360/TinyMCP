#pragma once

#include "types.hpp"
#include <string>
#include <optional>
#include <unordered_map>

namespace mcp {

/**
 * @brief Base class for all MCP messages
 */
class Message {
public:
    virtual ~Message() = default;
    
    /**
     * @brief Get the JSON-RPC version
     */
    virtual std::string getJsonRpcVersion() const = 0;
    
    /**
     * @brief Serialize the message to JSON string
     */
    virtual std::string serialize() const = 0;
    
    /**
     * @brief Deserialize JSON string to message
     */
    virtual bool deserialize(const std::string& json) = 0;
    
    /**
     * @brief Get message type
     */
    virtual MessageType getType() const = 0;
};

/**
 * @brief MCP Request message
 */
class Request : public Message {
private:
    std::string jsonrpc_;
    RequestId id_;
    std::string method_;
    std::optional<JsonValue> params_;

public:
    Request();
    Request(const RequestId& id, const std::string& method, 
            const std::optional<JsonValue>& params = std::nullopt);
    
    // Message interface
    std::string getJsonRpcVersion() const override;
    std::string serialize() const override;
    bool deserialize(const std::string& json) override;
    MessageType getType() const override;
    
    // Getters and setters
    RequestId getId() const { return id_; }
    void setId(const RequestId& id) { id_ = id; }
    
    std::string getMethod() const { return method_; }
    void setMethod(const std::string& method) { method_ = method; }
    
    std::optional<JsonValue> getParams() const { return params_; }
    void setParams(const std::optional<JsonValue>& params) { params_ = params; }
    
    // Helper methods
    bool isNotification() const;
    std::string getMethodName() const;
};

/**
 * @brief MCP Response message
 */
class Response : public Message {
private:
    std::string jsonrpc_;
    RequestId id_;
    std::optional<JsonValue> result_;
    std::optional<ErrorDetails> error_;

public:
    Response();
    Response(const RequestId& id, const JsonValue& result);
    Response(const RequestId& id, const ErrorDetails& error);
    
    // Message interface
    std::string getJsonRpcVersion() const override;
    std::string serialize() const override;
    bool deserialize(const std::string& json) override;
    MessageType getType() const override;
    
    // Getters and setters
    RequestId getId() const { return id_; }
    void setId(const RequestId& id) { id_ = id; }
    
    std::optional<JsonValue> getResult() const { return result_; }
    void setResult(const JsonValue& result) { result_ = result; }
    
    std::optional<ErrorDetails> getError() const { return error_; }
    void setError(const ErrorDetails& error) { error_ = error; }
    
    // Helper methods
    bool isError() const;
    bool isSuccess() const;
};

/**
 * @brief MCP Notification message
 */
class Notification : public Message {
private:
    std::string jsonrpc_;
    std::string method_;
    std::optional<JsonValue> params_;

public:
    Notification();
    Notification(const std::string& method, 
                const std::optional<JsonValue>& params = std::nullopt);
    
    // Message interface
    std::string getJsonRpcVersion() const override;
    std::string serialize() const override;
    bool deserialize(const std::string& json) override;
    MessageType getType() const override;
    
    // Getters and setters
    std::string getMethod() const { return method_; }
    void setMethod(const std::string& method) { method_ = method; }
    
    std::optional<JsonValue> getParams() const { return params_; }
    void setParams(const std::optional<JsonValue>& params) { params_ = params; }
    
    // Helper methods
    std::string getMethodName() const;
};

/**
 * @brief Message factory for creating messages from JSON
 */
class MessageFactory {
public:
    /**
     * @brief Create a message from JSON string
     */
    static std::unique_ptr<Message> createFromJson(const std::string& json);
    
    /**
     * @brief Create a request message
     */
    static std::unique_ptr<Request> createRequest(const RequestId& id, 
                                                 const std::string& method,
                                                 const std::optional<JsonValue>& params = std::nullopt);
    
    /**
     * @brief Create a response message
     */
    static std::unique_ptr<Response> createResponse(const RequestId& id, 
                                                   const JsonValue& result);
    
    /**
     * @brief Create an error response message
     */
    static std::unique_ptr<Response> createErrorResponse(const RequestId& id, 
                                                        const ErrorDetails& error);
    
    /**
     * @brief Create a notification message
     */
    static std::unique_ptr<Notification> createNotification(const std::string& method,
                                                           const std::optional<JsonValue>& params = std::nullopt);
};

/**
 * @brief Message handler interface
 */
class MessageHandler {
public:
    virtual ~MessageHandler() = default;
    
    /**
     * @brief Handle a request message
     */
    virtual std::unique_ptr<Response> handleRequest(const Request& request) = 0;
    
    /**
     * @brief Handle a notification message
     */
    virtual void handleNotification(const Notification& notification) = 0;
};

/**
 * @brief Message router for dispatching messages to appropriate handlers
 */
class MessageRouter {
private:
    std::unordered_map<std::string, std::shared_ptr<MessageHandler>> handlers_;
    
public:
    /**
     * @brief Register a message handler for a specific method
     */
    void registerHandler(const std::string& method, std::shared_ptr<MessageHandler> handler);
    
    /**
     * @brief Unregister a message handler
     */
    void unregisterHandler(const std::string& method);
    
    /**
     * @brief Route a message to the appropriate handler
     */
    std::unique_ptr<Response> routeMessage(std::unique_ptr<Message> message);
    
    /**
     * @brief Check if a handler exists for a method
     */
    bool hasHandler(const std::string& method) const;
};

} // namespace mcp