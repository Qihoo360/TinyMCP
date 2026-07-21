#pragma once
#include "../Interfaces/IMCPDownloader.hpp"
#include "../Interfaces/IMCPManager.hpp"
#include "../Utils/MCPConfigParser.hpp"
#include "../Utils/GitHubAPI.hpp"
#include <memory>
#include <string>
#include <vector>

namespace TinyMCP {
namespace MCP {

/**
 * @brief Main MCP Server Management Wrapper
 * Integrates download capabilities with MCP server management
 */
class MCPServerWrapper {
private:
    std::unique_ptr<Interfaces::IMCPDownloader> download_manager_;
    std::unique_ptr<Interfaces::IMCPManager> server_manager_;
    std::unique_ptr<Interfaces::IMCPRuntimeManager> runtime_manager_;
    std::unique_ptr<Utils::MCPConfigurationManager> config_manager_;
    std::unique_ptr<Utils::GitHubAPI> github_api_;

public:
    MCPServerWrapper();
    virtual ~MCPServerWrapper() = default;

    // MCP Tools for download operations
    std::string downloadPackage(const std::string& url,
                               const std::string& destination = "");
    
    std::string downloadGitHubRepo(const std::string& repo,
                                  const std::string& destination = "",
                                  const std::string& version = "latest");
    
    std::string downloadDockerImage(const std::string& imageName,
                                   const std::string& tag = "latest");
    
    std::string installNPMPackage(const std::string& packageName,
                                 const std::string& version = "latest");

    // MCP Tools for server management
    std::string installMCPServer(const std::string& serverName,
                                const std::string& version = "latest");
    
    std::string uninstallMCPServer(const std::string& serverName);
    
    std::string updateMCPServer(const std::string& serverName,
                               const std::string& version);
    
    std::string listInstalledServers();
    
    std::string listAvailableServers(const std::string& query = "MCP server");

    // MCP Tools for runtime management
    std::string startMCPServer(const std::string& serverName);
    
    std::string stopMCPServer(const std::string& serverName);
    
    std::string restartMCPServer(const std::string& serverName);
    
    std::string getServerStatus(const std::string& serverName);
    
    std::string getAllServerStatus();

    // MCP Tools for monitoring and information
    std::string getDownloadProgress(const std::string& jobId);
    
    std::string listActiveDownloads();
    
    std::string getDownloadLogs();
    
    std::string getConfiguration(const std::string& serverName = "");
    
    std::string getServerLogs(const std::string& serverName, int lines = 100);
    
    std::string getServerResourceUsage(const std::string& serverName);

    // MCP Resources
    std::string getDownloadLogsResource();
    std::string getConfigurationResource(const std::string& serverName = "");
    std::string getServerStatusResource();
    std::string getServerLogsResource(const std::string& serverName);

    // Utility methods
    std::string searchMCPServers(const std::string& query,
                                const std::string& language = "",
                                int limit = 30);
    
    std::string validateServerConfiguration(const std::string& serverName);
    
    std::string generateConfigurationTemplate(const std::string& serverName,
                                            const std::string& templatePath = "");

    // Configuration management
    bool loadConfiguration(const std::string& configPath);
    std::string getServerInfo(const std::string& serverName);
    std::vector<std::string> getServerTools(const std::string& serverName);
    std::vector<std::string> getServerResources(const std::string& serverName);

private:
    std::string formatJsonResponse(bool success, const std::string& message, const std::string& data = "");
    std::string formatErrorResponse(const std::string& error);
    std::string formatSuccessResponse(const std::string& message, const std::string& data = "");
    bool initializeComponents();
    std::string sanitizeInput(const std::string& input);
};

/**
 * @brief MCP Server Discovery and Management
 * Provides high-level operations for MCP server ecosystem management
 */
class MCPEcosystemManager {
private:
    std::unique_ptr<MCPServerWrapper> server_wrapper_;

public:
    MCPEcosystemManager();
    virtual ~MCPEcosystemManager() = default;

    /**
     * @brief Discover and install MCP servers from GitHub
     * @param searchQuery Search query for GitHub
     * @param maxResults Maximum number of results
     * @return JSON string with discovery results
     */
    std::string discoverAndInstallServers(const std::string& searchQuery = "MCP server",
                                         int maxResults = 10);

    /**
     * @brief Setup complete MCP environment
     * @param configPath Path to configuration file
     * @return true if setup successful, false otherwise
     */
    bool setupMCPEnvironment(const std::string& configPath);

    /**
     * @brief Get ecosystem health status
     * @return JSON string with health information
     */
    std::string getEcosystemHealth();

    /**
     * @brief Update all installed servers
     * @return JSON string with update results
     */
    std::string updateAllServers();

    /**
     * @brief Backup server configurations
     * @param backupPath Path to backup directory
     * @return true if backup successful, false otherwise
     */
    bool backupConfigurations(const std::string& backupPath);

    /**
     * @brief Restore server configurations
     * @param backupPath Path to backup directory
     * @return true if restore successful, false otherwise
     */
    bool restoreConfigurations(const std::string& backupPath);
};

} // namespace MCP
} // namespace TinyMCP