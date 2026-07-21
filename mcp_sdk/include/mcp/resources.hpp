#pragma once

#include "types.hpp"
#include "message.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

namespace mcp {

/**
 * @brief Resource handler interface
 */
class ResourceHandler {
public:
    virtual ~ResourceHandler() = default;
    
    /**
     * @brief Handle resource list request
     */
    virtual ListResourcesResult listResources() = 0;
    
    /**
     * @brief Handle resource read request
     */
    virtual ReadResourceResult readResource(const ReadResourceParams& params) = 0;
};

/**
 * @brief Default resource handler implementation
 */
class DefaultResourceHandler : public ResourceHandler {
private:
    std::unordered_map<std::string, Resource> resources_;
    std::unordered_map<std::string, std::function<ReadResourceResult(const ReadResourceParams&)>> resource_handlers_;
    std::mutex resources_mutex_;

public:
    /**
     * @brief Register a resource
     */
    void registerResource(const Resource& resource, std::function<ReadResourceResult(const ReadResourceParams&)> handler);
    
    /**
     * @brief Unregister a resource
     */
    void unregisterResource(const std::string& uri);
    
    /**
     * @brief Check if resource is registered
     */
    bool hasResource(const std::string& uri) const;
    
    /**
     * @brief Get registered resources
     */
    std::vector<Resource> getResources() const;
    
    // ResourceHandler interface
    ListResourcesResult listResources() override;
    ReadResourceResult readResource(const ReadResourceParams& params) override;
};

/**
 * @brief Resource registry for managing resources
 */
class ResourceRegistry {
private:
    std::unordered_map<std::string, Resource> resources_;
    std::unordered_map<std::string, std::function<ReadResourceResult(const ReadResourceParams&)>> handlers_;
    std::mutex registry_mutex_;

public:
    /**
     * @brief Register a resource
     */
    void registerResource(const Resource& resource, std::function<ReadResourceResult(const ReadResourceParams&)> handler);
    
    /**
     * @brief Unregister a resource
     */
    void unregisterResource(const std::string& uri);
    
    /**
     * @brief Check if resource is registered
     */
    bool hasResource(const std::string& uri) const;
    
    /**
     * @brief Get resource by URI
     */
    std::optional<Resource> getResource(const std::string& uri) const;
    
    /**
     * @brief Get all registered resources
     */
    std::vector<Resource> getAllResources() const;
    
    /**
     * @brief Get resource handler
     */
    std::optional<std::function<ReadResourceResult(const ReadResourceParams&)>> getHandler(const std::string& uri) const;
    
    /**
     * @brief Clear all resources
     */
    void clear();
    
    /**
     * @brief Get resource count
     */
    size_t size() const;
};

/**
 * @brief Built-in resource handlers
 */
namespace builtin {

/**
 * @brief File resource handler - reads files from filesystem
 */
ReadResourceResult fileResourceHandler(const ReadResourceParams& params);

/**
 * @brief HTTP resource handler - fetches content from URLs
 */
ReadResourceResult httpResourceHandler(const ReadResourceParams& params);

/**
 * @brief Memory resource handler - provides access to in-memory data
 */
ReadResourceResult memoryResourceHandler(const ReadResourceParams& params);

/**
 * @brief Database resource handler - provides access to database content
 */
ReadResourceResult databaseResourceHandler(const ReadResourceParams& params);

/**
 * @brief Environment resource handler - provides access to environment variables
 */
ReadResourceResult environmentResourceHandler(const ReadResourceParams& params);

/**
 * @brief Configuration resource handler - provides access to configuration data
 */
ReadResourceResult configurationResourceHandler(const ReadResourceParams& params);

/**
 * @brief Register all built-in resource handlers
 */
void registerBuiltinResourceHandlers(ResourceRegistry& registry);

} // namespace builtin

/**
 * @brief Resource URI parser
 */
class ResourceUriParser {
public:
    /**
     * @brief Parse resource URI
     */
    static std::unordered_map<std::string, std::string> parseUri(const std::string& uri);
    
    /**
     * @brief Validate resource URI
     */
    static bool validateUri(const std::string& uri);
    
    /**
     * @brief Get URI scheme
     */
    static std::string getScheme(const std::string& uri);
    
    /**
     * @brief Get URI path
     */
    static std::string getPath(const std::string& uri);
    
