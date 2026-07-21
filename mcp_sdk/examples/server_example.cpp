#include <mcp/mcp.hpp>
#include <mcp/examples/basic_server.hpp>
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

int main(int argc, char* argv[]) {
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Configure logging
    mcp::utils::LoggerConfig::configureDefault();
    MCP_LOG_INFO("Starting MCP Server Example");
    
    try {
        // Create server based on command line arguments
        std::unique_ptr<mcp::examples::BasicServer> server;
        
        if (argc > 1) {
            std::string transport = argv[1];
            if (transport == "http") {
                std::string host = argc > 2 ? argv[2] : "localhost";
                int port = argc > 3 ? std::stoi(argv[3]) : 8080;
                MCP_LOG_INFO("Creating HTTP server on " + host + ":" + std::to_string(port));
                server = mcp::examples::server_helpers::createBasicHttpServer(host, port);
            } else if (transport == "websocket") {
                std::string host = argc > 2 ? argv[2] : "localhost";
                int port = argc > 3 ? std::stoi(argv[3]) : 8080;
                MCP_LOG_INFO("Creating WebSocket server on " + host + ":" + std::to_string(port));
                server = mcp::examples::server_helpers::createBasicWebSocketServer(host, port);
            } else if (transport == "sse") {
                std::string host = argc > 2 ? argv[2] : "localhost";
                int port = argc > 3 ? std::stoi(argv[3]) : 8080;
                MCP_LOG_INFO("Creating SSE server on " + host + ":" + std::to_string(port));
                server = mcp::examples::server_helpers::createBasicSseServer(host, port);
            } else {
                std::cerr << "Unknown transport: " << transport << std::endl;
                std::cerr << "Usage: " << argv[0] << " [stdio|http|websocket|sse] [host] [port]" << std::endl;
                return 1;
            }
        } else {
            MCP_LOG_INFO("Creating stdio server");
            server = mcp::examples::server_helpers::createBasicStdioServer();
        }
        
        // Initialize server
        if (!server->initialize()) {
            MCP_LOG_ERROR("Failed to initialize server");
            return 1;
        }
        
        // Add custom tools
        MCP_LOG_INFO("Adding custom tools...");
        
        // Custom math tool
        mcp::Tool mathTool;
        mathTool.name = "advanced_math";
        mathTool.description = "Advanced mathematical operations";
        mathTool.inputSchema = mcp::utils::JsonUtils::fromObject({
            {"type", mcp::utils::JsonUtils::fromString("object")},
            {"properties", mcp::utils::JsonUtils::fromObject({
                {"operation", mcp::utils::JsonUtils::fromObject({
                    {"type", mcp::utils::JsonUtils::fromString("string")},
                    {"enum", mcp::utils::JsonUtils::fromArray({
                        mcp::utils::JsonUtils::fromString("sin"),
                        mcp::utils::JsonUtils::fromString("cos"),
                        mcp::utils::JsonUtils::fromString("tan"),
                        mcp::utils::JsonUtils::fromString("sqrt"),
                        mcp::utils::JsonUtils::fromString("log"),
                        mcp::utils::JsonUtils::fromString("exp")
                    })},
                    {"description", mcp::utils::JsonUtils::fromString("Mathematical operation to perform")}
                })},
                {"value", mcp::utils::JsonUtils::fromObject({
                    {"type", mcp::utils::JsonUtils::fromString("number")},
                    {"description", mcp::utils::JsonUtils::fromString("Input value for the operation")}
                })}
            })},
            {"required", mcp::utils::JsonUtils::fromArray({
                mcp::utils::JsonUtils::fromString("operation"),
                mcp::utils::JsonUtils::fromString("value")
            })}
        });
        
        server->addTool(mathTool, [](const mcp::CallToolParams& params) {
            mcp::CallToolResult result;
            
            try {
                auto args = params.arguments.value_or(std::unordered_map<std::string, mcp::JsonValue>{});
                auto operation = mcp::utils::JsonUtils::getString(args.at("operation"));
                auto value = mcp::utils::JsonUtils::getNumber(args.at("value"));
                
                double result_value = 0.0;
                
                if (operation == "sin") {
                    result_value = std::sin(value);
                } else if (operation == "cos") {
                    result_value = std::cos(value);
                } else if (operation == "tan") {
                    result_value = std::tan(value);
                } else if (operation == "sqrt") {
                    result_value = std::sqrt(value);
                } else if (operation == "log") {
                    result_value = std::log(value);
                } else if (operation == "exp") {
                    result_value = std::exp(value);
                } else {
                    throw std::invalid_argument("Unknown operation: " + operation);
                }
                
                mcp::Content content;
                content.type = mcp::ContentType::TEXT;
                content.text = std::to_string(result_value);
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
        
        // Custom file operations tool
        mcp::Tool fileOpsTool;
        fileOpsTool.name = "file_operations";
        fileOpsTool.description = "Advanced file operations";
        fileOpsTool.inputSchema = mcp::utils::JsonUtils::fromObject({
            {"type", mcp::utils::JsonUtils::fromString("object")},
            {"properties", mcp::utils::JsonUtils::fromObject({
                {"action", mcp::utils::JsonUtils::fromObject({
                    {"type", mcp::utils::JsonUtils::fromString("string")},
                    {"enum", mcp::utils::JsonUtils::fromArray({
                        mcp::utils::JsonUtils::fromString("list"),
                        mcp::utils::JsonUtils::fromString("info"),
                        mcp::utils::JsonUtils::fromString("exists"),
                        mcp::utils::JsonUtils::fromString("size")
                    })},
                    {"description", mcp::utils::JsonUtils::fromString("File operation to perform")}
                })},
                {"path", mcp::utils::JsonUtils::fromObject({
                    {"type", mcp::utils::JsonUtils::fromString("string")},
                    {"description", mcp::utils::JsonUtils::fromString("File or directory path")}
                })}
            })},
            {"required", mcp::utils::JsonUtils::fromArray({
                mcp::utils::JsonUtils::fromString("action"),
                mcp::utils::JsonUtils::fromString("path")
            })}
        });
        
        server->addTool(fileOpsTool, [](const mcp::CallToolParams& params) {
            mcp::CallToolResult result;
            
            try {
                auto args = params.arguments.value_or(std::unordered_map<std::string, mcp::JsonValue>{});
                auto action = mcp::utils::JsonUtils::getString(args.at("action"));
                auto path = mcp::utils::JsonUtils::getString(args.at("path"));
                
                std::string result_text;
                
                if (action == "exists") {
                    std::ifstream file(path);
                    result_text = file.good() ? "true" : "false";
                } else if (action == "size") {
                    std::ifstream file(path, std::ios::binary | std::ios::ate);
                    if (file.good()) {
                        result_text = std::to_string(file.tellg());
                    } else {
                        result_text = "Error: File not found";
                    }
                } else if (action == "info") {
                    std::ifstream file(path);
                    if (file.good()) {
                        result_text = "File exists and is readable";
                    } else {
                        result_text = "File does not exist or is not readable";
                    }
                } else if (action == "list") {
                    // Simple directory listing simulation
                    result_text = "Directory listing not implemented in this example";
                } else {
                    throw std::invalid_argument("Unknown action: " + action);
                }
                
                mcp::Content content;
                content.type = mcp::ContentType::TEXT;
                content.text = result_text;
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
        
        // Add custom resources
        MCP_LOG_INFO("Adding custom resources...");
        
        // System info resource
        mcp::Resource systemInfoResource;
        systemInfoResource.uri = "system://info";
        systemInfoResource.name = "System Information";
        systemInfoResource.description = "Current system information";
        systemInfoResource.mimeType = "application/json";
        
        server->addResource(systemInfoResource, [](const mcp::ReadResourceParams& params) {
            mcp::ReadResourceResult result;
            
            mcp::Content content;
            content.type = mcp::ContentType::TEXT;
            content.text = R"({
                "platform": "Linux",
                "architecture": "x86_64",
                "hostname": "example-host",
                "uptime": "2 days, 5 hours",
                "memory": {
                    "total": "16GB",
                    "used": "8GB",
                    "free": "8GB"
                },
                "cpu": {
                    "cores": 8,
                    "usage": "45%"
                }
            })";
            content.mimeType = "application/json";
            result.contents.push_back(content);
            
            return result;
        });
        
        // Add custom prompts
        MCP_LOG_INFO("Adding custom prompts...");
        
        // Code quality prompt
        mcp::Prompt codeQualityPrompt;
        codeQualityPrompt.name = "code_quality";
        codeQualityPrompt.description = "Analyze code quality and provide improvements";
        codeQualityPrompt.arguments = {
            {"code", {"C++ code to analyze", true}},
            {"language", {"Programming language", false, mcp::utils::JsonUtils::fromString("cpp")}},
            {"focus", {"Focus area (performance, security, style)", false, mcp::utils::JsonUtils::fromString("all")}}
        };
        
        server->addPrompt(codeQualityPrompt, [](const mcp::GetPromptParams& params) {
            mcp::GetPromptResult result;
            
            auto args = params.arguments.value_or(std::unordered_map<std::string, mcp::JsonValue>{});
            auto code = mcp::utils::JsonUtils::getString(args.at("code"));
            auto language = args.count("language") ? mcp::utils::JsonUtils::getString(args.at("language")) : "cpp";
            auto focus = args.count("focus") ? mcp::utils::JsonUtils::getString(args.at("focus")) : "all";
            
            // Description
            mcp::Content desc;
            desc.type = mcp::ContentType::TEXT;
            desc.text = "Analyze the provided " + language + " code for quality issues and suggest improvements. Focus on: " + focus;
            result.description.push_back(desc);
            
            // Messages
            mcp::Content msg;
            msg.type = mcp::ContentType::TEXT;
            msg.text = "Please analyze the following " + language + " code and provide quality feedback:\n\n```" + 
                      language + "\n" + code + "\n```\n\nFocus on: " + focus;
            result.messages.push_back(msg);
            
            return result;
        });
        
        // Start server
        if (!server->start()) {
            MCP_LOG_ERROR("Failed to start server");
            return 1;
        }
        
        MCP_LOG_INFO("Server started successfully");
        MCP_LOG_INFO("Available tools: echo, calculator, file_reader, file_writer, system_info, date_time, random_number, hash, base64, json_validator, advanced_math, file_operations");
        MCP_LOG_INFO("Available resources: file://, http://, memory://, database://, env://, config://, system://info");
        MCP_LOG_INFO("Available prompts: code_review, bug_fix, documentation, test_generation, refactoring, performance_optimization, security_audit, api_documentation, error_handling, code_explanation, code_quality");
        
        if (argc <= 1) {
            std::cout << "MCP Server running on stdio. Connect a client to interact." << std::endl;
            std::cout << "Press Ctrl+C to stop." << std::endl;
        }
        
        // Keep server running
        while (g_running.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // Stop server
        server->stop();
        MCP_LOG_INFO("Server stopped");
        
    } catch (const std::exception& e) {
        MCP_LOG_ERROR("Server error: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}