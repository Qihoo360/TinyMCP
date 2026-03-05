#pragma once

#include "types.hpp"
#include "message.hpp"
#include "transport.hpp"
#include <memory>
#include <future>
#include <unordered_map>
#include <functional>

namespace mcp {

/**
 * @brief MCP Client interface
 */
class Client {
private:
    std::unique_ptr<Transport> transport_;
    std::atomic<bool> initialized_;
    std::atomic<bool> running_;
    
    // Client information
    ClientInfo client_info_;
    Capabilities client_capabilities_;
    
    // Server information
    std::optional<ServerInfo> server_info_;
    std::optional<Capabilities> server_capabilities_;
    
    // Request ID counter
    std::atomic<int64_t> request_id_counter_;
    
    // Pending requests
    std::unordered_map<RequestId, std::promise<std::unique_ptr<Response>>> pending_requests_;
    std::mutex pending_requests_mutex_;
    
    // Event handlers
    std::function<void(const Notification&)> notification_handler_;
    std::function<void(const std::exception&)> error_handler_;

public:
    /**
     * @brief Constructor
     */
    Client(std::unique_ptr<Transport> transport, const ClientInfo& clientInfo);
    
    /**
     * @brief Destructor
     */
    virtual ~Client();
    
    /**
     * @brief Initialize the client and perform handshake with server
     */
    std::future<bool> initialize(const Capabilities& capabilities = Capabilities{});
    
    /**
     * @brief Start the client
     */
    bool start();
    
    /**
     * @brief Stop the client
     */
    void stop();
    
    /**
     * @brief Check if client is running
     */
    bool isRunning() const;
    
    /**
     * @brief Check if client is initialized
     */
    bool isInitialized() const;
    
    // Core MCP methods
    
    /**
     * @brief Send ping to server
     */
    std::future<bool> ping();
    
    /**
     * @brief List available tools
     */
    std::future<ListToolsResult> listTools();
    
    /**
     * @brief Call a tool
     */
    std::future<CallToolResult> callTool(const std::string& name, 
                                        const std::optional<std::unordered_map<std::string, JsonValue>>& arguments = std::nullopt);
    
    /**
     * @brief List available resources
     */
    std::future<ListResourcesResult> listResources();
    
    /**
     * @brief Read a resource
     */
    std::future<ReadResourceResult> readResource(const std::string& uri);
    
    /**
     * @brief List available prompts
     */
    std::future<ListPromptsResult> listPrompts();
    
    /**
     * @brief Get a prompt
     */
    std::future<GetPromptResult> getPrompt(const std::string& name,
                                          const std::optional<std::unordered_map<std::string, JsonValue>>& arguments = std::nullopt);
    
    // Event handlers
    
    /**
     * @brief Set notification handler
     */
    void setNotificationHandler(std::function<void(const Notification&)> handler);
    
    /**
     * @brief Set error handler
     */
    void setErrorHandler(std::function<void(const std::exception&)> handler);
    
    // Getters
    
    /**
     * @brief Get client info
     */
    const ClientInfo& getClientInfo() const { return client_info_; }
    
    /**
     * @brief Get server info
     */
    const std::optional<ServerInfo>& getServerInfo() const { return server_info_; }
    
    /**
     * @brief Get client capabilities
     */
    const Capabilities& getClientCapabilities() const { return client_capabilities_; }
    
    /**
     * @brief Get server capabilities
     */
    const std::optional<Capabilities>& getServerCapabilities() const { return server_capabilities_; }

private:
    /**
     * @brief Generate next request ID
     */
    RequestId generateRequestId();
    
    /**
     * @brief Send request and wait for response
     */
    std::future<std::unique_ptr<Response>> sendRequest(const Request& request);
    
    /**
     * @brief Handle incoming messages
     */
    void handleMessage(std::unique_ptr<Message> message);
    
    /**
     * @brief Handle incoming response
     */
    void handleResponse(std::unique_ptr<Response> response);
    
    /**
     * @brief Handle incoming notification
     */
    void handleNotification(std::unique_ptr<Notification> notification);
    
    /**
     * @brief Handle transport errors
     */
    void handleTransportError(const std::exception& error);
};

/**
 * @brief MCP Client builder for easy configuration
 */
class ClientBuilder {
private:
    std::unique_ptr<Transport> transport_;
    ClientInfo client_info_;
    Capabilities capabilities_;
    std::function<void(const Notification&)> notification_handler_;
    std::function<void(const std::exception&)> error_handler_;

public:
    ClientBuilder();
    
    /**
     * @brief Set transport
     */
    ClientBuilder& setTransport(std::unique_ptr<Transport> transport);
    
    /**
     * @brief Set client info
     */
    ClientBuilder& setClientInfo(const ClientInfo& info);
    
    /**
     * @brief Set capabilities
     */
    ClientBuilder& setCapabilities(const Capabilities& capabilities);
    
    /**
     * @brief Set notification handler
     */
    ClientBuilder& setNotificationHandler(std::function<void(const Notification&)> handler);
    
    /**
     * @brief Set error handler
     */
    ClientBuilder& setErrorHandler(std::function<void(const std::exception&)> handler);
    
    /**
     * @brief Build the client
     */
    std::unique_ptr<Client> build();
};

/**
 * @brief Convenience functions for creating common client configurations
 */
namespace client {

/**
 * @brief Create a stdio client
 */
std::unique_ptr<Client> createStdioClient(const ClientInfo& clientInfo);

/**
 * @brief Create an HTTP client
 */
std::unique_ptr<Client> createHttpClient(const std::string& baseUrl, 
                                        const ClientInfo& clientInfo,
                                        const std::string& authToken = "");

/**
 * @brief Create a WebSocket client
 */
std::unique_ptr<Client> createWebSocketClient(const std::string& wsUrl,
                                            const ClientInfo& clientInfo,
                                            const std::string& authToken = "");

/**
 * @brief Create an SSE client
 */
std::unique_ptr<Client> createSseClient(const std::string& sseUrl,
                                       const ClientInfo& clientInfo,
                                       const std::string& authToken = "");

} // namespace client

} // namespace mcp