#include "HttpClient.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace TinyMCP {
namespace Utils {

HttpClient::HttpClient() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_handle_ = curl_easy_init();
}

HttpClient::~HttpClient() {
    if (curl_handle_) {
        curl_easy_cleanup(curl_handle_);
    }
    curl_global_cleanup();
}

size_t HttpClient::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    if (!userp) return 0;

    std::ofstream* file = static_cast<std::ofstream*>(userp);
    size_t total_size = size * nmemb;
    file->write(static_cast<char*>(contents), total_size);
    return total_size;
}

int HttpClient::ProgressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, 
                                curl_off_t ultotal, curl_off_t ulnow) {
    HttpClient* client = static_cast<HttpClient*>(clientp);

    if (client->abort_requested_) {
        return 1; // Abort download
    }

    if (client->progress_callback_ && dltotal > 0) {
        DownloadProgress progress;
        progress.total_bytes = static_cast<size_t>(dltotal);
        progress.downloaded_bytes = static_cast<size_t>(dlnow);
        progress.progress_percent = (double)dlnow / dltotal * 100.0;
        progress.is_complete = (dlnow == dltotal);

        client->progress_callback_(progress);
    }

    return 0; // Continue
}

bool HttpClient::downloadFile(const std::string& url, const std::string& output_path) {
    if (!curl_handle_) return false;

    current_output_path_ = output_path;
    abort_requested_ = false;

    std::ofstream file(output_path, std::ios::binary);
    if (!file.is_open()) return false;

    curl_easy_setopt(curl_handle_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(curl_handle_, CURLOPT_PROGRESSFUNCTION, ProgressCallback);
    curl_easy_setopt(curl_handle_, CURLOPT_PROGRESSDATA, this);
    curl_easy_setopt(curl_handle_, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl_handle_, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl_handle_);
    file.close();

    if (res != CURLE_OK || abort_requested_) {
        std::filesystem::remove(output_path);
        return false;
    }

    return true;
}

bool HttpClient::resumeDownload(const std::string& url, const std::string& output_path) {
    if (!std::filesystem::exists(output_path)) {
        return downloadFile(url, output_path);
    }

    size_t existing_size = std::filesystem::file_size(output_path);

    std::ofstream file(output_path, std::ios::binary | std::ios::app);
    if (!file.is_open()) return false;

    curl_easy_setopt(curl_handle_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(curl_handle_, CURLOPT_RESUME_FROM, existing_size);

    CURLcode res = curl_easy_perform(curl_handle_);
    file.close();

    return res == CURLE_OK;
}

void HttpClient::setProgressCallback(std::function<void(const DownloadProgress&)> callback) {
    progress_callback_ = callback;
}

void HttpClient::abort() {
    abort_requested_ = true;
}

} // namespace Utils
} // namespace TinyMCP
