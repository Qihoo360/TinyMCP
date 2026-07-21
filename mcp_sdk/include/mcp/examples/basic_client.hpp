#pragma once

#include "../client.hpp"
#include <memory>
#include <future>

namespace mcp {
namespace examples {

/**
 * @brief Basic MCP client example
 */
class BasicClient {
private:
    std::unique_ptr<Client> client_;
    bool initialized_;

public:
    BasicClient();
    explicit BasicClient(std::unique_ptr<Client> client);
    ~BasicClient() = default;
    
    /**
     * @brief Initialize the client
     */
    std::future<bool> initialize();
    
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
    
    // Tool operations
    
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
     * @brief Echo a message
     */
    std::future<std::string> echo(const std::string& message);
    
    /**
     * @brief Calculate an expression
     */
    std::future<double> calculate(const std::string& expression);
    
    /**
     * @brief Read a file
     */
    std::future<std::string> readFile(const std::string& filename);
    
    /**
     * @brief Write to a file
     */
    std::future<bool> writeFile(const std::string& filename, const std::string& content);
    
    /**
     * @brief Get system information
     */
    std::future<std::string> getSystemInfo();
    
    /**
     * @brief Get current date and time
     */
    std::future<std::string> getCurrentDateTime();
    
    /**
     * @brief Generate a random number
     */
    std::future<int> generateRandomNumber(int min = 0, int max = 100);
    
    /**
     * @brief Generate a hash
     */
    std::future<std::string> generateHash(const std::string& input, const std::string& algorithm = "sha256");
    
    /**
     * @brief Base64 encode/decode
     */
    std::future<std::string> base64Encode(const std::string& input);
    std::future<std::string> base64Decode(const std::string& input);
    
    /**
     * @brief Validate JSON
     */
    std::future<bool> validateJson(const std::string& json);
    
    // Resource operations
    
    /**
     * @brief List available resources
     */
    std::future<ListResourcesResult> listResources();
    
    /**
     * @brief Read a resource
     */
    std::future<ReadResourceResult> readResource(const std::string& uri);
    
    /**
     * @brief Read a file resource
     */
    std::future<std::string> readFileResource(const std::string& filepath);
    
    /**
     * @brief Read an HTTP resource
     */
    std::future<std::string> readHttpResource(const std::string& url);
    
    /**
     * @brief Read a memory resource
     */
    std::future<std::string> readMemoryResource(const std::string& key);
    
    /**
     * @brief Read an environment resource
     */
    std::future<std::string> readEnvironmentResource(const std::string& variable);
    
    /**
     * @brief Read a configuration resource
     */
    std::future<std::string> readConfigurationResource(const std::string& key);
    
    // Prompt operations
    
    /**
     * @brief List available prompts
     */
    std::future<ListPromptsResult> listPrompts();
    
    /**
     * @brief Get a prompt
     */
    std::future<GetPromptResult> getPrompt(const std::string& name,
                                          const std::optional<std::unordered_map<std::string, JsonValue>>& arguments = std::nullopt);
    
    /**
     * @brief Get code review prompt
     */
    std::future<GetPromptResult> getCodeReviewPrompt(const std::string& code, 
                                                    const std::string& language = "cpp");
    
    /**
     * @brief Get bug fix prompt
     */
    std::future<GetPromptResult> getBugFixPrompt(const std::string& error, 
                                                const std::string& code = "");
    
    /**
     * @brief Get documentation prompt
     */
    std::future<GetPromptResult> getDocumentationPrompt(const std::string& code, 
                                                       const std::string& language = "cpp");
    
    /**
     * @brief Get test generation prompt
     */
    std::future<GetPromptResult> getTestGenerationPrompt(const std::string& code, 
                                                        const std::string& language = "cpp");
    
    /**
     * @brief Get refactoring prompt
     */
    std::future<GetPromptResult> getRefactoringPrompt(const std::string& code, 
                                                     const std::string& language = "cpp");
    
    /**
     * @brief Get performance optimization prompt
     */
    std::future<GetPromptResult> getPerformanceOptimizationPrompt(const std::string& code, 
                                                                 const std::string& language = "cpp");
    
    /**
     * @brief Get security audit prompt
     */
    std::future<GetPromptResult> getSecurityAuditPrompt(const std::string& code, 
                                                       const std::string& language = "cpp");
    
    /**
     * @brief Get API documentation prompt
     */
    std::future<GetPromptResult> getApiDocumentationPrompt(const std::string& code, 
                                                          const std::string& language = "cpp");
    
    /**
     * @brief Get error handling prompt
     */
    std::future<GetPromptResult> getErrorHandlingPrompt(const std::string& code, 
                                                       const std::string& language = "cpp");
    
    /**
     * @brief Get code explanation prompt
     */
    std::future<GetPromptResult> getCodeExplanationPrompt(const std::string& code, 
                                                         const std::string& language = "cpp");
    
    // Utility methods
    
    /**
     * @brief Ping the server
     */
    std::future<bool> ping();
    
    /**
     * @brief Get client information
     */
    const ClientInfo& getClientInfo() const;
    
    /**
     * @brief Get server information
     */
    const std::optional<ServerInfo>& getServerInfo() const;
    
    /**
     * @brief Set notification handler
     */
    void setNotificationHandler(std::function<void(const Notification&)> handler);
    
    /**
     * @brief Set error handler
     */
    void setErrorHandler(std::function<void(const std::exception&)> handler);

private:
    void setupDefaultClientInfo();
};

/**
 * @brief Helper functions for creating client instances
 */
namespace client_helpers {

/**
 * @brief Create a basic stdio client
 */
std::unique_ptr<BasicClient> createBasicStdioClient();

/**
 * @brief Create a basic HTTP client
 */
std::unique_ptr<BasicClient> createBasicHttpClient(const std::string& baseUrl, 
                                                  const std::string& authToken = "");

/**
 * @brief Create a basic WebSocket client
 */
std::unique_ptr<BasicClient> createBasicWebSocketClient(const std::string& wsUrl,
                                                       const std::string& authToken = "");

/**
 * @brief Create a basic SSE client
 */
std::unique_ptr<BasicClient> createBasicSseClient(const std::string& sseUrl,
                                                 const std::string& authToken = "");

/**
 * @brief Create a client with custom info
 */
std::unique_ptr<BasicClient> createClientWithInfo(std::unique_ptr<Client> client,
                                                 const ClientInfo& clientInfo);

/**
 * @brief Create a client with custom capabilities
 */
std::unique_ptr<BasicClient> createClientWithCapabilities(std::unique_ptr<Client> client,
                                                         const Capabilities& capabilities);

} // namespace client_helpers

/**
 * @brief Client demo functions
 */
namespace demo {

/**
 * @brief Run a basic client demo
 */
void runBasicDemo(std::unique_ptr<BasicClient> client);

/**
 * @brief Run a tools demo
 */
void runToolsDemo(std::unique_ptr<BasicClient> client);

/**
 * @brief Run a resources demo
 */
void runResourcesDemo(std::unique_ptr<BasicClient> client);

/**
 * @brief Run a prompts demo
 */
void runPromptsDemo(std::unique_ptr<BasicClient> client);

/**
 * @brief Run a comprehensive demo
 */
void runComprehensiveDemo(std::unique_ptr<BasicClient> client);

/**
 * @brief Run a performance demo
 */
void runPerformanceDemo(std::unique_ptr<BasicClient> client);

/**
 * @brief Run an error handling demo
 */
void runErrorHandlingDemo(std::unique_ptr<BasicClient> client);

} // namespace demo

} // namespace examples
} // namespace mcp