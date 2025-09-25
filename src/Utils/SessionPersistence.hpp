#pragma once
#include <string>
#include <unordered_map>
#include <json/json.h>

namespace TinyMCP {
namespace Utils {

struct DownloadSession {
    std::string session_id;
    std::string url;
    std::string output_path;
    size_t total_bytes = 0;
    size_t downloaded_bytes = 0;
    bool is_complete = false;
    std::string status; // "active", "paused", "completed", "failed"
    int64_t created_timestamp = 0;
    int64_t last_modified_timestamp = 0;
};

class SessionPersistence {
private:
    std::string sessions_dir_;
    std::unordered_map<std::string, DownloadSession> active_sessions_;

public:
    SessionPersistence(const std::string& sessions_dir = "sessions");

    std::string createSession(const std::string& url, const std::string& output_path = "");
    bool saveSession(const DownloadSession& session);
    bool loadSession(const std::string& session_id, DownloadSession& session);
    bool sessionExists(const std::string& session_id) const;
    void updateSessionProgress(const std::string& session_id, size_t downloaded_bytes);
    void markSessionComplete(const std::string& session_id);
    void markSessionFailed(const std::string& session_id, const std::string& error);

    std::vector<std::string> listActiveSessions() const;
    bool removeSession(const std::string& session_id);

private:
    std::string generateSessionId() const;
    std::string getSessionFilePath(const std::string& session_id) const;
    Json::Value sessionToJson(const DownloadSession& session) const;
    DownloadSession sessionFromJson(const Json::Value& json) const;
};

} // namespace Utils
} // namespace TinyMCP
