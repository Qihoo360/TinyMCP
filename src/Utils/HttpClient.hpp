#pragma once
#include <string>
#include <memory>
#include <functional>
#include <curl/curl.h>

namespace TinyMCP {
namespace Utils {

struct DownloadProgress {
    size_t total_bytes = 0;
    size_t downloaded_bytes = 0;
    double progress_percent = 0.0;
    bool is_complete = false;
    std::string error_message;
};

class HttpClient {
private:
    CURL* curl_handle_;
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static int ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, 
                               curl_off_t ultotal, curl_off_t ulnow);

public:
    HttpClient();
    ~HttpClient();

    // Non-copyable
    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;

    bool downloadFile(const std::string& url, const std::string& output_path);
    bool downloadChunk(const std::string& url, const std::string& output_path, 
                      size_t start_byte, size_t chunk_size);
    bool resumeDownload(const std::string& url, const std::string& output_path);

    void setProgressCallback(std::function<void(const DownloadProgress&)> callback);
    void abort();

private:
    std::function<void(const DownloadProgress&)> progress_callback_;
    bool abort_requested_ = false;
    std::string current_output_path_;
};

} // namespace Utils
} // namespace TinyMCP
