#include "mcp/capabilities.hpp"

namespace mcp {

ServerCapabilities::ServerCapabilities() {
    tools = nlohmann::json::object();
    resources = nlohmann::json::object();
    prompts = nlohmann::json::object();
}

void ServerCapabilities::setToolsCapability(bool enabled) {
    if (enabled) {
        tools = nlohmann::json::object();
    } else {
        tools = nlohmann::json::object();
        tools = nullptr; // Remove the capability
    }
}

void ServerCapabilities::setResourcesCapability(bool enabled) {
    if (enabled) {
        resources = nlohmann::json::object();
    } else {
        resources = nlohmann::json::object();
        resources = nullptr; // Remove the capability
    }
}

void ServerCapabilities::setPromptsCapability(bool enabled) {
    if (enabled) {
        prompts = nlohmann::json::object();
    } else {
        prompts = nlohmann::json::object();
        prompts = nullptr; // Remove the capability
    }
}

bool ServerCapabilities::hasToolsCapability() const {
    return !tools.is_null();
}

bool ServerCapabilities::hasResourcesCapability() const {
    return !resources.is_null();
}

bool ServerCapabilities::hasPromptsCapability() const {
    return !prompts.is_null();
}

nlohmann::json ServerCapabilities::toJson() const {
    nlohmann::json result;
    
    if (hasToolsCapability()) {
        result["tools"] = tools;
    }
    
    if (hasResourcesCapability()) {
        result["resources"] = resources;
    }
    
    if (hasPromptsCapability()) {
        result["prompts"] = prompts;
    }
    
    return result;
}

ServerCapabilities ServerCapabilities::fromJson(const nlohmann::json& json) {
    ServerCapabilities capabilities;
    
    if (json.contains("tools") && !json["tools"].is_null()) {
        capabilities.tools = json["tools"];
    }
    
    if (json.contains("resources") && !json["resources"].is_null()) {
        capabilities.resources = json["resources"];
    }
    
    if (json.contains("prompts") && !json["prompts"].is_null()) {
        capabilities.prompts = json["prompts"];
    }
    
    return capabilities;
}

} // namespace mcp