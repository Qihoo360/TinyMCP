#include "mcp/resources.hpp"
#include "mcp/utils/logger.hpp"
#include <stdexcept>

namespace mcp {

// ResourceRegistry implementation
void ResourceRegistry::registerResource(const std::string& uri, const ResourceInfo& info, std::function<nlohmann::json()> handler) {
    if (uri.empty()) {
        throw std::invalid_argument("Resource URI cannot be empty");
    }
    
    if (resources_.find(uri) != resources_.end()) {
        utils::getLogger().warn("Overwriting existing resource: " + uri);
    }
    
    resources_[uri] = {info, handler};
    utils::getLogger().info("Registered resource: " + uri);
}

void ResourceRegistry::unregisterResource(const std::string& uri) {
    auto it = resources_.find(uri);
    if (it != resources_.end()) {
        resources_.erase(it);
        utils::getLogger().info("Unregistered resource: " + uri);
    }
}

nlohmann::json ResourceRegistry::readResource(const std::string& uri) {
    auto it = resources_.find(uri);
    if (it == resources_.end()) {
        throw MCPError(ErrorCode::ResourceNotFound, "Resource not found: " + uri);
    }
    
    try {
        return it->second.handler();
    } catch (const std::exception& e) {
        throw MCPError(ErrorCode::InternalError, "Resource read error: " + std::string(e.what()));
    }
}

std::map<std::string, ResourceInfo> ResourceRegistry::getResources() const {
    std::map<std::string, ResourceInfo> result;
    for (const auto& [uri, resource] : resources_) {
        result[uri] = resource.info;
    }
    return result;
}

bool ResourceRegistry::hasResource(const std::string& uri) const {
    return resources_.find(uri) != resources_.end();
}

// Example resources implementation
nlohmann::json FileResource::read() {
    nlohmann::json result;
    
    // Note: In a real implementation, you would read the actual file
    // For this example, we'll return mock data
    result["uri"] = uri_;
    result["mimeType"] = "text/plain";
    result["text"] = "Mock file content for " + uri_;
    
    return result;
}

ResourceInfo FileResource::getInfo() const {
    ResourceInfo info;
    info.uri = uri_;
    info.name = name_;
    info.description = description_;
    info.mime_type = "text/plain";
    return info;
}

nlohmann::json HttpResource::read() {
    nlohmann::json result;
    
    // Note: In a real implementation, you would make an HTTP request
    // For this example, we'll return mock data
    result["uri"] = uri_;
    result["mimeType"] = "application/json";
    result["text"] = "{\"mock\": \"HTTP response data\", \"url\": \"" + uri_ + "\"}";
    
    return result;
}

ResourceInfo HttpResource::getInfo() const {
    ResourceInfo info;
    info.uri = uri_;
    info.name = name_;
    info.description = description_;
    info.mime_type = "application/json";
    return info;
}

nlohmann::json DatabaseResource::read() {
    nlohmann::json result;
    
    // Note: In a real implementation, you would query the database
    // For this example, we'll return mock data
    result["uri"] = uri_;
    result["mimeType"] = "application/json";
    
    nlohmann::json data;
    data["table"] = table_name_;
    data["query"] = query_;
    data["results"] = nlohmann::json::array();
    data["results"].push_back({{"id", 1}, {"name", "Mock Record 1"}});
    data["results"].push_back({{"id", 2}, {"name", "Mock Record 2"}});
    
    result["text"] = data.dump();
    
    return result;
}

ResourceInfo DatabaseResource::getInfo() const {
    ResourceInfo info;
    info.uri = uri_;
    info.name = name_;
    info.description = description_;
    info.mime_type = "application/json";
    return info;
}

nlohmann::json MemoryResource::read() {
    nlohmann::json result;
    
    result["uri"] = uri_;
    result["mimeType"] = mime_type_;
    result["text"] = content_;
    
    return result;
}

ResourceInfo MemoryResource::getInfo() const {
    ResourceInfo info;
    info.uri = uri_;
    info.name = name_;
    info.description = description_;
    info.mime_type = mime_type_;
    return info;
}

void MemoryResource::setContent(const std::string& content) {
    content_ = content;
}

std::string MemoryResource::getContent() const {
    return content_;
}

} // namespace mcp