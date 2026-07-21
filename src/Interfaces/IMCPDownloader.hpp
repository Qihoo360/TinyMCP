#pragma once
#include "../Public/Core.hpp"
#include "../Utils/HttpClient.hpp"
#include <string>
#include <vector>
#include <memory>

namespace TinyMCP {
namespace Interfaces {

/**
 * @brief Interface for MCP-specific download operations
 * Extends the base download functionality with MCP-aware capabilities
 */
class IMCPDownloader {
public:
    virtual ~IMCPDownloader() = default;

    /**
     * @brief Download an MCP package from GitHub repository
     * @param repoUrl GitHub repository URL
     * @param destination Local destination path
     * @param version Specific version/tag (default: latest)
     * @return true if successful, false otherwise
     */
    virtual bool downloadMCPPackage(const std::string& repoUrl,
                                   const std::string& destination,
                                   const std::string& version = "latest") = 0;

    /**
     * @brief Download a Docker image for MCP server
     * @param imageName Docker image name
     * @param tag Docker image tag (default: latest)
     * @param destination Local destination path
     * @return true if successful, false otherwise
     */
    virtual bool downloadDockerImage(const std::string& imageName,
                                   const std::string& tag = "latest",
                                   const std::string& destination = "") = 0;

    /**
     * @brief Install an NPM package for MCP server
     * @param packageName NPM package name
     * @param version Package version (default: latest)
     * @param destination Installation directory
     * @return true if successful, false otherwise
     */
    virtual bool installNPMPackage(const std::string& packageName,
                                 const std::string& version = "latest",
                                 const std::string& destination = "") = 0;

    /**
     * @brief Download GitHub release assets
     * @param repo Repository in format "owner/repo"
     * @param tag Release tag (default: latest)
     * @param destination Local destination path
     * @return true if successful, false otherwise
     */
    virtual bool downloadGitHubRelease(const std::string& repo,
                                     const std::string& tag = "latest",
                                     const std::string& destination = "") = 0;

    /**
     * @brief Download configuration file for MCP server
     * @param configUrl Configuration file URL
     * @param destination Local destination path
     * @return true if successful, false otherwise
     */
    virtual bool downloadConfigurationFile(const std::string& configUrl,
                                         const std::string& destination) = 0;

    /**
     * @brief Download documentation for MCP server
     * @param docUrl Documentation URL
     * @param destination Local destination path
     * @return true if successful, false otherwise
     */
    virtual bool downloadDocumentation(const std::string& docUrl,
                                     const std::string& destination) = 0;

    /**
     * @brief Get download progress for a specific job
     * @param jobId Job identifier
     * @return Progress information
     */
    virtual Core::MCPDownloadProgress getDownloadProgress(const std::string& jobId) = 0;

    /**
     * @brief Cancel an active download
     * @param jobId Job identifier
     * @return true if cancelled successfully, false otherwise
     */
    virtual bool cancelDownload(const std::string& jobId) = 0;

    /**
     * @brief List all active download jobs
     * @return Vector of job IDs
     */
    virtual std::vector<std::string> listActiveDownloads() = 0;

    /**
     * @brief Set progress callback for downloads
     * @param callback Progress callback function
     */
    virtual void setProgressCallback(std::function<void(const Core::MCPDownloadProgress&)> callback) = 0;
};

/**
 * @brief MCP Download Manager implementation
 * Combines the existing download capabilities with MCP-specific features
 */
class MCPDownloadManager : public IMCPDownloader {
private:
    std::unique_ptr<Utils::HttpClient> http_client_;
    std::unordered_map<std::string, Core::MCPDownloadProgress> active_downloads_;
    std::function<void(const Core::MCPDownloadProgress&)> progress_callback_;
    std::mutex downloads_mutex_;

public:
    MCPDownloadManager();
    virtual ~MCPDownloadManager() = default;

    // IMCPDownloader interface implementation
    bool downloadMCPPackage(const std::string& repoUrl,
                           const std::string& destination,
                           const std::string& version = "latest") override;

    bool downloadDockerImage(const std::string& imageName,
                           const std::string& tag = "latest",
                           const std::string& destination = "") override;

    bool installNPMPackage(const std::string& packageName,
                         const std::string& version = "latest",
                         const std::string& destination = "") override;

    bool downloadGitHubRelease(const std::string& repo,
                             const std::string& tag = "latest",
                             const std::string& destination = "") override;

    bool downloadConfigurationFile(const std::string& configUrl,
                                 const std::string& destination) override;

    bool downloadDocumentation(const std::string& docUrl,
                             const std::string& destination) override;

    Core::MCPDownloadProgress getDownloadProgress(const std::string& jobId) override;
    bool cancelDownload(const std::string& jobId) override;
    std::vector<std::string> listActiveDownloads() override;
    void setProgressCallback(std::function<void(const Core::MCPDownloadProgress&)> callback) override;

private:
    std::string generateJobId();
    void updateProgress(const std::string& jobId, const Utils::DownloadProgress& progress);
    std::string getGitHubReleaseUrl(const std::string& repo, const std::string& tag);
    std::string getDockerImageUrl(const std::string& imageName, const std::string& tag);
    std::string getNPMDownloadUrl(const std::string& packageName, const std::string& version);
};

} // namespace Interfaces
} // namespace TinyMCP