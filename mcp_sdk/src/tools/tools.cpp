#include "mcp/tools.hpp"
#include "mcp/utils/logger.hpp"
#include <stdexcept>

namespace mcp {

// ToolRegistry implementation
void ToolRegistry::registerTool(const std::string& name, const ToolInfo& info, std::function<nlohmann::json(const nlohmann::json&)> handler) {
    if (name.empty()) {
        throw std::invalid_argument("Tool name cannot be empty");
    }
    
    if (tools_.find(name) != tools_.end()) {
        utils::getLogger().warn("Overwriting existing tool: " + name);
    }
    
    tools_[name] = {info, handler};
    utils::getLogger().info("Registered tool: " + name);
}

void ToolRegistry::unregisterTool(const std::string& name) {
    auto it = tools_.find(name);
    if (it != tools_.end()) {
        tools_.erase(it);
        utils::getLogger().info("Unregistered tool: " + name);
    }
}

nlohmann::json ToolRegistry::callTool(const std::string& name, const nlohmann::json& arguments) {
    auto it = tools_.find(name);
    if (it == tools_.end()) {
        throw MCPError(ErrorCode::MethodNotFound, "Tool not found: " + name);
    }
    
    try {
        return it->second.handler(arguments);
    } catch (const std::exception& e) {
        throw MCPError(ErrorCode::InternalError, "Tool execution error: " + std::string(e.what()));
    }
}

std::map<std::string, ToolInfo> ToolRegistry::getTools() const {
    std::map<std::string, ToolInfo> result;
    for (const auto& [name, tool] : tools_) {
        result[name] = tool.info;
    }
    return result;
}

bool ToolRegistry::hasTool(const std::string& name) const {
    return tools_.find(name) != tools_.end();
}

// Example tools implementation
nlohmann::json EchoTool::execute(const nlohmann::json& arguments) {
    nlohmann::json result;
    
    if (arguments.contains("message")) {
        result["echo"] = arguments["message"];
    } else {
        result["echo"] = "No message provided";
    }
    
    return result;
}

ToolInfo EchoTool::getInfo() const {
    ToolInfo info;
    info.name = "echo";
    info.description = "Echo back the provided message";
    info.input_schema = {
        {"type", "object"},
        {"properties", {
            {"message", {
                {"type", "string"},
                {"description", "The message to echo back"}
            }}
        }},
        {"required", nlohmann::json::array({"message"})}
    };
    return info;
}

nlohmann::json CalculatorTool::execute(const nlohmann::json& arguments) {
    if (!arguments.contains("operation") || !arguments.contains("a") || !arguments.contains("b")) {
        throw std::invalid_argument("Missing required parameters: operation, a, b");
    }
    
    std::string operation = arguments["operation"].get<std::string>();
    double a = arguments["a"].get<double>();
    double b = arguments["b"].get<double>();
    
    double result = 0.0;
    
    if (operation == "add") {
        result = a + b;
    } else if (operation == "subtract") {
        result = a - b;
    } else if (operation == "multiply") {
        result = a * b;
    } else if (operation == "divide") {
        if (b == 0) {
            throw std::invalid_argument("Division by zero");
        }
        result = a / b;
    } else {
        throw std::invalid_argument("Invalid operation: " + operation);
    }
    
    nlohmann::json response;
    response["result"] = result;
    response["operation"] = operation;
    response["a"] = a;
    response["b"] = b;
    
    return response;
}

ToolInfo CalculatorTool::getInfo() const {
    ToolInfo info;
    info.name = "calculator";
    info.description = "Perform basic arithmetic operations";
    info.input_schema = {
        {"type", "object"},
        {"properties", {
            {"operation", {
                {"type", "string"},
                {"enum", {"add", "subtract", "multiply", "divide"}},
                {"description", "The arithmetic operation to perform"}
            }},
            {"a", {
                {"type", "number"},
                {"description", "First operand"}
            }},
            {"b", {
                {"type", "number"},
                {"description", "Second operand"}
            }}
        }},
        {"required", nlohmann::json::array({"operation", "a", "b"})}
    };
    return info;
}

nlohmann::json FileReadTool::execute(const nlohmann::json& arguments) {
    if (!arguments.contains("path")) {
        throw std::invalid_argument("Missing required parameter: path");
    }
    
    std::string path = arguments["path"].get<std::string>();
    
    // Note: In a real implementation, you would read the file here
    // For this example, we'll just return the path
    nlohmann::json result;
    result["path"] = path;
    result["content"] = "File content would be read here";
    result["size"] = 0; // Would be actual file size
    
    return result;
}

ToolInfo FileReadTool::getInfo() const {
    ToolInfo info;
    info.name = "file_read";
    info.description = "Read contents of a file";
    info.input_schema = {
        {"type", "object"},
        {"properties", {
            {"path", {
                {"type", "string"},
                {"description", "Path to the file to read"}
            }}
        }},
        {"required", nlohmann::json::array({"path"})}
    };
    return info;
}

} // namespace mcp