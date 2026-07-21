#pragma once
#include "../Public/Core.hpp"
#include <string>
#include <vector>
#include <memory>

namespace TinyMCP {
namespace Interfaces {

/**
 * @brief Interface for MCP server lifecycle management
 */
class IMCPManager {
public:
    virtual ~IMCPManager() = default;

    /**
     * @brief Install an MCP server
     * @param serverName Name of the server to install
     * @param options Installation options
     * @return true if successful, false otherwise
     */
    virtual bool installServer(const std::string& serverName,
                              const Core::MCPInstallOptions& options) = 0;

    /**
     * @brief Uninstall an MCP server
     * @param serverName Name of the server to uninstall
     * @return true if successful, false otherwise
     */
    virtual bool uninstallServer(const std::string& serverName) = 0;

    /**
     * @brief Update an MCP server to a specific version
     * @param serverName Name of the server to update
     * @param version Target version
     * @return true if successful, false otherwise
     */
    virtual bool updateServer(const std::string& serverName,
                             const std::string& version) = 0;

    /**
     * @brief List all installed MCP servers
     * @return Vector of server information
     */
    virtual std::vector<Core::MCPServerInfo> listInstalledServers() = 0;

    /**
     * @brief Get information about a specific server
     * @param serverName Name of the server
     * @return Server information
     */
    virtual Core::MCPServerInfo getServerInfo(const std::string& serverName) = 0;

    /**
     * @brief Check if a server is installed
     * @param serverName Name of the server
     * @return true if installed, false otherwise
     */
    virtual bool isServerInstalled(const std::string& serverName) = 0;

    /**
     * @brief Validate server configuration
     * @param serverName Name of the server
     * @return true if configuration is valid, false otherwise
     */
    virtual bool validateServerConfiguration(const std::string& serverName) = 0;

    /**
     * @brief Generate server configuration template
     * @param serverName Name of the server
     * @param templatePath Path to template file
     * @return true if successful, false otherwise
     */
    virtual bool generateConfigurationTemplate(const std::string& serverName,
                                             const std::string& templatePath) = 0;
};

/**
 * @brief Interface for MCP server runtime management
 */
class IMCPRuntimeManager {
public:
    virtual ~IMCPRuntimeManager() = default;

    /**
     * @brief Start an MCP server
     * @param serverName Name of the server to start
     * @return true if successful, false otherwise
     */
    virtual bool startServer(const std::string& serverName) = 0;

    /**
     * @brief Stop an MCP server
     * @param serverName Name of the server to stop
     * @return true if successful, false otherwise
     */
    virtual bool stopServer(const std::string& serverName) = 0;

    /**
     * @brief Restart an MCP server
     * @param serverName Name of the server to restart
     * @return true if successful, false otherwise
     */
    virtual bool restartServer(const std::string& serverName) = 0;

    /**
     * @brief Get server status
     * @param serverName Name of the server
     * @return Server status
     */
    virtual Core::ServerStatus getServerStatus(const std::string& serverName) = 0;

    /**
     * @brief Get status of all servers
     * @return Vector of server statuses
     */
    virtual std::vector<std::pair<std::string, Core::ServerStatus>> getAllServerStatus() = 0;

    /**
     * @brief Get server logs
     * @param serverName Name of the server
     * @param lines Number of lines to retrieve (0 for all)
     * @return Server logs
     */
    virtual std::string getServerLogs(const std::string& serverName, int lines = 100) = 0;

    /**
     * @brief Follow server logs in real-time
     * @param serverName Name of the server
     * @param callback Log line callback function
     * @return true if successful, false otherwise
     */
    virtual bool followServerLogs(const std::string& serverName,
                                std::function<void(const std::string&)> callback) = 0;

    /**
     * @brief Get server resource usage
     * @param serverName Name of the server
     * @return Resource usage information (JSON format)
     */
    virtual std::string getServerResourceUsage(const std::string& serverName) = 0;
};

/**
 * @brief MCP Server Manager implementation
 * Handles installation, configuration, and lifecycle management of MCP servers
 */
class MCPServerManager : public IMCPManager {
private:
    std::string servers_directory_;
    std::unique_ptr<Interfaces::IMCPDownloader> downloader_;
    std::unordered_map<std::string, Core::MCPServerInfo> installed_servers_;

public:
    MCPServerManager(const std::string& serversDir = "./mcp_servers");
    virtual ~MCPServerManager() = default;

    // IMCPManager interface implementation
    bool installServer(const std::string& serverName,
                      const Core::MCPInstallOptions& options) override;

    bool uninstallServer(const std::string& serverName) override;
    bool updateServer(const std::string& serverName, const std::string& version) override;
    std::vector<Core::MCPServerInfo> listInstalledServers() override;
    Core::MCPServerInfo getServerInfo(const std::string& serverName) override;
    bool isServerInstalled(const std::string& serverName) override;
    bool validateServerConfiguration(const std::string& serverName) override;
    bool generateConfigurationTemplate(const std::string& serverName,
                                     const std::string& templatePath) override;

private:
    std::string getServerPath(const std::string& serverName);
    std::string getServerConfigPath(const std::string& serverName);
    bool extractArchive(const std::string& archivePath, const std::string& destination);
    bool installDependencies(const std::string& serverPath);
    bool createServerConfiguration(const std::string& serverName, const Core::MCPInstallOptions& options);
    void loadInstalledServers();
    void saveServerInfo(const Core::MCPServerInfo& info);
};

} // namespace Interfaces
} // namespace TinyMCP