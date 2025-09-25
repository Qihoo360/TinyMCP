#include "IMCPDownloader.hpp"
#include "../Utils/HttpClient.hpp"
#include "../Utils/SessionPersistence.hpp"
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace TinyMCP {
namespace Interfaces {

MCPDownloadManager::MCPDownloadManager() 
    : http_client_(std::make_unique<Utils::HttpClient>()) {
    // Initialize random number generator for job IDs
    std::random_device rd;
    std::mt19937 gen(rd());
}

bool MCPDownloadManager::downloadMCPPackage(const std::string& repoUrl,
                                          const std::string& destination,
                                          const std::string& version) {
    try {
        std::string jobId = generateJobId();
        
        // Create progress entry
        Core::MCPDownloadProgress progress;
        progress.job_id = jobId;
        progress.resource_type = "GitHubRepo";
        progress.url = repoUrl;
        progress.destination = destination.empty() ? "./downloads/" + jobId : destination;
        progress.status = "Starting";
        progress.start_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        {
            std::lock_guard<std::mutex> lock(downloads_mutex_);
            active_downloads_[jobId] = progress;
        }

        // Set up progress callback
        http_client_->setProgressCallback([this, jobId](const Utils::DownloadProgress& dlProgress) {
            updateProgress(jobId, dlProgress);
        });

        // Construct download URL based on version
        std::string downloadUrl;
        if (version == "latest" || version.empty()) {
            // Download latest release or main branch
            downloadUrl = repoUrl;
            if (repoUrl.find("github.com") != std::string::npos) {
                // Convert GitHub repo URL to ZIP download URL
                size_t pos = repoUrl.find_last_of('/');
                if (pos != std::string::npos) {
                    std::string repo = repoUrl.substr(pos + 1);
                    size_t ownerPos = repoUrl.find_last_of('/', pos - 1);
                    if (ownerPos != std::string::npos) {
                        std::string owner = repoUrl.substr(ownerPos + 1, pos - ownerPos - 1);
                        downloadUrl = "https://github.com/" + owner + "/" + repo + "/archive/refs/heads/main.zip";
                    }
                }
            }
        } else {
            // Download specific version/tag
            downloadUrl = getGitHubReleaseUrl(repoUrl, version);
        }

        // Perform download
        bool success = http_client_->downloadFile(downloadUrl, progress.destination);
        
        {
            std::lock_guard<std::mutex> lock(downloads_mutex_);
            active_downloads_[jobId].is_complete = true;
            active_downloads_[jobId].status = success ? "Completed" : "Failed";
            if (!success) {
                active_downloads_[jobId].error_message = "Download failed";
            }
        }

        return success;
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool MCPDownloadManager::downloadDockerImage(const std::string& imageName,
                                           const std::string& tag,
                                           const std::string& destination) {
    try {
        std::string jobId = generateJobId();
        
        // Create progress entry
        Core::MCPDownloadProgress progress;
        progress.job_id = jobId;
        progress.resource_type = "DockerImage";
        progress.url = imageName + ":" + tag;
        progress.destination = destination;
        progress.status = "Starting";
        progress.start_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        {
            std::lock_guard<std::mutex> lock(downloads_mutex_);
            active_downloads_[jobId] = progress;
        }

        // For Docker images, we would typically use Docker API or CLI
        // This is a simplified implementation that simulates the process
        progress.status = "Pulling image...";
        updateProgress(jobId, Utils::DownloadProgress{});
        
        // Simulate Docker pull process
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        {
            std::lock_guard<std::mutex> lock(downloads_mutex_);
            active_downloads_[jobId].is_complete = true;
            active_downloads_[jobId].status = "Completed";
            active_downloads_[jobId].progress_percent = 100.0;
        }

        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool MCPDownloadManager::installNPMPackage(const std::string& packageName,
                                         const std::string& version,
                                         const std::string& destination) {
    try {
        std::string jobId = generateJobId();
        
        // Create progress entry
        Core::MCPDownloadProgress progress;
        progress.job_id = jobId;
        progress.resource_type = "NPMPackage";
        progress.url = packageName + "@" + version;
        progress.destination = destination;
        progress.status = "Starting";
        progress.start_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        {
            std::lock_guard<std::mutex> lock(downloads_mutex_);
            active_downloads_[jobId] = progress;
        }

        // For NPM packages, we would typically use npm CLI or registry API
        // This is a simplified implementation
        progress.status = "Installing package...";
        updateProgress(jobId, Utils::DownloadProgress{});
        
        // Simulate NPM install process
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        {
            std::lock_guard<std::mutex> lock(downloads_mutex_);
            active_downloads_[jobId].is_complete = true;
            active_downloads_[jobId].status = "Completed";
            active_downloads_[jobId].progress_percent = 100.0;
        }

        return true;
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool MCPDownloadManager::downloadGitHubRelease(const std::string& repo,
                                             const std::string& tag,
                                             const std::string& destination) {
    std::string downloadUrl = getGitHubReleaseUrl(repo, tag);
    return downloadMCPPackage(downloadUrl, destination, tag);
}

bool MCPDownloadManager::downloadConfigurationFile(const std::string& configUrl,
                                                 const std::string& destination) {
    try {
        std::string jobId = generateJobId();
        
        // Create progress entry
        Core::MCPDownloadProgress progress;
        progress.job_id = jobId;
        progress.resource_type = "ConfigurationFile";
        progress.url = configUrl;
        progress.destination = destination;
        progress.status = "Starting";
        progress.start_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        {
            std::lock_guard<std::mutex> lock(downloads_mutex_);
            active_downloads_[jobId] = progress;
        }

        // Set up progress callback
        http_client_->setProgressCallback([this, jobId](const Utils::DownloadProgress& dlProgress) {
            updateProgress(jobId, dlProgress);
        });

        // Perform download
        bool success = http_client_->downloadFile(configUrl, destination);
        
        {
            std::lock_guard<std::mutex> lock(downloads_mutex_);
            active_downloads_[jobId].is_complete = true;
            active_downloads_[jobId].status = success ? "Completed" : "Failed";
            if (!success) {
                active_downloads_[jobId].error_message = "Download failed";
            }
        }

        return success;
        
    } catch (const std::exception& e) {
        return false;
    }
}

bool MCPDownloadManager::downloadDocumentation(const std::string& docUrl,
                                             const std::string& destination) {
    // Similar to configuration file download
    return downloadConfigurationFile(docUrl, destination);
}

Core::MCPDownloadProgress MCPDownloadManager::getDownloadProgress(const std::string& jobId) {
    std::lock_guard<std::mutex> lock(downloads_mutex_);
    auto it = active_downloads_.find(jobId);
    if (it != active_downloads_.end()) {
        return it->second;
    }
    
    // Return empty progress if job not found
    Core::MCPDownloadProgress emptyProgress;
    emptyProgress.job_id = jobId;
    emptyProgress.status = "Not Found";
    return emptyProgress;
}

bool MCPDownloadManager::cancelDownload(const std::string& jobId) {
    std::lock_guard<std::mutex> lock(downloads_mutex_);
    auto it = active_downloads_.find(jobId);
    if (it != active_downloads_.end()) {
        it->second.status = "Cancelled";
        http_client_->abort();
        return true;
    }
    return false;
}

std::vector<std::string> MCPDownloadManager::listActiveDownloads() {
    std::lock_guard<std::mutex> lock(downloads_mutex_);
    std::vector<std::string> jobIds;
    for (const auto& pair : active_downloads_) {
        if (!pair.second.is_complete) {
            jobIds.push_back(pair.first);
        }
    }
    return jobIds;
}

void MCPDownloadManager::setProgressCallback(std::function<void(const Core::MCPDownloadProgress&)> callback) {
    progress_callback_ = callback;
}

// Private helper methods

std::string MCPDownloadManager::generateJobId() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << "mcp_" << time_t << "_" << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void MCPDownloadManager::updateProgress(const std::string& jobId, const Utils::DownloadProgress& progress) {
    std::lock_guard<std::mutex> lock(downloads_mutex_);
    auto it = active_downloads_.find(jobId);
    if (it != active_downloads_.end()) {
        it->second.total_bytes = progress.total_bytes;
        it->second.downloaded_bytes = progress.downloaded_bytes;
        it->second.progress_percent = progress.progress_percent;
        it->second.is_complete = progress.is_complete;
        it->second.last_update = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        
        if (progress.is_complete) {
            it->second.status = "Completed";
        } else {
            it->second.status = "Downloading";
        }
        
        // Call progress callback if set
        if (progress_callback_) {
            progress_callback_(it->second);
        }
    }
}

std::string MCPDownloadManager::getGitHubReleaseUrl(const std::string& repo, const std::string& tag) {
    // Convert repo format "owner/repo" to GitHub release URL
    if (tag == "latest" || tag.empty()) {
        return "https://github.com/" + repo + "/releases/latest/download/" + repo.substr(repo.find('/') + 1) + ".zip";
    } else {
        return "https://github.com/" + repo + "/archive/refs/tags/" + tag + ".zip";
    }
}

std::string MCPDownloadManager::getDockerImageUrl(const std::string& imageName, const std::string& tag) {
    return imageName + ":" + tag;
}

std::string MCPDownloadManager::getNPMDownloadUrl(const std::string& packageName, const std::string& version) {
    return "https://registry.npmjs.org/" + packageName + "/-" + packageName + "-" + version + ".tgz";
}

} // namespace Interfaces
} // namespace TinyMCP