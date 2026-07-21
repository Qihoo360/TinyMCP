#pragma once

#include "../server.hpp"
#include "../tools.hpp"
#include "../resources.hpp"
#include "../prompts.hpp"
#include <memory>

namespace mcp {
namespace examples {

/**
 * @brief Basic MCP server example
 */
class BasicServer {
private:
    std::unique_ptr<Server> server_;
    std::unique_ptr<ToolRegistry> tool_registry_;
    std::unique_ptr<ResourceRegistry> resource_registry_;
    std::unique_ptr<PromptRegistry> prompt_registry_;

public:
    BasicServer();
    ~BasicServer() = default;
    
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
     * @brief Add a custom tool
     */
    void addTool(const Tool& tool, std::function<CallToolResult(const CallToolParams&)> handler);
    
    /**
     * @brief Add a custom resource
     */
    void addResource(const Resource& resource, std::function<ReadResourceResult(const ReadResourceParams&)> handler);
    
    /**
     * @brief Add a custom prompt
     */
    void addPrompt(const Prompt& prompt, std::function<GetPromptResult(const GetPromptParams&)> handler);

private:
    void setupBuiltinTools();
    void setupBuiltinResources();
    void setupBuiltinPrompts();
    void setupServerCapabilities();
};

/**
 * @brief Echo tool implementation
 */
CallToolResult echoToolHandler(const CallToolParams& params);

/**
 * @brief Calculator tool implementation
 */
CallToolResult calculatorToolHandler(const CallToolParams& params);

/**
 * @brief File reader tool implementation
 */
CallToolResult fileReaderToolHandler(const CallToolParams& params);

/**
 * @brief File writer tool implementation
 */
CallToolResult fileWriterToolHandler(const CallToolParams& params);

/**
 * @brief System info tool implementation
 */
CallToolResult systemInfoToolHandler(const CallToolParams& params);

/**
 * @brief Date/time tool implementation
 */
CallToolResult dateTimeToolHandler(const CallToolParams& params);

/**
 * @brief Random number tool implementation
 */
CallToolResult randomNumberToolHandler(const CallToolParams& params);

/**
 * @brief Hash tool implementation
 */
CallToolResult hashToolHandler(const CallToolParams& params);

/**
 * @brief Base64 tool implementation
 */
CallToolResult base64ToolHandler(const CallToolParams& params);

/**
 * @brief JSON validator tool implementation
 */
CallToolResult jsonValidatorToolHandler(const CallToolParams& params);

/**
 * @brief File resource handler
 */
ReadResourceResult fileResourceHandler(const ReadResourceParams& params);

/**
 * @brief HTTP resource handler
 */
ReadResourceResult httpResourceHandler(const ReadResourceParams& params);

/**
 * @brief Memory resource handler
 */
ReadResourceResult memoryResourceHandler(const ReadResourceParams& params);

/**
 * @brief Environment resource handler
 */
ReadResourceResult environmentResourceHandler(const ReadResourceParams& params);

/**
 * @brief Configuration resource handler
 */
ReadResourceResult configurationResourceHandler(const ReadResourceParams& params);

/**
 * @brief Code review prompt handler
 */
GetPromptResult codeReviewPromptHandler(const GetPromptParams& params);

/**
 * @brief Bug fix prompt handler
 */
GetPromptResult bugFixPromptHandler(const GetPromptParams& params);

/**
 * @brief Documentation prompt handler
 */
GetPromptResult documentationPromptHandler(const GetPromptParams& params);

/**
 * @brief Test generation prompt handler
 */
GetPromptResult testGenerationPromptHandler(const GetPromptParams& params);

/**
 * @brief Refactoring prompt handler
 */
GetPromptResult refactoringPromptHandler(const GetPromptParams& params);

/**
 * @brief Performance optimization prompt handler
 */
GetPromptResult performanceOptimizationPromptHandler(const GetPromptParams& params);

/**
 * @brief Security audit prompt handler
 */
GetPromptResult securityAuditPromptHandler(const GetPromptParams& params);

/**
 * @brief API documentation prompt handler
 */
GetPromptResult apiDocumentationPromptHandler(const GetPromptParams& params);

/**
 * @brief Error handling prompt handler
 */
GetPromptResult errorHandlingPromptHandler(const GetPromptParams& params);

/**
 * @brief Code explanation prompt handler
 */
GetPromptResult codeExplanationPromptHandler(const GetPromptParams& params);

/**
 * @brief Helper functions for creating server instances
 */
namespace server_helpers {

/**
 * @brief Create a basic stdio server
 */
std::unique_ptr<BasicServer> createBasicStdioServer();

/**
 * @brief Create a basic HTTP server
 */
std::unique_ptr<BasicServer> createBasicHttpServer(const std::string& host = "localhost", int port = 8080);

/**
 * @brief Create a basic WebSocket server
 */
std::unique_ptr<BasicServer> createBasicWebSocketServer(const std::string& host = "localhost", int port = 8080);

/**
 * @brief Create a basic SSE server
 */
std::unique_ptr<BasicServer> createBasicSseServer(const std::string& host = "localhost", int port = 8080);

/**
 * @brief Create a minimal server with only echo tool
 */
std::unique_ptr<BasicServer> createMinimalServer();

/**
 * @brief Create a development server with all built-in tools
 */
std::unique_ptr<BasicServer> createDevelopmentServer();

/**
 * @brief Create a production server with selected tools
 */
std::unique_ptr<BasicServer> createProductionServer();

} // namespace server_helpers

} // namespace examples
} // namespace mcp