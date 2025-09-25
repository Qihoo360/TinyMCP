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

int main(int argc, char* argv[]) {
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Configure logging
    mcp::utils::LoggerConfig::configureDefault();
    MCP_LOG_INFO("Starting MCP Client Example");
    
    try {
        // Create client based on command line arguments
        std::unique_ptr<mcp::examples::BasicClient> client;
        
        if (argc > 1) {
            std::string transport = argv[1];
            if (transport == "http") {
                std::string url = argc > 2 ? argv[2] : "http://localhost:8080";
                std::string token = argc > 3 ? argv[3] : "";
                MCP_LOG_INFO("Creating HTTP client for " + url);
                client = mcp::examples::client_helpers::createBasicHttpClient(url, token);
            } else if (transport == "websocket") {
                std::string url = argc > 2 ? argv[2] : "ws://localhost:8080";
                std::string token = argc > 3 ? argv[3] : "";
                MCP_LOG_INFO("Creating WebSocket client for " + url);
                client = mcp::examples::client_helpers::createBasicWebSocketClient(url, token);
            } else if (transport == "sse") {
                std::string url = argc > 2 ? argv[2] : "http://localhost:8080/sse";
                std::string token = argc > 3 ? argv[3] : "";
                MCP_LOG_INFO("Creating SSE client for " + url);
                client = mcp::examples::client_helpers::createBasicSseClient(url, token);
            } else {
                std::cerr << "Unknown transport: " << transport << std::endl;
                std::cerr << "Usage: " << argv[0] << " [stdio|http|websocket|sse] [url] [token]" << std::endl;
                return 1;
            }
        } else {
            MCP_LOG_INFO("Creating stdio client");
            client = mcp::examples::client_helpers::createBasicStdioClient();
        }
        
        // Set up event handlers
        client->setNotificationHandler([](const mcp::Notification& notification) {
            MCP_LOG_INFO("Received notification: " + notification.getMethod());
            std::cout << "📢 Notification: " << notification.getMethod() << std::endl;
        });
        
        client->setErrorHandler([](const std::exception& error) {
            MCP_LOG_ERROR("Client error: " + std::string(error.what()));
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
        std::cout << "🚀 MCP Client Example" << std::endl;
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
        
        // Test basic tools
        printSeparator();
        std::cout << "🧪 Testing Basic Tools:" << std::endl;
        
        // Echo test
        std::cout << "Testing echo tool..." << std::endl;
        auto echoFuture = client->echo("Hello, MCP World!");
        auto echoResult = echoFuture.get();
        std::cout << "Echo result: " << echoResult << std::endl;
        
        // Calculator test
        std::cout << "Testing calculator tool..." << std::endl;
        auto calcFuture = client->calculate("2 + 2 * 3");
        auto calcResult = calcFuture.get();
        std::cout << "Calculator result: " << calcResult << std::endl;
        
        // System info test
        std::cout << "Testing system info tool..." << std::endl;
        auto sysInfoFuture = client->getSystemInfo();
        auto sysInfoResult = sysInfoFuture.get();
        std::cout << "System info: " << sysInfoResult << std::endl;
        
        // Date/time test
        std::cout << "Testing date/time tool..." << std::endl;
        auto dateTimeFuture = client->getCurrentDateTime();
        auto dateTimeResult = dateTimeFuture.get();
        std::cout << "Current date/time: " << dateTimeResult << std::endl;
        
        // Random number test
        std::cout << "Testing random number tool..." << std::endl;
        auto randomFuture = client->generateRandomNumber(1, 100);
        auto randomResult = randomFuture.get();
        std::cout << "Random number (1-100): " << randomResult << std::endl;
        
        // Hash test
        std::cout << "Testing hash tool..." << std::endl;
        auto hashFuture = client->generateHash("Hello, MCP!", "sha256");
        auto hashResult = hashFuture.get();
        std::cout << "SHA256 hash: " << hashResult << std::endl;
        
        // Base64 test
        std::cout << "Testing base64 encoding..." << std::endl;
        auto base64Future = client->base64Encode("Hello, MCP!");
        auto base64Result = base64Future.get();
        std::cout << "Base64 encoded: " << base64Result << std::endl;
        
        // Test advanced tools if available
        printSeparator();
        std::cout << "🔬 Testing Advanced Tools:" << std::endl;
        
        // Advanced math test
        std::cout << "Testing advanced math tool..." << std::endl;
        auto mathFuture = client->callTool("advanced_math", 
            std::unordered_map<std::string, mcp::JsonValue>{
                {"operation", mcp::utils::JsonUtils::fromString("sin")},
                {"value", mcp::utils::JsonUtils::fromNumber(3.14159 / 2)}
            });
        auto mathResult = mathFuture.get();
        if (!mathResult.content.empty()) {
            std::cout << "sin(π/2) = " << mathResult.content[0].text.value_or("") << std::endl;
        }
        
        // File operations test
        std::cout << "Testing file operations tool..." << std::endl;
        auto fileOpsFuture = client->callTool("file_operations",
            std::unordered_map<std::string, mcp::JsonValue>{
                {"action", mcp::utils::JsonUtils::fromString("exists")},
                {"path", mcp::utils::JsonUtils::fromString("/etc/passwd")}
            });
        auto fileOpsResult = fileOpsFuture.get();
        if (!fileOpsResult.content.empty()) {
            std::cout << "File /etc/passwd exists: " << fileOpsResult.content[0].text.value_or("") << std::endl;
        }
        
        // List available resources
        printSeparator();
        std::cout << "📚 Available Resources:" << std::endl;
        auto resourcesFuture = client->listResources();
        auto resources = resourcesFuture.get();
        for (const auto& resource : resources.resources) {
            std::cout << "  • " << resource.uri << ": " << resource.description << std::endl;
        }
        
        // Test resources
        printSeparator();
        std::cout << "🔍 Testing Resources:" << std::endl;
        
        // System info resource test
        std::cout << "Testing system info resource..." << std::endl;
        auto sysResFuture = client->readResource("system://info");
        auto sysResResult = sysResFuture.get();
        if (!sysResResult.contents.empty()) {
            std::cout << "System info resource: " << sysResResult.contents[0].text.value_or("") << std::endl;
        }
        
        // List available prompts
        printSeparator();
        std::cout << "💬 Available Prompts:" << std::endl;
        auto promptsFuture = client->listPrompts();
        auto prompts = promptsFuture.get();
        for (const auto& prompt : prompts.prompts) {
            std::cout << "  • " << prompt.name << ": " << prompt.description << std::endl;
        }
        
        // Test prompts
        printSeparator();
        std::cout << "🎯 Testing Prompts:" << std::endl;
        
        // Code review prompt test
        std::cout << "Testing code review prompt..." << std::endl;
        auto codeReviewFuture = client->getCodeReviewPrompt(
            "int main() { return 0; }", "cpp");
        auto codeReviewResult = codeReviewFuture.get();
        if (!codeReviewResult.messages.empty()) {
            std::cout << "Code review prompt: " << codeReviewResult.messages[0].text.value_or("") << std::endl;
        }
        
        // Code quality prompt test
        std::cout << "Testing code quality prompt..." << std::endl;
        auto codeQualityFuture = client->getPrompt("code_quality",
            std::unordered_map<std::string, mcp::JsonValue>{
                {"code", mcp::utils::JsonUtils::fromString("int main() { return 0; }")},
                {"language", mcp::utils::JsonUtils::fromString("cpp")},
                {"focus", mcp::utils::JsonUtils::fromString("style")}
            });
        auto codeQualityResult = codeQualityFuture.get();
        if (!codeQualityResult.messages.empty()) {
            std::cout << "Code quality prompt: " << codeQualityResult.messages[0].text.value_or("") << std::endl;
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
                std::cout << "  echo <message>     - Echo a message" << std::endl;
                std::cout << "  calc <expression>  - Calculate expression" << std::endl;
                std::cout << "  random [min] [max] - Generate random number" << std::endl;
                std::cout << "  hash <text> [alg]  - Generate hash" << std::endl;
                std::cout << "  base64 <text>      - Base64 encode" << std::endl;
                std::cout << "  ping               - Ping server" << std::endl;
                std::cout << "  tools              - List tools" << std::endl;
                std::cout << "  resources          - List resources" << std::endl;
                std::cout << "  prompts            - List prompts" << std::endl;
                std::cout << "  help               - Show this help" << std::endl;
                std::cout << "  quit/exit          - Exit program" << std::endl;
            } else if (input.substr(0, 4) == "echo") {
                std::string message = input.substr(5);
                auto future = client->echo(message);
                auto result = future.get();
                std::cout << "Echo: " << result << std::endl;
            } else if (input.substr(0, 4) == "calc") {
                std::string expression = input.substr(5);
                auto future = client->calculate(expression);
                auto result = future.get();
                std::cout << "Result: " << result << std::endl;
            } else if (input.substr(0, 6) == "random") {
                std::istringstream iss(input.substr(7));
                int min = 0, max = 100;
                iss >> min >> max;
                auto future = client->generateRandomNumber(min, max);
                auto result = future.get();
                std::cout << "Random (" << min << "-" << max << "): " << result << std::endl;
            } else if (input.substr(0, 4) == "hash") {
                std::istringstream iss(input.substr(5));
                std::string text, alg = "sha256";
                iss >> text >> alg;
                auto future = client->generateHash(text, alg);
                auto result = future.get();
                std::cout << alg << " hash: " << result << std::endl;
            } else if (input.substr(0, 6) == "base64") {
                std::string text = input.substr(7);
                auto future = client->base64Encode(text);
                auto result = future.get();
                std::cout << "Base64: " << result << std::endl;
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
            } else if (input == "resources") {
                auto future = client->listResources();
                auto result = future.get();
                std::cout << "Available resources:" << std::endl;
                for (const auto& resource : result.resources) {
                    std::cout << "  • " << resource.uri << ": " << resource.description << std::endl;
                }
            } else if (input == "prompts") {
                auto future = client->listPrompts();
                auto result = future.get();
                std::cout << "Available prompts:" << std::endl;
                for (const auto& prompt : result.prompts) {
                    std::cout << "  • " << prompt.name << ": " << prompt.description << std::endl;
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