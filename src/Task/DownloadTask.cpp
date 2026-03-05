#include "DownloadTask.hpp"
#include <filesystem>
#include <iostream>

namespace TinyMCP {
namespace Task {

DownloadTask::DownloadTask(const std::string& url, const std::string& session_id)
    : url_(url), session_id_(session_id) {

    http_client_ = std::make_unique<Utils::HttpClient>();
    session_manager_ = std::make_unique<Utils::SessionPersistence>();

    if (session_id_.empty()) {
        session_id_ = session_manager_->createSession(url_);
    }

    output_path_ = generateOutputPath(url_);

    // Set progress callback
    http_client_->setProgressCallback(
        [this](const Utils::DownloadProgress& progress) {
            onProgressUpdate(progress);
        }
    );
}

bool DownloadTask::execute() {
    try {
        // Try to resume existing session
        if (!session_id_.empty() && session_manager_->sessionExists(session_id_)) {
            Utils::DownloadSession session;
            if (session_manager_->loadSession(session_id_, session)) {
                output_path_ = session.output_path;
                if (session.downloaded_bytes > 0) {
                    std::cout << "Resuming download from " << session.downloaded_bytes 
                             << " bytes for session " << session_id_ << std::endl;
                    return http_client_->resumeDownload(url_, output_path_);
                }
            }
        }

        // Fresh download
        std::cout << "Starting new download: " << url_ << " -> " << output_path_ << std::endl;
        bool success = http_client_->downloadFile(url_, output_path_);

        if (success) {
            session_manager_->markSessionComplete(session_id_);
            is_complete_ = true;
        } else {
            session_manager_->markSessionFailed(session_id_, "Download failed");
        }

        return success;
    } catch (const std::exception& e) {
        session_manager_->markSessionFailed(session_id_, e.what());
        return false;
    }
}

void DownloadTask::cancel() {
    abort_requested_ = true;
    if (http_client_) {
        http_client_->abort();
    }
}

bool DownloadTask::isComplete() const {
    return is_complete_.load();
}

void DownloadTask::onProgressUpdate(const Utils::DownloadProgress& progress) {
    current_progress_ = progress;
    session_manager_->updateSessionProgress(session_id_, progress.downloaded_bytes);

    // Print progress
    std::cout << "\rProgress: " << std::fixed << std::setprecision(1) 
              << progress.progress_percent << "% (" 
              << progress.downloaded_bytes << "/" << progress.total_bytes << " bytes)" 
              << std::flush;

    if (progress.is_complete) {
        std::cout << std::endl;
        is_complete_ = true;
    }
}

std::string DownloadTask::generateOutputPath(const std::string& url) const {
    std::filesystem::path url_path(url);
    std::string filename = url_path.filename().string();

    if (filename.empty() || filename.find('.') == std::string::npos) {
        filename = "downloaded_file_" + session_id_;
    }

    return filename;
}

} // namespace Task
} // namespace TinyMCP
