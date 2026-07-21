#include <mcp/mcp.hpp>
#include <mcp/examples/basic_client.hpp>
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

void printSeparator() {
    std::cout << "\n" << std::string(50, '=') << std::endl;
}

int main() {
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Configure logging
    mcp::utils::LoggerConfig::configureMinimal();
    MCP_LOG_INFO("Starting MCP Echo Client");
    
    try {
        // Create client
        auto client = mcp::examples::client_helpers::createBasicStdioClient();
        
        // Set up event handlers
        client->setNotificationHandler([](const mcp::Notification& notification) {
            std::cout << "📢 Notification: " << notification.getMethod() << std::endl;
        });
        
        client->setErrorHandler([](const std::exception& error) {
            std::cout << "❌ Error: " << error.what() << std::endl;
        });
        
        // Initialize client
        MCP_LOG_INFO("Initializing client...");
        auto initFuture = client->initialize();
        if (!initFuture.get()) {
            MCP_LOG_ERROR("Failed to initialize client");
            return 1;
        }
        
        // Start client
        if (!client->start()) {
            MCP_LOG_ERROR("Failed to start client");
            return 1;
        }
        
        MCP_LOG_INFO("Client started successfully");
        
        printSeparator();
        std::cout << "🔄 MCP Echo Client" << std::endl;
        printSeparator();
        
        // Test ping
        std::cout << "📡 Testing ping..." << std::endl;
        auto pingFuture = client->ping();
        bool pingResult = pingFuture.get();
        std::cout << "Ping result: " << (pingResult ? "✅ Success" : "❌ Failed") << std::endl;
        
        // List available tools
        printSeparator();
        std::cout << "🛠️  Available Tools:" << std::endl;
        auto toolsFuture = client->listTools();
        auto tools = toolsFuture.get();
        for (const auto& tool : tools.tools) {
            std::cout << "  • " << tool.name << ": " << tool.description << std::endl;
        }
        
        // Test echo tools
        printSeparator();
        std::cout << "🧪 Testing Echo Tools:" << std::endl;
        
        // Echo test
        std::cout << "Testing echo tool..." << std::endl;
        auto echoFuture = client->callTool("echo", 
            std::unordered_map<std::string, mcp::JsonValue>{
                {"message", mcp::utils::JsonUtils::fromString("Hello, Echo Server!")}
            });
        auto echoResult = echoFuture.get();
        if (!echoResult.content.empty()) {
            std::cout << "Echo result: " << echoResult.content[0].text.value_or("") << std::endl;
        }
        
        // Reverse test
        std::cout << "Testing reverse tool..." << std::endl;
        auto reverseFuture = client->callTool("reverse",
            std::unordered_map<std::string, mcp::JsonValue>{
                {"message", mcp::utils::JsonUtils::fromString("Hello, World!")}
            });
        auto reverseResult = reverseFuture.get();
        if (!reverseResult.content.empty()) {
            std::cout << "Reverse result: " << reverseResult.content[0].text.value_or("") << std::endl;
        }
        
        // Uppercase test
        std::cout << "Testing uppercase tool..." << std::endl;
        auto uppercaseFuture = client->callTool("uppercase",
            std::unordered_map<std::string, mcp::JsonValue>{
                {"message", mcp::utils::JsonUtils::fromString("hello, world!")}
            });
        auto uppercaseResult = uppercaseFuture.get();
        if (!uppercaseResult.content.empty()) {
            std::cout << "Uppercase result: " << uppercaseResult.content[0].text.value_or("") << std::endl;
        }
        
        // Lowercase test
        std::cout << "Testing lowercase tool..." << std::endl;
        auto lowercaseFuture = client->callTool("lowercase",
            std::unordered_map<std::string, mcp::JsonValue>{
                {"message", mcp::utils::JsonUtils::fromString("HELLO, WORLD!")}
            });
        auto lowercaseResult = lowercaseFuture.get();
        if (!lowercaseResult.content.empty()) {
            std::cout << "Lowercase result: " << lowercaseResult.content[0].text.value_or("") << std::endl;
        }
        
        // Word count test
        std::cout << "Testing word count tool..." << std::endl;
        auto wordCountFuture = client->callTool("word_count",
            std::unordered_map<std::string, mcp::JsonValue>{
                {"message", mcp::utils::JsonUtils::fromString("This is a test message with multiple words")}
            });
        auto wordCountResult = wordCountFuture.get();
        if (!wordCountResult.content.empty()) {
            std::cout << "Word count result: " << wordCountResult.content[0].text.value_or("") << std::endl;
        }
        
        // Character count test
        std::cout << "Testing character count tool..." << std::endl;
        auto charCountFuture = client->callTool("char_count",
            std::unordered_map<std::string, mcp::JsonValue>{
                {"message", mcp::utils::JsonUtils::fromString("Hello, World!")}
            });
        auto charCountResult = charCountFuture.get();
        if (!charCountResult.content.empty()) {
            std::cout << "Character count result: " << charCountResult.content[0].text.value_or("") << std::endl;
        }
        
        printSeparator();
        std::cout << "✅ All tests completed successfully!" << std::endl;
        printSeparator();
        
        // Interactive mode
        std::cout << "🎮 Interactive Mode (type 'help' for commands, 'quit' to exit):" << std::endl;
        
        std::string input;
        while (g_running.load() && std::getline(std::cin, input)) {
            if (input.empty()) continue;
            
            if (input == "quit" || input == "exit") {
                break;
            } else if (input == "help") {
                std::cout << "\nAvailable commands:" << std::endl;
                std::cout << "  echo <message>      - Echo a message" << std::endl;
                std::cout << "  reverse <message>   - Reverse a message" << std::endl;
                std::cout << "  upper <message>     - Convert to uppercase" << std::endl;
                std::cout << "  lower <message>     - Convert to lowercase" << std::endl;
                std::cout << "  words <message>     - Count words" << std::endl;
                std::cout << "  chars <message>     - Count characters" << std::endl;
                std::cout << "  ping                - Ping server" << std::endl;
                std::cout << "  tools               - List tools" << std::endl;
                std::cout << "  help                - Show this help" << std::endl;
                std::cout << "  quit/exit           - Exit program" << std::endl;
            } else if (input.substr(0, 4) == "echo") {
                std::string message = input.substr(5);
                auto future = client->callTool("echo",
                    std::unordered_map<std::string, mcp::JsonValue>{
                        {"message", mcp::utils::JsonUtils::fromString(message)}
                    });
                auto result = future.get();
                if (!result.content.empty()) {
                    std::cout << result.content[0].text.value_or("") << std::endl;
                }
            } else if (input.substr(0, 7) == "reverse") {
                std::string message = input.substr(8);
                auto future = client->callTool("reverse",
                    std::unordered_map<std::string, mcp::JsonValue>{
                        {"message", mcp::utils::JsonUtils::fromString(message)}
                    });
                auto result = future.get();
                if (!result.content.empty()) {
                    std::cout << result.content[0].text.value_or("") << std::endl;
                }
            } else if (input.substr(0, 5) == "upper") {
                std::string message = input.substr(6);
                auto future = client->callTool("uppercase",
                    std::unordered_map<std::string, mcp::JsonValue>{
                        {"message", mcp::utils::JsonUtils::fromString(message)}
                    });
                auto result = future.get();
                if (!result.content.empty()) {
                    std::cout << result.content[0].text.value_or("") << std::endl;
                }
            } else if (input.substr(0, 5) == "lower") {
                std::string message = input.substr(6);
                auto future = client->callTool("lowercase",
                    std::unordered_map<std::string, mcp::JsonValue>{
                        {"message", mcp::utils::JsonUtils::fromString(message)}
                    });
                auto result = future.get();
                if (!result.content.empty()) {
                    std::cout << result.content[0].text.value_or("") << std::endl;
                }
            } else if (input.substr(0, 5) == "words") {
                std::string message = input.substr(6);
                auto future = client->callTool("word_count",
                    std::unordered_map<std::string, mcp::JsonValue>{
                        {"message", mcp::utils::JsonUtils::fromString(message)}
                    });
                auto result = future.get();
                if (!result.content.empty()) {
                    std::cout << result.content[0].text.value_or("") << std::endl;
                }
            } else if (input.substr(0, 5) == "chars") {
                std::string message = input.substr(6);
                auto future = client->callTool("char_count",
                    std::unordered_map<std::string, mcp::JsonValue>{
                        {"message", mcp::utils::JsonUtils::fromString(message)}
                    });
                auto result = future.get();
                if (!result.content.empty()) {
                    std::cout << result.content[0].text.value_or("") << std::endl;
                }
            } else if (input == "ping") {
                auto future = client->ping();
                auto result = future.get();
                std::cout << "Ping: " << (result ? "✅ Success" : "❌ Failed") << std::endl;
            } else if (input == "tools") {
                auto future = client->listTools();
                auto result = future.get();
                std::cout << "Available tools:" << std::endl;
                for (const auto& tool : result.tools) {
                    std::cout << "  • " << tool.name << ": " << tool.description << std::endl;
                }
            } else {
                std::cout << "Unknown command: " << input << std::endl;
                std::cout << "Type 'help' for available commands." << std::endl;
            }
            
            std::cout << "> ";
        }
        
        // Stop client
        client->stop();
        MCP_LOG_INFO("Client stopped");
        
    } catch (const std::exception& e) {
        MCP_LOG_ERROR("Client error: " + std::string(e.what()));
        std::cout << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}