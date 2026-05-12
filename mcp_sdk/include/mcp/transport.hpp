#pragma once

#include "types.hpp"
#include "message.hpp"
#include <string>
#include <functional>
#include <memory>
#include <future>

namespace mcp {

/**
 * @brief Transport layer interface
 */
class Transport {
public:
    virtual ~Transport() = default;
    
    /**
     * @brief Initialize the transport
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Start the transport
     */
    virtual bool start() = 0;
    
    /**
     * @brief Stop the transport
     */
    virtual void stop() = 0;
    
    /**
     * @brief Check if transport is running
     */
    virtual bool isRunning() const = 0;
    
    /**
     * @brief Send a message
     */
    virtual bool sendMessage(const Message& message) = 0;
    
    /**
     * @brief Send a request and wait for response
     */
    virtual std::future<std::unique_ptr<Response>> sendRequest(const Request& request) = 0;
    
    /**
     * @brief Set message handler callback
     */
    virtual void setMessageHandler(std::function<void(std::unique_ptr<Message>)> handler) = 0;
    
    /**
     * @brief Set error handler callback
     */
    virtual void setErrorHandler(std::function<void(const std::exception&)> handler) = 0;
};

/**
 * @brief Stdio transport implementation
 */
class StdioTransport : public Transport {
private:
    bool running_;
    std::function<void(std::unique_ptr<Message>)> message_handler_;
    std::function<void(const std::exception&)> error_handler_;
    std::unordered_map<RequestId, std::promise<std::unique_ptr<Response>>> pending_requests_;
    std::mutex pending_requests_mutex_;
    
    void readLoop();
    void processLine(const std::string& line);
    
public:
    StdioTransport();
    virtual ~StdioTransport();
    
    bool initialize() override;
    bool start() override;
    void stop() override;
    bool isRunning() const override;
    bool sendMessage(const Message& message) override;
    std::future<std::unique_ptr<Response>> sendRequest(const Request& request) override;
    void setMessageHandler(std::function<void(std::unique_ptr<Message>)> handler) override;
    void setErrorHandler(std::function<void(const std::exception&)> handler) override;
};

/**
 * @brief HTTP transport implementation
 */
class HttpTransport : public Transport {
private:
    std::string base_url_;
    std::string auth_token_;
    bool running_;
    std::function<void(std::unique_ptr<Message>)> message_handler_;
    std::function<void(const std::exception&)> error_handler_;
    std::unordered_map<RequestId, std::promise<std::unique_ptr<Response>>> pending_requests_;
    std::mutex pending_requests_mutex_;
    
    std::string makeHttpRequest(const std::string& method, const std::string& path, 
                               const std::string& body = "");
    
public:
    HttpTransport(const std::string& baseUrl, const std::string& authToken = "");
    virtual ~HttpTransport();
    
    bool initialize() override;
    bool start() override;
    void stop() override;
    bool isRunning() const override;
    bool sendMessage(const Message& message) override;
    std::future<std::unique_ptr<Response>> sendRequest(const Request& request) override;
    void setMessageHandler(std::function<void(std::unique_ptr<Message>)> handler) override;
    void setErrorHandler(std::function<void(const std::exception&)> handler) override;
};

/**
 * @brief WebSocket transport implementation
 */
class WebSocketTransport : public Transport {
private:
    std::string ws_url_;
    std::string auth_token_;
    bool running_;
    std::function<void(std::unique_ptr<Message>)> message_handler_;
    std::function<void(const std::exception&)> error_handler_;
    std::unordered_map<RequestId, std::promise<std::unique_ptr<Response>>> pending_requests_;
    std::mutex pending_requests_mutex_;
    
    void connect();
    void disconnect();
    void onMessage(const std::string& message);
    void onError(const std::exception& error);
    
public:
    WebSocketTransport(const std::string& wsUrl, const std::string& authToken = "");
    virtual ~WebSocketTransport();
    
    bool initialize() override;
    bool start() override;
    void stop() override;
    bool isRunning() const override;
    bool sendMessage(const Message& message) override;
    std::future<std::unique_ptr<Response>> sendRequest(const Request& request) override;
    void setMessageHandler(std::function<void(std::unique_ptr<Message>)> handler) override;
    void setErrorHandler(std::function<void(const std::exception&)> handler) override;
};

/**
 * @brief Server-Sent Events (SSE) transport implementation
 */
class SseTransport : public Transport {
private:
    std::string sse_url_;
    std::string auth_token_;
    bool running_;
    std::function<void(std::unique_ptr<Message>)> message_handler_;
    std::function<void(const std::exception&)> error_handler_;
    std::unordered_map<RequestId, std::promise<std::unique_ptr<Response>>> pending_requests_;
    std::mutex pending_requests_mutex_;
    
    void connect();
    void disconnect();
    void eventLoop();
    void processEvent(const std::string& event);
    
public:
    SseTransport(const std::string& sseUrl, const std::string& authToken = "");
    virtual ~SseTransport();
    
    bool initialize() override;
    bool start() override;
    void stop() override;
    bool isRunning() const override;
    bool sendMessage(const Message& message) override;
    std::future<std::unique_ptr<Response>> sendRequest(const Request& request) override;
    void setMessageHandler(std::function<void(std::unique_ptr<Message>)> handler) override;
    void setErrorHandler(std::function<void(const std::exception&)> handler) override;
};

/**
 * @brief Transport factory
 */
class TransportFactory {
public:
    /**
     * @brief Create a transport based on type
     */
    static std::unique_ptr<Transport> createTransport(TransportType type, 
                                                     const std::string& url = "",
                                                     const std::string& authToken = "");
    
    /**
     * @brief Create stdio transport
     */
    static std::unique_ptr<StdioTransport> createStdioTransport();
    
    /**
     * @brief Create HTTP transport
     */
    static std::unique_ptr<HttpTransport> createHttpTransport(const std::string& baseUrl,
                                                            const std::string& authToken = "");
    
    /**
     * @brief Create WebSocket transport
     */
    static std::unique_ptr<WebSocketTransport> createWebSocketTransport(const std::string& wsUrl,
                                                                       const std::string& authToken = "");
    
    /**
     * @brief Create SSE transport
     */
    static std::unique_ptr<SseTransport> createSseTransport(const std::string& sseUrl,
                                                          const std::string& authToken = "");
};

} // namespace mcp