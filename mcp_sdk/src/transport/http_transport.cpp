#include "mcp/transport.hpp"
#include <curl/curl.h>
#include <thread>
#include <chrono>
#include <sstream>
#include <mutex>

namespace mcp {

struct HttpTransport::WriteCallback {
    std::string data;
};

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, HttpTransport::WriteCallback* userp) {
    size_t totalSize = size * nmemb;
    userp->data.append((char*)contents, totalSize);
    return totalSize;
}

static size_t ReadCallback(char* ptr, size_t size, size_t nmemb, std::string* userp) {
    size_t copySize = std::min(size * nmemb, userp->length());
    memcpy(ptr, userp->c_str(), copySize);
    userp->erase(0, copySize);
    return copySize;
}

HttpTransport::HttpTransport(const std::string& url) 
    : url_(url), connected_(false), running_(false), curl_(nullptr) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

HttpTransport::~HttpTransport() {
    disconnect();
    curl_global_cleanup();
}

bool HttpTransport::connect() {
    if (connected_) {
        return true;
    }
    
    curl_ = curl_easy_init();
    if (!curl_) {
        if (onError) {
            onError("Failed to initialize CURL");
        }
        return false;
    }
    
    // Configure CURL
    curl_easy_setopt(curl_, CURLOPT_URL, url_.c_str());
    curl_easy_setopt(curl_, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(curl_, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl_, CURLOPT_TCP_KEEPIDLE, 10L);
    curl_easy_setopt(curl_, CURLOPT_TCP_KEEPINTVL, 5L);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, 10L);
    
    // SSL settings
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 2L);
    
    connected_ = true;
    running_ = true;
    
    return true;
}

void HttpTransport::disconnect() {
    running_ = false;
    connected_ = false;
    
    if (curl_) {
        curl_easy_cleanup(curl_);
        curl_ = nullptr;
    }
}

bool HttpTransport::isConnected() const {
    return connected_ && running_;
}

bool HttpTransport::write(const std::string& data) {
    if (!isConnected() || !curl_) {
        return false;
    }
    
    try {
        WriteCallback callback;
        
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, data.length());
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &callback);
        
        CURLcode res = curl_easy_perform(curl_);
        
        if (res != CURLE_OK) {
            if (onError) {
                onError("HTTP request failed: " + std::string(curl_easy_strerror(res)));
            }
            return false;
        }
        
        long responseCode;
        curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &responseCode);
        
        if (responseCode >= 200 && responseCode < 300) {
            if (!callback.data.empty() && onMessage) {
                onMessage(callback.data);
            }
            return true;
        } else {
            if (onError) {
                onError("HTTP error: " + std::to_string(responseCode));
            }
            return false;
        }
        
    } catch (const std::exception& e) {
        if (onError) {
            onError("HTTP write error: " + std::string(e.what()));
        }
        return false;
    }
}

std::string HttpTransport::read() {
    // For HTTP transport, reading is handled in the write method
    // when we receive a response. This method is mainly for compatibility.
    return "";
}

void HttpTransport::setMessageCallback(std::function<void(const std::string&)> callback) {
    onMessage = callback;
}

void HttpTransport::setErrorCallback(std::function<void(const std::string&)> callback) {
    onError = callback;
}

} // namespace mcp