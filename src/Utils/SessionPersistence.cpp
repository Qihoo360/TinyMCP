#include "SessionPersistence.hpp"
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace TinyMCP {
namespace Utils {

SessionPersistence::SessionPersistence(const std::string& sessions_dir) 
    : sessions_dir_(sessions_dir) {
    std::filesystem::create_directories(sessions_dir_);
}

std::string SessionPersistence::generateSessionId() const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    for (int i = 0; i < 8; ++i) {
        ss << std::hex << dis(gen);
    }
    return ss.str();
}

std::string SessionPersistence::createSession(const std::string& url, const std::string& output_path) {
    std::string session_id = generateSessionId();

    DownloadSession session;
    session.session_id = session_id;
    session.url = url;
    session.output_path = output_path.empty() ? 
        std::filesystem::path(url).filename().string() : output_path;
    session.status = "active";

    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    session.created_timestamp = timestamp;
    session.last_modified_timestamp = timestamp;

    active_sessions_[session_id] = session;
    saveSession(session);

    return session_id;
}

bool SessionPersistence::saveSession(const DownloadSession& session) {
    try {
        Json::Value json = sessionToJson(session);
        std::ofstream file(getSessionFilePath(session.session_id));
        Json::StreamWriterBuilder builder;
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(json, &file);
        return true;
    } catch (...) {
        return false;
    }
}

bool SessionPersistence::loadSession(const std::string& session_id, DownloadSession& session) {
    try {
        std::ifstream file(getSessionFilePath(session_id));
        if (!file.is_open()) return false;

        Json::Value json;
        Json::CharReaderBuilder builder;
        std::string errors;

        if (Json::parseFromStream(builder, file, &json, &errors)) {
            session = sessionFromJson(json);
            active_sessions_[session_id] = session;
            return true;
        }
    } catch (...) {
        // Fall through to return false
    }
    return false;
}

std::string SessionPersistence::getSessionFilePath(const std::string& session_id) const {
    return sessions_dir_ + "/" + session_id + ".json";
}

Json::Value SessionPersistence::sessionToJson(const DownloadSession& session) const {
    Json::Value json;
    json["session_id"] = session.session_id;
    json["url"] = session.url;
    json["output_path"] = session.output_path;
    json["total_bytes"] = static_cast<Json::UInt64>(session.total_bytes);
    json["downloaded_bytes"] = static_cast<Json::UInt64>(session.downloaded_bytes);
    json["is_complete"] = session.is_complete;
    json["status"] = session.status;
    json["created_timestamp"] = static_cast<Json::Int64>(session.created_timestamp);
    json["last_modified_timestamp"] = static_cast<Json::Int64>(session.last_modified_timestamp);
    return json;
}

DownloadSession SessionPersistence::sessionFromJson(const Json::Value& json) const {
    DownloadSession session;
    session.session_id = json["session_id"].asString();
    session.url = json["url"].asString();
    session.output_path = json["output_path"].asString();
    session.total_bytes = json["total_bytes"].asUInt64();
    session.downloaded_bytes = json["downloaded_bytes"].asUInt64();
    session.is_complete = json["is_complete"].asBool();
    session.status = json["status"].asString();
    session.created_timestamp = json["created_timestamp"].asInt64();
    session.last_modified_timestamp = json["last_modified_timestamp"].asInt64();
    return session;
}

bool SessionPersistence::sessionExists(const std::string& session_id) const {
    return std::filesystem::exists(getSessionFilePath(session_id));
}

void SessionPersistence::updateSessionProgress(const std::string& session_id, size_t downloaded_bytes) {
    auto it = active_sessions_.find(session_id);
    if (it != active_sessions_.end()) {
        it->second.downloaded_bytes = downloaded_bytes;
        auto now = std::chrono::system_clock::now();
        it->second.last_modified_timestamp = 
            std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        saveSession(it->second);
    }
}

} // namespace Utils
} // namespace TinyMCP
