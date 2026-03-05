#include "mcp/client.hpp"
#include "mcp/utils/logger.hpp"
#include <stdexcept>
#include <chrono>
#include <thread>

namespace mcp {

MCPClient::MCPClient(std::unique_ptr<ITransport> transport) 
    : transport_(std::move(transport)), initialized_(false), request_id_counter_(0) {
    
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

MCPClient::~MCPClient() {
    if (transport_ && transport_->isConnected()) {
        disconnect();
    }
}

bool MCPClient::connect() {
    if (!transport_) {
        return false;
    }
    
    if (!transport_->connect()) {
        return false;
    }
    
    utils::getLogger().info("MCP Client connected");
    return true;
}

void MCPClient::disconnect() {
    if (transport_) {
        transport_->disconnect();
        utils::getLogger().info("MCP Client disconnected");
    }
    initialized_ = false;
}

bool MCPClient::isConnected() const {
    return transport_ && transport_->isConnected();
}

std::future<Response> MCPClient::initialize(const ClientInfo& clientInfo) {
    if (!isConnected()) {
        throw std::runtime_error("Client not connected");
    }
    
    nlohmann::json params;
    params["protocolVersion"] = "2024-11-05";
    params["capabilities"] = nlohmann::json::object();
    params["clientInfo"] = {
        {"name", clientInfo.name},
        {"version", clientInfo.version}
    };
    
    return sendRequest("initialize", params);
}

std::future<Response> MCPClient::callTool(const std::string& name, const nlohmann::json& arguments) {
    if (!initialized_) {
        throw std::runtime_error("Client not initialized");
    }
    
    nlohmann::json params;
    params["name"] = name;
    params["arguments"] = arguments;
    
    return sendRequest("tools/call", params);
}

std::future<Response> MCPClient::listTools() {
    if (!initialized_) {
        throw std::runtime_error("Client not initialized");
    }
    
    return sendRequest("tools/list", nlohmann::json::object());
}

std::future<Response> MCPClient::readResource(const std::string& uri) {
    if (!initialized_) {
        throw std::runtime_error("Client not initialized");
    }
    
    nlohmann::json params;
    params["uri"] = uri;
    
    return sendRequest("resources/read", params);
}

std::future<Response> MCPClient::listResources() {
    if (!initialized_) {
        throw std::runtime_error("Client not initialized");
    }
    
    return sendRequest("resources/list", nlohmann::json::object());
}

std::future<Response> MCPClient::getPrompt(const std::string& name, const nlohmann::json& arguments) {
    if (!initialized_) {
        throw std::runtime_error("Client not initialized");
    }
    
    nlohmann::json params;
    params["name"] = name;
    params["arguments"] = arguments;
    
    return sendRequest("prompts/get", params);
}

std::future<Response> MCPClient::listPrompts() {
    if (!initialized_) {
        throw std::runtime_error("Client not initialized");
    }
    
    return sendRequest("prompts/list", nlohmann::json::object());
}

void MCPClient::subscribe(const std::string& method, std::function<void(const nlohmann::json&)> callback) {
    subscriptions_[method] = callback;
}

void MCPClient::unsubscribe(const std::string& method) {
    subscriptions_.erase(method);
}

void MCPClient::onNotification(std::function<void(const std::string&, const nlohmann::json&)> callback) {
    notificationCallback_ = callback;
}

void MCPClient::onResponse(std::function<void(const RequestId&, const Response&)> callback) {
    responseCallback_ = callback;
}

std::future<Response> MCPClient::sendRequest(const std::string& method, const nlohmann::json& params) {
    if (!isConnected()) {
        throw std::runtime_error("Client not connected");
    }
    
    RequestId id;
    id.id = ++request_id_counter_;
    
    Request request(id, method, params);
    std::string message = request.serialize();
    
    // Create promise and future
    auto promise = std::make_shared<std::promise<Response>>();
    auto future = promise->get_future();
    
    // Store the promise
    pending_requests_[id] = promise;
    
    // Send the request
    if (!transport_->write(message)) {
        pending_requests_.erase(id);
        promise->set_exception(std::make_exception_ptr(std::runtime_error("Failed to send request")));
        return future;
    }
    
    utils::getLogger().debug("Sent request: " + method);
    return future;
}

void MCPClient::handleMessage(const std::string& message) {
    try {
        // Try to parse as response first
        auto response = Response::deserialize(message);
        if (response && response->getId().has_value()) {
            handleResponse(*response);
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

void MCPClient::handleResponse(const Response& response) {
    if (!response.getId().has_value()) {
        utils::getLogger().warn("Received response without ID");
        return;
    }
    
    RequestId id = response.getId().value();
    auto it = pending_requests_.find(id);
    
    if (it != pending_requests_.end()) {
        auto promise = it->second;
        pending_requests_.erase(it);
        
        promise->set_value(response);
        
        if (responseCallback_) {
            responseCallback_(id, response);
        }
    } else {
        utils::getLogger().warn("Received response for unknown request ID");
    }
}

void MCPClient::handleNotification(const Notification& notification) {
    std::string method = notification.getMethod();
    
    // Check for specific subscriptions
    auto sub_it = subscriptions_.find(method);
    if (sub_it != subscriptions_.end()) {
        sub_it->second(notification.getParams());
    }
    
    // Call general notification callback
    if (notificationCallback_) {
        notificationCallback_(method, notification.getParams());
    }
    
    utils::getLogger().debug("Received notification: " + method);
}

void MCPClient::setInitialized(bool initialized) {
    initialized_ = initialized;
}

bool MCPClient::isInitialized() const {
    return initialized_;
}

} // namespace mcp