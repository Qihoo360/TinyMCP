#include <mcp/mcp.hpp>
#include <iostream>
#include <signal.h>
#include <atomic>

std::atomic<bool> g_running{true};

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nReceived signal " << signal << ". Shutting down..." << std::endl;
        g_running = false;
    }
}

int main() {
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Configure logging
    mcp::utils::LoggerConfig::configureMinimal();
    MCP_LOG_INFO("Starting MCP Echo Server");
    
    try {
        // Create server info
        mcp::ServerInfo serverInfo;
        serverInfo.name = "echo-server";
        serverInfo.version = "1.0.0";
        serverInfo.description = "Simple MCP echo server";
        
        // Create server with stdio transport
        auto server = mcp::server::createStdioServer(serverInfo);
        
        // Set up capabilities
        mcp::Capabilities capabilities;
        capabilities.tools = true;
        capabilities.logging = true;
        
        // Initialize server
        if (!server->initialize()) {
            MCP_LOG_ERROR("Failed to initialize server");
            return 1;
        }
        
        // Register echo tool
        mcp::Tool echoTool;
        echoTool.name = "echo";
        echoTool.description = "Echo back the input message";
        echoTool.inputSchema = mcp::utils::JsonUtils::fromObject({
            {"type", mcp::utils::JsonUtils::fromString("object")},
            {"properties", mcp::utils::JsonUtils::fromObject({
                {"message", mcp::utils::JsonUtils::fromObject({
                    {"type", mcp::utils::JsonUtils::fromString("string")},
                    {"description", mcp::utils::JsonUtils::fromString("Message to echo back")}
                })}
            })},
            {"required", mcp::utils::JsonUtils::fromArray({
                mcp::utils::JsonUtils::fromString("message")
            })}
        });
        
        server->registerTool(echoTool, [](const mcp::CallToolParams& params) {
            mcp::CallToolResult result;
            
            try {
                auto args = params.arguments.value_or(std::unordered_map<std::string, mcp::JsonValue>{});
                auto message = mcp::utils::JsonUtils::getString(args.at("message"));
                
                mcp::Content content;
                content.type = mcp::ContentType::TEXT;
                content.text = "Echo: " + message;
                result.content.push_back(content);
                
            } catch (const std::exception& e) {
                mcp::Content content;
                content.type = mcp::ContentType::TEXT;
                content.text = "Error: " + std::string(e.what());
                result.content.push_back(content);
                result.isError = true;
            }
            
            return result;
        });
        
        // Register reverse tool
        mcp::Tool reverseTool;
        reverseTool.name = "reverse";
        reverseTool.description = "Reverse the input message";
        reverseTool.inputSchema = mcp::utils::JsonUtils::fromObject({
            {"type", mcp::utils::JsonUtils::fromString("object")},
            {"properties", mcp::utils::JsonUtils::fromObject({
                {"message", mcp::utils::JsonUtils::fromObject({
                    {"type", mcp::utils::JsonUtils::fromString("string")},
                    {"description", mcp::utils::JsonUtils::fromString("Message to reverse")}
                })}
            })},
            {"required", mcp::utils::JsonUtils::fromArray({
                mcp::utils::JsonUtils::fromString("message")
            })}
        });
        
        server->registerTool(reverseTool, [](const mcp::CallToolParams& params) {
            mcp::CallToolResult result;
            
            try {
                auto args = params.arguments.value_or(std::unordered_map<std::string, mcp::JsonValue>{});
                auto message = mcp::utils::JsonUtils::getString(args.at("message"));
                
                std::string reversed(message.rbegin(), message.rend());
                
                mcp::Content content;
                content.type = mcp::ContentType::TEXT;
                content.text = "Reversed: " + reversed;
                result.content.push_back(content);
                
            } catch (const std::exception& e) {
                mcp::Content content;
                content.type = mcp::ContentType::TEXT;
                content.text = "Error: " + std::string(e.what());
                result.content.push_back(content);
                result.isError = true;
            }
            
            return result;
        });
        
        // Register uppercase tool
        mcp::Tool uppercaseTool;
        uppercaseTool.name = "uppercase";
        uppercaseTool.description = "Convert message to uppercase";
        uppercaseTool.inputSchema = mcp::utils::JsonUtils::fromObject({
            {"type", mcp::utils::JsonUtils::fromString("object")},
            {"properties", mcp::utils::JsonUtils::fromObject({
                {"message", mcp::utils::JsonUtils::fromObject({
                    {"type", mcp::utils::JsonUtils::fromString("string")},
                    {"description", mcp::utils::JsonUtils::fromString("Message to convert to uppercase")}
                })}
            })},
            {"required", mcp::utils::JsonUtils::fromArray({
                mcp::utils::JsonUtils::fromString("message")
            })}
        });
        
        server->registerTool(uppercaseTool, [](const mcp::CallToolParams& params) {
            mcp::CallToolResult result;
            
            try {
                auto args = params.arguments.value_or(std::unordered_map<std::string, mcp::JsonValue>{});
                auto message = mcp::utils::JsonUtils::getString(args.at("message"));
                
                std::string uppercase;
                std::transform(message.begin(), message.end(), std::back_inserter(uppercase), ::toupper);
                
                mcp::Content content;
                content.type = mcp::ContentType::TEXT;
                content.text = "Uppercase: " + uppercase;
                result.content.push_back(content);
                
            } catch (const std::exception& e) {
                mcp::Content content;
                content.type = mcp::ContentType::TEXT;
                content.text = "Error: " + std::string(e.what());
                result.content.push_back(content);
                result.isError = true;
            }
            
            return result;
        });
        
        // Register lowercase tool
        mcp::Tool lowercaseTool;
        lowercaseTool.name = "lowercase";
        lowercaseTool.description = "Convert message to lowercase";
        lowercaseTool.inputSchema = mcp::utils::JsonUtils::fromObject({
            {"type", mcp::utils::JsonUtils::fromString("object")},
            {"properties", mcp::utils::JsonUtils::fromObject({
                {"message", mcp::utils::JsonUtils::fromObject({
                    {"type", mcp::utils::JsonUtils::fromString("string")},
                    {"description", mcp::utils::JsonUtils::fromString("Message to convert to lowercase")}
                })}
            })},
            {"required", mcp::utils::JsonUtils::fromArray({
                mcp::utils::JsonUtils::fromString("message")
            })}
        });
        
        server->registerTool(lowercaseTool, [](const mcp::CallToolParams& params) {
            mcp::CallToolResult result;
            
            try {
                auto args = params.arguments.value_or(std::unordered_map<std::string, mcp::JsonValue>{});
                auto message = mcp::utils::JsonUtils::getString(args.at("message"));
                
                std::string lowercase;
                std::transform(message.begin(), message.end(), std::back_inserter(lowercase), ::tolower);
                
                mcp::Content content;
                content.type = mcp::ContentType::TEXT;
                content.text = "Lowercase: " + lowercase;
                result.content.push_back(content);
                
            } catch (const std::exception& e) {
                mcp::Content content;
                content.type = mcp::ContentType::TEXT;
                content.text = "Error: " + std::string(e.what());
                result.content.push_back(content);
                result.isError = true;
            }
            
            return result;
        });
        
        // Register word count tool
        mcp::Tool wordCountTool;
        wordCountTool.name = "word_count";
        wordCountTool.description = "Count words in the message";
        wordCountTool.inputSchema = mcp::utils::JsonUtils::fromObject({
            {"type", mcp::utils::JsonUtils::fromString("object")},
            {"properties", mcp::utils::JsonUtils::fromObject({
                {"message", mcp::utils::JsonUtils::fromObject({
                    {"type", mcp::utils::JsonUtils::fromString("string")},
                    {"description", mcp::utils::JsonUtils::fromString("Message to count words in")}
                })}
            })},
            {"required", mcp::utils::JsonUtils::fromArray({
                mcp::utils::JsonUtils::fromString("message")
            })}
        });
        
        server->registerTool(wordCountTool, [](const mcp::CallToolParams& params) {
            mcp::CallToolResult result;
            
            try {
                auto args = params.arguments.value_or(std::unordered_map<std::string, mcp::JsonValue>{});
                auto message = mcp::utils::JsonUtils::getString(args.at("message"));
                
                std::istringstream iss(message);
                std::string word;
                int count = 0;
                while (iss >> word) {
                    count++;
                }
                
                mcp::Content content;
                content.type = mcp::ContentType::TEXT;
                content.text = "Word count: " + std::to_string(count);
                result.content.push_back(content);
                
            } catch (const std::exception& e) {
                mcp::Content content;
                content.type = mcp::ContentType::TEXT;
                content.text = "Error: " + std::string(e.what());
                result.content.push_back(content);
                result.isError = true;
            }
            
            return result;
        });
        
        // Register character count tool
        mcp::Tool charCountTool;
        charCountTool.name = "char_count";
        charCountTool.description = "Count characters in the message";
        charCountTool.inputSchema = mcp::utils::JsonUtils::fromObject({
            {"type", mcp::utils::JsonUtils::fromString("object")},
            {"properties", mcp::utils::JsonUtils::fromObject({
                {"message", mcp::utils::JsonUtils::fromObject({
                    {"type", mcp::utils::JsonUtils::fromString("string")},
                    {"description", mcp::utils::JsonUtils::fromString("Message to count characters in")}
                })}
            })},
            {"required", mcp::utils::JsonUtils::fromArray({
                mcp::utils::JsonUtils::fromString("message")
            })}
        });
        
        server->registerTool(charCountTool, [](const mcp::CallToolParams& params) {
            mcp::CallToolResult result;
            
            try {
                auto args = params.arguments.value_or(std::unordered_map<std::string, mcp::JsonValue>{});
                auto message = mcp::utils::JsonUtils::getString(args.at("message"));
                
                int count = message.length();
                
                mcp::Content content;
                content.type = mcp::ContentType::TEXT;
                content.text = "Character count: " + std::to_string(count);
                result.content.push_back(content);
                
            } catch (const std::exception& e) {
                mcp::Content content;
                content.type = mcp::ContentType::TEXT;
                content.text = "Error: " + std::string(e.what());
                result.content.push_back(content);
                result.isError = true;
            }
            
            return result;
        });
        
        // Start server
        if (!server->start()) {
            MCP_LOG_ERROR("Failed to start server");
            return 1;
        }
        
        MCP_LOG_INFO("Echo server started successfully");
        std::cout << "🔄 MCP Echo Server running on stdio" << std::endl;
        std::cout << "Available tools:" << std::endl;
        std::cout << "  • echo - Echo back the input message" << std::endl;
        std::cout << "  • reverse - Reverse the input message" << std::endl;
        std::cout << "  • uppercase - Convert message to uppercase" << std::endl;
        std::cout << "  • lowercase - Convert message to lowercase" << std::endl;
        std::cout << "  • word_count - Count words in the message" << std::endl;
        std::cout << "  • char_count - Count characters in the message" << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;
        
        // Keep server running
        while (g_running.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // Stop server
        server->stop();
        MCP_LOG_INFO("Echo server stopped");
        
    } catch (const std::exception& e) {
        MCP_LOG_ERROR("Server error: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}