#pragma once
#include "../Public/Core.hpp"
#include "../Utils/HttpClient.hpp"
#include "../Utils/SessionPersistence.hpp"
#include <memory>
#include <atomic>
#include <thread>

namespace TinyMCP {
namespace Task {

class DownloadTask : public TaskBase {
private:
    std::string url_;
    std::string session_id_;
    std::string output_path_;
    std::unique_ptr<Utils::HttpClient> http_client_;
    std::unique_ptr<Utils::SessionPersistence> session_manager_;
    std::atomic<bool> abort_requested_{false};
    std::atomic<bool> is_complete_{false};
    Utils::DownloadProgress current_progress_;

public:
    DownloadTask(const std::string& url, const std::string& session_id = "");
    virtual ~DownloadTask() = default;

    // TaskBase overrides
    bool execute() override;
    void cancel() override;
    bool isComplete() const override;

    // Download-specific methods
    void setOutputPath(const std::string& path);
    std::string getSessionId() const { return session_id_; }
    Utils::DownloadProgress getProgress() const { return current_progress_; }

    // For chunk-based processing (tick-based)
    bool processChunk();
    bool resumeFromSession();

private:
    void onProgressUpdate(const Utils::DownloadProgress& progress);
    std::string generateOutputPath(const std::string& url) const;
};

} // namespace Task
} // namespace TinyMCP
