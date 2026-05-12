#pragma once

#include "types.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace mcp {

/**
 * @brief MCP Capabilities manager
 */
class CapabilitiesManager {
public:
    /**
     * @brief Check if a capability is supported
     */
    static bool supportsCapability(const Capabilities& capabilities, const std::string& capability);
    
    /**
     * @brief Merge capabilities
     */
    static Capabilities mergeCapabilities(const Capabilities& client, const Capabilities& server);
    
    /**
     * @brief Get common capabilities
     */
    static Capabilities getCommonCapabilities(const Capabilities& client, const Capabilities& server);
    
    /**
     * @brief Validate capabilities
     */
    static bool validateCapabilities(const Capabilities& capabilities);
    
    /**
     * @brief Get capability version
     */
    static std::optional<std::string> getCapabilityVersion(const Capabilities& capabilities, const std::string& capability);
    
    /**
     * @brief Set capability version
     */
    static void setCapabilityVersion(Capabilities& capabilities, const std::string& capability, const std::string& version);
    
    /**
     * @brief Remove capability
     */
    static void removeCapability(Capabilities& capabilities, const std::string& capability);
    
    /**
     * @brief Get all capabilities
     */
    static std::vector<std::string> getAllCapabilities(const Capabilities& capabilities);
    
    /**
     * @brief Check if capabilities are compatible
     */
    static bool areCompatible(const Capabilities& client, const Capabilities& server);
};

/**
 * @brief Standard MCP capabilities
 */
namespace capabilities {

// Core capabilities
inline constexpr const char* LOGGING = "logging";
inline constexpr const char* PROMPTS = "prompts";
inline constexpr const char* RESOURCES = "resources";
inline constexpr const char* TOOLS = "tools";
inline constexpr const char* EXPERIMENTAL = "experimental";

// Extended capabilities
inline constexpr const char* SAMPLES = "samples";
inline constexpr const char* COMPLETIONS = "completions";
inline constexpr const char* EMBEDDINGS = "embeddings";
inline constexpr const char* FILES = "files";
inline constexpr const char* DATABASE = "database";
inline constexpr const char* MEMORY = "memory";
inline constexpr const char* CACHING = "caching";
inline constexpr const char* AUTHENTICATION = "authentication";
inline constexpr const char* AUTHORIZATION = "authorization";
inline constexpr const char* RATE_LIMITING = "rate_limiting";
inline constexpr const char* METRICS = "metrics";
inline constexpr const char* HEALTH_CHECK = "health_check";
inline constexpr const char* CONFIGURATION = "configuration";
inline constexpr const char* DEPLOYMENT = "deployment";

// Version information
inline constexpr const char* VERSION_1_0 = "1.0";
inline constexpr const char* VERSION_1_1 = "1.1";
inline constexpr const char* VERSION_2_0 = "2.0";

} // namespace capabilities

/**
 * @brief Capability feature flags
 */
namespace feature_flags {

// Logging features
inline constexpr const char* LOGGING_LEVELS = "logging_levels";
inline constexpr const char* LOGGING_FILTERS = "logging_filters";
inline constexpr const char* LOGGING_FORMATS = "logging_formats";

// Prompt features
inline constexpr const char* PROMPT_TEMPLATES = "prompt_templates";
inline constexpr const char* PROMPT_VALIDATION = "prompt_validation";
inline constexpr const char* PROMPT_CACHING = "prompt_caching";

// Resource features
inline constexpr const char* RESOURCE_CACHING = "resource_caching";
inline constexpr const char* RESOURCE_STREAMING = "resource_streaming";
inline constexpr const char* RESOURCE_COMPRESSION = "resource_compression";

// Tool features
inline constexpr const char* TOOL_VALIDATION = "tool_validation";
inline constexpr const char* TOOL_TIMEOUTS = "tool_timeouts";
inline constexpr const char* TOOL_CACHING = "tool_caching";
inline constexpr const char* TOOL_STREAMING = "tool_streaming";

// Security features
inline constexpr const char* AUTH_OAUTH = "auth_oauth";
inline constexpr const char* AUTH_API_KEY = "auth_api_key";
inline constexpr const char* AUTH_JWT = "auth_jwt";
inline constexpr const char* AUTH_BASIC = "auth_basic";

// Performance features
inline constexpr const char* RATE_LIMIT_GLOBAL = "rate_limit_global";
inline constexpr const char* RATE_LIMIT_PER_USER = "rate_limit_per_user";
inline constexpr const char* RATE_LIMIT_PER_TOOL = "rate_limit_per_tool";

} // namespace feature_flags

