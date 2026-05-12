#pragma once

#include "types.hpp"
#include "message.hpp"
#include "transport.hpp"
#include "tools.hpp"
#include "resources.hpp"
#include "prompts.hpp"
#include <memory>
#include <unordered_map>
#include <functional>

namespace mcp {

/**
 * @brief MCP Server interface
 */
class Server {
private:
    std::unique_ptr<Transport> transport_;
    std::atomic<bool> initialized_;
    std::atomic<bool> running_;
    
    // Server information
    ServerInfo server_info_;
    Capabilities server_capabilities_;
    
    // Message router
    MessageRouter message_router_;
    
    // Handlers
    std::shared_ptr<ToolHandler> tool_handler_;
    std::shared_ptr<ResourceHandler> resource_handler_;
    std::shared_ptr<PromptHandler> prompt_handler_;
    
    // Event handlers
    std::function<void(const std::exception&)> error_handler_;

public:
    /**
     * @brief Constructor
     */
    Server(std::unique_ptr<Transport> transport, const ServerInfo& serverInfo);
    
    /**
     * @brief Destructor
     */
    virtual ~Server();
    
    /**
     * @brief Initialize the server
     */
    bool initialize();
    
    /**
     * @brief Start the server
     */
    bool start();
    
    /**
     * @brief Stop the server
     */
    void stop();
    
    /**
     * @brief Check if server is running
     */
    bool isRunning() const;
    
    /**
     * @brief Check if server is initialized
     */
    bool isInitialized() const;
    
    // Tool management
    
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
    
    // Resource management
    
    /**
     * @brief Register a resource
     */
    void registerResource(const Resource& resource, std::function<ReadResourceResult(const ReadResourceParams&)> handler);
    
    /**
     * @brief Unregister a resource
     */
    void unregisterResource(const std::string& uri);
    
    /**
     * @brief Check if resource is registered
     */
    bool hasResource(const std::string& uri) const;
    
    /**
     * @brief Get registered resources
     */
    std::vector<Resource> getResources() const;
    
    // Prompt management
    
    /**
     * @brief Register a prompt
     */
    void registerPrompt(const Prompt& prompt, std::function<GetPromptResult(const GetPromptParams&)> handler);
    
    /**
     * @brief Unregister a prompt
     */
    void unregisterPrompt(const std::string& name);
    
    /**
     * @brief Check if prompt is registered
     */
    bool hasPrompt(const std::string& name) const;
    
    /**
     * @brief Get registered prompts
     */
    std::vector<Prompt> getPrompts() const;
    
    // Notifications
    
    /**
     * @brief Send a notification
     */
    bool sendNotification(const std::string& method, const std::optional<JsonValue>& params = std::nullopt);
    
    /**
     * @brief Send progress notification
     */
    bool sendProgressNotification(const std::string& progressToken, 
                                 const std::optional<double>& progress = std::nullopt,
                                 const std::optional<double>& total = std::nullopt);
    
    /**
     * @brief Send log notification
     */
    bool sendLogNotification(const std::optional<std::string>& level = std::nullopt,
                           const std::optional<std::string>& data = std::nullopt,
                           const std::optional<std::string>& logger = std::nullopt);
    
    // Event handlers
    
    /**
     * @brief Set error handler
     */
    void setErrorHandler(std::function<void(const std::exception&)> handler);
    
    // Getters
    
    /**
     * @brief Get server info
     */
    const ServerInfo& getServerInfo() const { return server_info_; }
    
    /**
     * @brief Get server capabilities
     */
    const Capabilities& getServerCapabilities() const { return server_capabilities_; }

private:
    /**
     * @brief Handle incoming messages
     */
    void handleMessage(std::unique_ptr<Message> message);
    
    /**
     * @brief Handle initialize request
     */
    std::unique_ptr<Response> handleInitialize(const Request& request);
    
    /**
     * @brief Handle ping request
     */
    std::unique_ptr<Response> handlePing(const Request& request);
    
    /**
     * @brief Handle list tools request
     */
    std::unique_ptr<Response> handleListTools(const Request& request);
    
    /**
     * @brief Handle call tool request
     */
    std::unique_ptr<Response> handleCallTool(const Request& request);
    
    /**
     * @brief Handle list resources request
     */
    std::unique_ptr<Response> handleListResources(const Request& request);
    
    /**
     * @brief Handle read resource request
     */
    std::unique_ptr<Response> handleReadResource(const Request& request);
    
    /**
     * @brief Handle list prompts request
     */
    std::unique_ptr<Response> handleListPrompts(const Request& request);
    
    /**
     * @brief Handle get prompt request
     */
    std::unique_ptr<Response> handleGetPrompt(const Request& request);
    
    /**
     * @brief Handle transport errors
     */
    void handleTransportError(const std::exception& error);
};

/**
 * @brief MCP Server builder for easy configuration
 */
class ServerBuilder {
private:
    std::unique_ptr<Transport> transport_;
    ServerInfo server_info_;
    Capabilities capabilities_;
    std::function<void(const std::exception&)> error_handler_;

public:
    ServerBuilder();
    
    /**
     * @brief Set transport
     */
    ServerBuilder& setTransport(std::unique_ptr<Transport> transport);
    
    /**
     * @brief Set server info
     */
    ServerBuilder& setServerInfo(const ServerInfo& info);
    
    /**
     * @brief Set capabilities
     */
    ServerBuilder& setCapabilities(const Capabilities& capabilities);
    
    /**
     * @brief Set error handler
     */
    ServerBuilder& setErrorHandler(std::function<void(const std::exception&)> handler);
    
    /**
     * @brief Build the server
     */
    std::unique_ptr<Server> build();
};

/**
 * @brief Convenience functions for creating common server configurations
 */
namespace server {

/**
 * @brief Create a stdio server
 */
std::unique_ptr<Server> createStdioServer(const ServerInfo& serverInfo);

/**
 * @brief Create an HTTP server
 */
std::unique_ptr<Server> createHttpServer(const std::string& host, int port,
                                        const ServerInfo& serverInfo);

/**
 * @brief Create a WebSocket server
 */
std::unique_ptr<Server> createWebSocketServer(const std::string& host, int port,
                                             const ServerInfo& serverInfo);

/**
 * @brief Create an SSE server
 */
std::unique_ptr<Server> createSseServer(const std::string& host, int port,
                                       const ServerInfo& serverInfo);

} // namespace server

} // namespace mcp