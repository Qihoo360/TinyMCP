#pragma once
#include <string>
#include <vector>
#include <memory>
#include <atomic>

namespace TinyMCP {
namespace Core {

/**
 * @brief Base class for all tasks in the TinyMCP system
 */
class TaskBase {
public:
    virtual ~TaskBase() = default;
    
    /**
     * @brief Execute the task
     * @return true if successful, false otherwise
     */
    virtual bool execute() = 0;
    
    /**
     * @brief Cancel the task
     */
    virtual void cancel() = 0;
    
    /**
     * @brief Check if the task is complete
     * @return true if complete, false otherwise
     */
    virtual bool isComplete() const = 0;
    
    /**
     * @brief Get the task ID
     * @return unique task identifier
     */
    virtual std::string getTaskId() const = 0;
};

/**
 * @brief MCP Resource types for different download scenarios
 */
enum class MCPResourceType {
    GitHubRepo,         // GitHub repository
    DockerImage,        // Docker container image
    NPMPackage,         // NPM package
    ConfigurationFile,  // Configuration file
    Documentation,      // Documentation files
    BinaryRelease,      // Binary release package
    SourceCode,         // Source code archive
    Dependencies        // Package dependencies
};

/**
 * @brief MCP Server installation options
 */
struct MCPInstallOptions {
    std::string version = "latest";
    std::string destination = "";
    bool install_dependencies = true;
    bool create_config = true;
    std::string config_template = "";
    std::vector<std::string> environment_vars;
};

/**
 * @brief MCP Server information
 */
struct MCPServerInfo {
    std::string name;
    std::string version;
    std::string description;
    std::string repository_url;
    std::string install_path;
    std::string config_path;
    bool is_running = false;
    std::string status;
    int64_t last_updated = 0;
};

/**
 * @brief MCP Server status
 */
enum class ServerStatus {
    Unknown,
    NotInstalled,
    Installed,
    Running,
    Stopped,
    Error,
    Updating
};

/**
 * @brief MCP Download progress information
 */
struct MCPDownloadProgress {
    std::string job_id;
    std::string resource_type;
    std::string url;
    std::string destination;
    size_t total_bytes = 0;
    size_t downloaded_bytes = 0;
    double progress_percent = 0.0;
    bool is_complete = false;
    std::string status;
    std::string error_message;
    int64_t start_time = 0;
    int64_t last_update = 0;
};

/**
 * @brief MCP Configuration structure
 */
struct MCPConfiguration {
    std::string server_name;
    std::string version;
    std::string description;
    std::vector<std::string> tools;
    std::vector<std::string> resources;
    std::vector<std::string> prompts;
    std::unordered_map<std::string, std::string> environment;
    std::unordered_map<std::string, std::string> settings;
};

} // namespace Core
} // namespace TinyMCP