#include "mcp/examples/basic_client.hpp"
#include "mcp/transport.hpp"
#include "mcp/client.hpp"
#include "mcp/utils/logger.hpp"
#include <iostream>
#include <memory>
#include <future>
#include <thread>

namespace mcp {
namespace examples {

std::unique_ptr<MCPClient> createBasicClient() {
    // Create stdio transport
    auto transport = std::make_unique<StdioTransport>();
    
    // Create client
    auto client = std::make_unique<MCPClient>(std::move(transport));
    
    // Set up notification handler
    client->onNotification([](const std::string& method, const nlohmann::json& params) {
        utils::getLogger().info("Received notification: " + method);
    });
    
    // Set up response handler
    client->onResponse([](const RequestId& id, const Response& response) {
        if (response.isError()) {
            utils::getLogger().error("Request failed: " + response.getError()->what());
        } else {
            utils::getLogger().info("Request completed successfully");
        }
    });
    
    return client;
}

void runBasicClient() {
    utils::getLogger().info("Starting Basic MCP Client...");
    
    auto client = createBasicClient();
    
    if (!client->connect()) {
        utils::getLogger().error("Failed to connect client");
        return;
    }
    
    utils::getLogger().info("Client connected successfully");
    
    try {
        // Initialize the client
        ClientInfo clientInfo;
        clientInfo.name = "Basic MCP Client";
        clientInfo.version = "1.0.0";
        
        auto initFuture = client->initialize(clientInfo);
        auto initResponse = initFuture.get();
        
        if (initResponse.isError()) {
            utils::getLogger().error("Initialization failed: " + initResponse.getError()->what());
            return;
        }
        
        utils::getLogger().info("Client initialized successfully");
        client->setInitialized(true);
        
        // List available tools
        auto toolsFuture = client->listTools();
        auto toolsResponse = toolsFuture.get();
        
        if (!toolsResponse.isError()) {
            utils::getLogger().info("Available tools:");
            auto result = toolsResponse.getResult();
            if (result.contains("tools")) {
                for (const auto& tool : result["tools"]) {
                    utils::getLogger().info("  - " + tool["name"].get<std::string>() + ": " + tool["description"].get<std::string>());
                }
            }
        }
        
        // List available resources
        auto resourcesFuture = client->listResources();
        auto resourcesResponse = resourcesFuture.get();
        
        if (!resourcesResponse.isError()) {
            utils::getLogger().info("Available resources:");
            auto result = resourcesResponse.getResult();
            if (result.contains("resources")) {
                for (const auto& resource : result["resources"]) {
                    utils::getLogger().info("  - " + resource["uri"].get<std::string>() + ": " + resource["description"].get<std::string>());
                }
            }
        }
        
        // List available prompts
        auto promptsFuture = client->listPrompts();
        auto promptsResponse = promptsFuture.get();
        
        if (!promptsResponse.isError()) {
            utils::getLogger().info("Available prompts:");
            auto result = promptsResponse.getResult();
            if (result.contains("prompts")) {
                for (const auto& prompt : result["prompts"]) {
                    utils::getLogger().info("  - " + prompt["name"].get<std::string>() + ": " + prompt["description"].get<std::string>());
                }
            }
        }
        
        // Example: Call echo tool
        nlohmann::json echoArgs;
        echoArgs["message"] = "Hello from MCP Client!";
        
        auto echoFuture = client->callTool("echo", echoArgs);
        auto echoResponse = echoFuture.get();
        
        if (!echoResponse.isError()) {
            auto result = echoResponse.getResult();
            utils::getLogger().info("Echo result: " + result.dump());
        }
        
        // Example: Call calculator tool
        nlohmann::json calcArgs;
        calcArgs["operation"] = "add";
        calcArgs["a"] = 10;
        calcArgs["b"] = 5;
        
        auto calcFuture = client->callTool("calculator", calcArgs);
        auto calcResponse = calcFuture.get();
        
        if (!calcResponse.isError()) {
            auto result = calcResponse.getResult();
            utils::getLogger().info("Calculator result: " + result.dump());
        }
        
        // Example: Read a resource
        auto resourceFuture = client->readResource("memory://data");
        auto resourceResponse = resourceFuture.get();
        
        if (!resourceResponse.isError()) {
            auto result = resourceResponse.getResult();
            utils::getLogger().info("Resource content: " + result.dump());
        }
        
        // Example: Get a prompt
        nlohmann::json promptArgs;
        promptArgs["language"] = "C++";
        promptArgs["description"] = "A function to calculate factorial";
        
        auto promptFuture = client->getPrompt("code_generation", promptArgs);
        auto promptResponse = promptFuture.get();
        
        if (!promptResponse.isError()) {
            auto result = promptResponse.getResult();
            utils::getLogger().info("Generated prompt: " + result.dump());
        }
        
        utils::getLogger().info("Client operations completed successfully");
        
    } catch (const std::exception& e) {
        utils::getLogger().error("Client error: " + std::string(e.what()));
    }
    
    client->disconnect();
    utils::getLogger().info("Client disconnected");
}

} // namespace examples
} // namespace mcp