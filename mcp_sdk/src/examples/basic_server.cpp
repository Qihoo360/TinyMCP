#include "mcp/examples/basic_server.hpp"
#include "mcp/transport.hpp"
#include "mcp/server.hpp"
#include "mcp/tools.hpp"
#include "mcp/resources.hpp"
#include "mcp/prompts.hpp"
#include "mcp/utils/logger.hpp"
#include <iostream>
#include <memory>

namespace mcp {
namespace examples {

std::unique_ptr<MCPServer> createBasicServer() {
    // Create stdio transport
    auto transport = std::make_unique<StdioTransport>();
    
    // Create server
    auto server = std::make_unique<MCPServer>(std::move(transport));
    
    // Set server info
    server->setServerInfo("Basic MCP Server", "1.0.0");
    
    // Register echo tool
    EchoTool echoTool;
    server->registerTool("echo", echoTool.getInfo(), [&echoTool](const nlohmann::json& args) {
        return echoTool.execute(args);
    });
    
    // Register calculator tool
    CalculatorTool calculatorTool;
    server->registerTool("calculator", calculatorTool.getInfo(), [&calculatorTool](const nlohmann::json& args) {
        return calculatorTool.execute(args);
    });
    
    // Register file read tool
    FileReadTool fileReadTool;
    server->registerTool("file_read", fileReadTool.getInfo(), [&fileReadTool](const nlohmann::json& args) {
        return fileReadTool.execute(args);
    });
    
    // Register memory resource
    auto memoryResource = std::make_shared<MemoryResource>(
        "memory://data", "Sample Data", "In-memory data resource", "application/json"
    );
    memoryResource->setContent("{\"message\": \"Hello from memory resource!\", \"timestamp\": \"2024-01-01T00:00:00Z\"}");
    
    server->registerResource(memoryResource->getInfo().uri, memoryResource->getInfo(), [memoryResource]() {
        return memoryResource->read();
    });
    
    // Register HTTP resource
    auto httpResource = std::make_shared<HttpResource>(
        "https://api.example.com/data", "API Data", "External API resource"
    );
    
    server->registerResource(httpResource->getInfo().uri, httpResource->getInfo(), [httpResource]() {
        return httpResource->read();
    });
    
    // Register code generation prompt
    CodeGenerationPrompt codePrompt;
    server->registerPrompt("code_generation", codePrompt.getInfo(), [&codePrompt](const nlohmann::json& args) {
        return codePrompt.generate(args);
    });
    
    // Register documentation prompt
    DocumentationPrompt docPrompt;
    server->registerPrompt("documentation", docPrompt.getInfo(), [&docPrompt](const nlohmann::json& args) {
        return docPrompt.generate(args);
    });
    
    // Register debugging prompt
    DebuggingPrompt debugPrompt;
    server->registerPrompt("debugging", debugPrompt.getInfo(), [&debugPrompt](const nlohmann::json& args) {
        return debugPrompt.generate(args);
    });
    
    return server;
}

void runBasicServer() {
    utils::getLogger().info("Starting Basic MCP Server...");
    
    auto server = createBasicServer();
    
    if (!server->start()) {
        utils::getLogger().error("Failed to start server");
        return;
    }
    
    utils::getLogger().info("Server started successfully");
    utils::getLogger().info("Available tools: echo, calculator, file_read");
    utils::getLogger().info("Available resources: memory://data, https://api.example.com/data");
    utils::getLogger().info("Available prompts: code_generation, documentation, debugging");
    
    // Keep the server running
    std::cout << "Server is running. Press Ctrl+C to stop." << std::endl;
    
    try {
        while (server->isRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } catch (const std::exception& e) {
        utils::getLogger().error("Server error: " + std::string(e.what()));
    }
    
    server->stop();
    utils::getLogger().info("Server stopped");
}

} // namespace examples
} // namespace mcp