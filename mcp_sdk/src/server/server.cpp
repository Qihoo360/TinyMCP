#include "mcp/server.hpp"
#include "mcp/utils/logger.hpp"
#include <stdexcept>
#include <algorithm>

namespace mcp {

MCPServer::MCPServer(std::unique_ptr<ITransport> transport) 
    : transport_(std::move(transport)), running_(false), initialized_(false) {
    
    if (!transport_) {
        throw std::invalid_argument("Transport cannot be null");
    }
    
    // Set up transport callbacks
    transport_->setMessageCallback([this](const std::string& message) {
        handleMessage(message);
    });
    
    transport_->setErrorCallback([this](const std::string& error) {
        utils::getLogger().error("Transport error: " + error);
    });
}

MCPServer::~MCPServer() {
    if (running_) {
        stop();
    }
}

bool MCPServer::start() {
    if (running_) {
        return true;
    }
    
    if (!transport_->connect()) {
        return false;
    }
    
    running_ = true;
    utils::getLogger().info("MCP Server started");
    return true;
}

void MCPServer::stop() {
    running_ = false;
    
    if (transport_) {
        transport_->disconnect();
    }
    
    utils::getLogger().info("MCP Server stopped");
}

bool MCPServer::isRunning() const {
    return running_ && transport_ && transport_->isConnected();
}

void MCPServer::registerTool(const std::string& name, const ToolInfo& info, std::function<nlohmann::json(const nlohmann::json&)> handler) {
    tool_registry_.registerTool(name, info, handler);
}

void MCPServer::registerResource(const std::string& uri, const ResourceInfo& info, std::function<nlohmann::json()> handler) {
    resource_registry_.registerResource(uri, info, handler);
}

void MCPServer::registerPrompt(const std::string& name, const PromptInfo& info, std::function<nlohmann::json(const nlohmann::json&)> handler) {
    prompt_registry_.registerPrompt(name, info, handler);
}

void MCPServer::sendNotification(const std::string& method, const nlohmann::json& params) {
    if (!isRunning()) {
        throw std::runtime_error("Server not running");
    }
    
    Notification notification(method, params);
    std::string message = notification.serialize();
    
    if (!transport_->write(message)) {
        throw std::runtime_error("Failed to send notification");
    }
    
    utils::getLogger().debug("Sent notification: " + method);
}

void MCPServer::handleMessage(const std::string& message) {
    try {
        // Try to parse as request first
        auto request = Request::deserialize(message);
        if (request && request->getId().has_value()) {
            handleRequest(*request);
            return;
        }
        
        // Try to parse as notification
        auto notification = Notification::deserialize(message);
        if (notification) {
            handleNotification(*notification);
            return;
        }
        
        utils::getLogger().warn("Received unknown message type: " + message);
        
    } catch (const std::exception& e) {
        utils::getLogger().error("Failed to handle message: " + std::string(e.what()));
    }
}

void MCPServer::handleRequest(const Request& request) {
    std::string method = request.getMethod();
    nlohmann::json params = request.getParams();
    RequestId id = request.getId().value();
    
    try {
        nlohmann::json result;
        
        if (method == "initialize") {
            result = handleInitialize(params);
        } else if (method == "tools/call") {
            result = handleToolCall(params);
        } else if (method == "tools/list") {
            result = handleToolsList(params);
        } else if (method == "resources/read") {
            result = handleResourceRead(params);
        } else if (method == "resources/list") {
            result = handleResourcesList(params);
        } else if (method == "prompts/get") {
            result = handlePromptGet(params);
        } else if (method == "prompts/list") {
            result = handlePromptsList(params);
        } else {
            throw MCPError(ErrorCode::MethodNotFound, "Method not found: " + method);
        }
        
        Response response(id, result);
        std::string responseMessage = response.serialize();
        
        if (!transport_->write(responseMessage)) {
            throw std::runtime_error("Failed to send response");
        }
        
        utils::getLogger().debug("Handled request: " + method);
        
    } catch (const MCPError& e) {
        Response response(id, e);
        std::string responseMessage = response.serialize();
        transport_->write(responseMessage);
    } catch (const std::exception& e) {
        MCPError error(ErrorCode::InternalError, "Internal error: " + std::string(e.what()));
        Response response(id, error);
        std::string responseMessage = response.serialize();
        transport_->write(responseMessage);
    }
}

void MCPServer::handleNotification(const Notification& notification) {
    std::string method = notification.getMethod();
    
    if (method == "notifications/initialized") {
        initialized_ = true;
        utils::getLogger().info("Client initialized");
    }
    
    utils::getLogger().debug("Received notification: " + method);
}

nlohmann::json MCPServer::handleInitialize(const nlohmann::json& params) {
    // Parse client info
    if (!params.contains("clientInfo")) {
        throw MCPError(ErrorCode::InvalidParams, "Missing clientInfo");
    }
    
    nlohmann::json clientInfo = params["clientInfo"];
    if (!clientInfo.contains("name") || !clientInfo.contains("version")) {
        throw MCPError(ErrorCode::InvalidParams, "Invalid clientInfo");
    }
    
    // Build server capabilities
    nlohmann::json capabilities;
    
    if (!tool_registry_.getTools().empty()) {
        capabilities["tools"] = nlohmann::json::object();
    }
    
    if (!resource_registry_.getResources().empty()) {
        capabilities["resources"] = nlohmann::json::object();
    }
    
    if (!prompt_registry_.getPrompts().empty()) {
        capabilities["prompts"] = nlohmann::json::object();
    }
    
    // Build server info
    nlohmann::json serverInfo;
    serverInfo["name"] = server_name_;
    serverInfo["version"] = server_version_;
    
    nlohmann::json result;
    result["protocolVersion"] = "2024-11-05";
    result["capabilities"] = capabilities;
    result["serverInfo"] = serverInfo;
    
    return result;
}

nlohmann::json MCPServer::handleToolCall(const nlohmann::json& params) {
    if (!params.contains("name")) {
        throw MCPError(ErrorCode::InvalidParams, "Missing tool name");
    }
    
    std::string name = params["name"].get<std::string>();
    nlohmann::json arguments = params.contains("arguments") ? params["arguments"] : nlohmann::json::object();
    
    return tool_registry_.callTool(name, arguments);
}

nlohmann::json MCPServer::handleToolsList(const nlohmann::json& params) {
    nlohmann::json result;
    result["tools"] = nlohmann::json::array();
    
    for (const auto& [name, info] : tool_registry_.getTools()) {
        nlohmann::json tool;
        tool["name"] = name;
        tool["description"] = info.description;
        tool["inputSchema"] = info.input_schema;
        result["tools"].push_back(tool);
    }
    
    return result;
}

nlohmann::json MCPServer::handleResourceRead(const nlohmann::json& params) {
    if (!params.contains("uri")) {
        throw MCPError(ErrorCode::InvalidParams, "Missing resource URI");
    }
    
    std::string uri = params["uri"].get<std::string>();
    
    return resource_registry_.readResource(uri);
}

nlohmann::json MCPServer::handleResourcesList(const nlohmann::json& params) {
    nlohmann::json result;
    result["resources"] = nlohmann::json::array();
    
    for (const auto& [uri, info] : resource_registry_.getResources()) {
        nlohmann::json resource;
        resource["uri"] = uri;
        resource["name"] = info.name;
        resource["description"] = info.description;
        resource["mimeType"] = info.mime_type;
        result["resources"].push_back(resource);
    }
    
    return result;
}

nlohmann::json MCPServer::handlePromptGet(const nlohmann::json& params) {
    if (!params.contains("name")) {
        throw MCPError(ErrorCode::InvalidParams, "Missing prompt name");
    }
    
    std::string name = params["name"].get<std::string>();
    nlohmann::json arguments = params.contains("arguments") ? params["arguments"] : nlohmann::json::object();
    
    return prompt_registry_.getPrompt(name, arguments);
}

nlohmann::json MCPServer::handlePromptsList(const nlohmann::json& params) {
    nlohmann::json result;
    result["prompts"] = nlohmann::json::array();
    
    for (const auto& [name, info] : prompt_registry_.getPrompts()) {
        nlohmann::json prompt;
        prompt["name"] = name;
        prompt["description"] = info.description;
        prompt["arguments"] = info.arguments;
        result["prompts"].push_back(prompt);
    }
    
    return result;
}

void MCPServer::setServerInfo(const std::string& name, const std::string& version) {
    server_name_ = name;
    server_version_ = version;
}

} // namespace mcp