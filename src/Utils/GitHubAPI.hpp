#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <json/json.h>

namespace TinyMCP {
namespace Utils {

/**
 * @brief GitHub repository information
 */
struct GitHubRepoInfo {
    std::string name;
    std::string full_name;
    std::string description;
    std::string html_url;
    std::string clone_url;
    std::string default_branch;
    std::string language;
    bool is_private = false;
    int64_t stargazers_count = 0;
    int64_t forks_count = 0;
    std::string created_at;
    std::string updated_at;
};

/**
 * @brief GitHub release information
 */
struct GitHubReleaseInfo {
    std::string tag_name;
    std::string name;
    std::string body;
    std::string html_url;
    std::string zipball_url;
    std::string tarball_url;
    bool prerelease = false;
    bool draft = false;
    std::string created_at;
    std::string published_at;
    std::vector<std::string> asset_urls;
};

/**
 * @brief GitHub API client for MCP server discovery and download
 */
class GitHubAPI {
private:
    std::string api_base_url_;
    std::string token_;
    std::unique_ptr<Utils::HttpClient> http_client_;

public:
    GitHubAPI(const std::string& token = "");
    virtual ~GitHubAPI() = default;

    /**
     * @brief Set GitHub API token for authenticated requests
     * @param token GitHub personal access token
     */
    void setToken(const std::string& token);

    /**
     * @brief Search for MCP servers on GitHub
     * @param query Search query (e.g., "MCP server", "model context protocol")
     * @param language Programming language filter
     * @param sort Sort criteria (stars, updated, created)
     * @param limit Maximum number of results
     * @return Vector of repository information
     */
    std::vector<GitHubRepoInfo> searchMCPServers(const std::string& query = "MCP server",
                                                const std::string& language = "",
                                                const std::string& sort = "stars",
                                                int limit = 30);

    /**
     * @brief Get repository information
     * @param owner Repository owner
     * @param repo Repository name
     * @return Repository information
     */
    GitHubRepoInfo getRepositoryInfo(const std::string& owner, const std::string& repo);

    /**
     * @brief Get repository releases
     * @param owner Repository owner
     * @param repo Repository name
     * @param limit Maximum number of releases
     * @return Vector of release information
     */
    std::vector<GitHubReleaseInfo> getRepositoryReleases(const std::string& owner,
                                                       const std::string& repo,
                                                       int limit = 10);

    /**
     * @brief Get latest release information
     * @param owner Repository owner
     * @param repo Repository name
     * @return Latest release information
     */
    GitHubReleaseInfo getLatestRelease(const std::string& owner, const std::string& repo);

    /**
     * @brief Get release by tag
     * @param owner Repository owner
     * @param repo Repository name
     * @param tag Release tag
     * @return Release information
     */
    GitHubReleaseInfo getReleaseByTag(const std::string& owner,
                                    const std::string& repo,
                                    const std::string& tag);

    /**
     * @brief Download repository as ZIP archive
     * @param owner Repository owner
     * @param repo Repository name
     * @param destination Local destination path
     * @param ref Git reference (branch, tag, or commit)
     * @return true if successful, false otherwise
     */
    bool downloadRepository(const std::string& owner,
                          const std::string& repo,
                          const std::string& destination,
                          const std::string& ref = "main");

    /**
     * @brief Download release assets
     * @param owner Repository owner
     * @param repo Repository name
     * @param tag Release tag
     * @param destination Local destination directory
     * @return true if successful, false otherwise
     */
    bool downloadReleaseAssets(const std::string& owner,
                             const std::string& repo,
                             const std::string& tag,
                             const std::string& destination);

    /**
     * @brief Check if repository has MCP server configuration
     * @param owner Repository owner
     * @param repo Repository name
     * @return true if MCP server detected, false otherwise
     */
    bool isMCPServer(const std::string& owner, const std::string& repo);

    /**
     * @brief Get MCP server configuration from repository
     * @param owner Repository owner
     * @param repo Repository name
     * @return MCP configuration as JSON
     */
    Json::Value getMCPConfiguration(const std::string& owner, const std::string& repo);

private:
    std::string makeAPIRequest(const std::string& endpoint);
    Json::Value parseJSONResponse(const std::string& response);
    GitHubRepoInfo parseRepoInfo(const Json::Value& json);
    GitHubReleaseInfo parseReleaseInfo(const Json::Value& json);
    std::string buildSearchQuery(const std::string& query, const std::string& language);
    std::vector<std::string> getMCPIndicatorFiles();
    bool checkFileExists(const std::string& owner, const std::string& repo, const std::string& path);
};

} // namespace Utils
} // namespace TinyMCP