/**
 * @brief Capability requirements
 */
struct CapabilityRequirement {
    std::string name;
    std::string version;
    bool required;
    std::optional<std::vector<std::string>> dependencies;
    std::optional<std::unordered_map<std::string, JsonValue>> options;
};

/**
 * @brief Capability validator
 */
class CapabilityValidator {
public:
    /**
     * @brief Validate capability requirements
     */
    static bool validateRequirements(const std::vector<CapabilityRequirement>& requirements,
                                   const Capabilities& capabilities);
    
    /**
     * @brief Check if requirement is satisfied
     */
    static bool isRequirementSatisfied(const CapabilityRequirement& requirement,
                                      const Capabilities& capabilities);
    
    /**
     * @brief Get missing requirements
     */
    static std::vector<CapabilityRequirement> getMissingRequirements(const std::vector<CapabilityRequirement>& requirements,
                                                                    const Capabilities& capabilities);
    
    /**
     * @brief Get conflicting requirements
     */
    static std::vector<CapabilityRequirement> getConflictingRequirements(const std::vector<CapabilityRequirement>& requirements,
                                                                        const Capabilities& capabilities);
};

/**
 * @brief Capability negotiation
 */
class CapabilityNegotiator {
public:
    /**
     * @brief Negotiate capabilities between client and server
     */
    static Capabilities negotiate(const Capabilities& client_capabilities,
                                const Capabilities& server_capabilities,
                                const std::vector<CapabilityRequirement>& requirements = {});
    
    /**
     * @brief Get negotiation result
     */
    struct NegotiationResult {
        Capabilities negotiated_capabilities;
        std::vector<std::string> supported_capabilities;
        std::vector<std::string> unsupported_capabilities;
        std::vector<std::string> downgraded_capabilities;
        bool negotiation_successful;
    };
    
    /**
     * @brief Perform detailed capability negotiation
     */
    static NegotiationResult negotiateDetailed(const Capabilities& client_capabilities,
                                             const Capabilities& server_capabilities,
                                             const std::vector<CapabilityRequirement>& requirements = {});
};

/**
 * @brief Capability registry
 */
class CapabilityRegistry {
private:
    std::unordered_map<std::string, std::unordered_map<std::string, JsonValue>> capabilities_;
    std::mutex registry_mutex_;

public:
    /**
     * @brief Register a capability
     */
    void registerCapability(const std::string& name, const std::string& version, 
                           const std::unordered_map<std::string, JsonValue>& metadata = {});
    
    /**
     * @brief Unregister a capability
     */
    void unregisterCapability(const std::string& name);
    
    /**
     * @brief Check if capability is registered
     */
    bool hasCapability(const std::string& name) const;
    
    /**
     * @brief Get capability metadata
     */
    std::optional<std::unordered_map<std::string, JsonValue>> getCapabilityMetadata(const std::string& name) const;
    
    /**
     * @brief Get capability version
     */
    std::optional<std::string> getCapabilityVersion(const std::string& name) const;
    
    /**
     * @brief Get all registered capabilities
     */
    std::vector<std::string> getAllCapabilities() const;
    
    /**
     * @brief Clear all capabilities
     */
    void clear();
    
    /**
     * @brief Get capability count
     */
    size_t size() const;
};

/**
 * @brief Default capability sets
 */
namespace default_capabilities {

/**
 * @brief Get minimal client capabilities
 */
Capabilities getMinimalClientCapabilities();

/**
 * @brief Get full client capabilities
 */
Capabilities getFullClientCapabilities();

/**
 * @brief Get minimal server capabilities
 */
Capabilities getMinimalServerCapabilities();

/**
 * @brief Get full server capabilities
 */
Capabilities getFullServerCapabilities();

/**
 * @brief Get experimental capabilities
 */
Capabilities getExperimentalCapabilities();

/**
 * @brief Get development capabilities
 */
Capabilities getDevelopmentCapabilities();

/**
 * @brief Get production capabilities
 */
Capabilities getProductionCapabilities();

} // namespace default_capabilities

} // namespace mcp