    /**
     * @brief Get URI query parameters
     */
    static std::unordered_map<std::string, std::string> getQueryParams(const std::string& uri);
    
    /**
     * @brief Get URI fragment
     */
    static std::string getFragment(const std::string& uri);
};

/**
 * @brief Resource content types
 */
namespace content_types {

inline constexpr const char* TEXT_PLAIN = "text/plain";
inline constexpr const char* TEXT_HTML = "text/html";
inline constexpr const char* TEXT_JSON = "application/json";
inline constexpr const char* TEXT_XML = "application/xml";
inline constexpr const char* TEXT_CSV = "text/csv";
inline constexpr const char* TEXT_MARKDOWN = "text/markdown";
inline constexpr const char* IMAGE_PNG = "image/png";
inline constexpr const char* IMAGE_JPEG = "image/jpeg";
inline constexpr const char* IMAGE_GIF = "image/gif";
inline constexpr const char* IMAGE_SVG = "image/svg+xml";
inline constexpr const char* APPLICATION_PDF = "application/pdf";
inline constexpr const char* APPLICATION_ZIP = "application/zip";
inline constexpr const char* APPLICATION_OCTET_STREAM = "application/octet-stream";

} // namespace content_types

/**
 * @brief Resource access control
 */
class ResourceAccessControl {
public:
    /**
     * @brief Check if resource access is allowed
     */
    static bool isAccessAllowed(const std::string& uri, const std::string& user_id = "");
    
    /**
     * @brief Check if resource is public
     */
    static bool isPublicResource(const std::string& uri);
    
    /**
     * @brief Check if resource requires authentication
     */
    static bool requiresAuthentication(const std::string& uri);
    
    /**
     * @brief Get resource permissions
     */
    static std::vector<std::string> getResourcePermissions(const std::string& uri);
};

/**
 * @brief Resource caching
 */
class ResourceCache {
private:
    std::unordered_map<std::string, ReadResourceResult> cache_;
    std::unordered_map<std::string, std::chrono::system_clock::time_point> cache_timestamps_;
    std::chrono::seconds cache_ttl_;
    std::mutex cache_mutex_;

public:
    ResourceCache(std::chrono::seconds ttl = std::chrono::minutes(5));
    
    /**
     * @brief Get cached resource
     */
    std::optional<ReadResourceResult> get(const std::string& uri);
    
    /**
     * @brief Cache resource result
     */
    void put(const std::string& uri, const ReadResourceResult& result);
    
    /**
     * @brief Remove from cache
     */
    void remove(const std::string& uri);
    
    /**
     * @brief Clear cache
     */
    void clear();
    
    /**
     * @brief Check if resource is cached
     */
    bool contains(const std::string& uri) const;
    
    /**
     * @brief Get cache size
     */
    size_t size() const;
    
    /**
     * @brief Set cache TTL
     */
    void setTtl(std::chrono::seconds ttl);
    
    /**
     * @brief Clean expired entries
     */
    void cleanExpired();

private:
    bool isExpired(const std::string& uri) const;
};

/**
 * @brief Resource manager for high-level resource operations
 */
class ResourceManager {
private:
    ResourceRegistry registry_;
    ResourceCache cache_;
    std::unique_ptr<ResourceAccessControl> access_control_;

public:
    ResourceManager(std::chrono::seconds cache_ttl = std::chrono::minutes(5));
    
    /**
     * @brief Register a resource
     */
    void registerResource(const Resource& resource, std::function<ReadResourceResult(const ReadResourceParams&)> handler);
    
    /**
     * @brief Unregister a resource
     */
    void unregisterResource(const std::string& uri);
    
    /**
     * @brief List all resources
     */
    ListResourcesResult listResources();
    
    /**
     * @brief Read a resource
     */
    ReadResourceResult readResource(const ReadResourceParams& params);
    
    /**
     * @brief Check if resource exists
     */
    bool hasResource(const std::string& uri) const;
    
    /**
     * @brief Get resource info
     */
    std::optional<Resource> getResource(const std::string& uri) const;
    
    /**
     * @brief Clear cache
     */
    void clearCache();
    
    /**
     * @brief Set access control
     */
    void setAccessControl(std::unique_ptr<ResourceAccessControl> access_control);
};

} // namespace mcp