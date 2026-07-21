#include "mcp/transport.hpp"
#include <curl/curl.h>
#include <thread>
#include <chrono>
#include <sstream>
#include <regex>

namespace mcp {

struct SseTransport::WriteCallback {
    std::string data;
    SseTransport* transport;
};

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, SseTransport::WriteCallback* userp) {
    size_t totalSize = size * nmemb;
    std::string chunk(static_cast<char*>(contents), totalSize);
    
    // Process SSE data line by line
    std::istringstream stream(chunk);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.empty() || line == "\r") {
            // Empty line indicates end of event
            if (!userp->data.empty()) {
                userp->transport->processSseEvent(userp->data);
                userp->data.clear();
            }
        } else if (line.substr(0, 6) == "data: ") {
            // SSE data line
            userp->data += line.substr(6) + "\n";
        }
        // Ignore other SSE fields like "event:", "id:", etc.
    }
    
    return totalSize;
}

SseTransport::SseTransport(const std::string& url) 
    : url_(url), connected_(false), running_(false), curl_(nullptr) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

SseTransport::~SseTransport() {
    disconnect();
    curl_global_cleanup();
}

bool SseTransport::connect() {
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
    
    // Configure CURL for SSE
    curl_easy_setopt(curl_, CURLOPT_URL, url_.c_str());
    curl_easy_setopt(curl_, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(curl_, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl_, CURLOPT_TCP_KEEPIDLE, 10L);
    curl_easy_setopt(curl_, CURLOPT_TCP_KEEPINTVL, 5L);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 0L); // No timeout for SSE
    curl_easy_setopt(curl_, CURLOPT_CONNECTTIMEOUT, 10L);
    
    // SSE specific headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: text/event-stream");
    headers = curl_slist_append(headers, "Cache-Control: no-cache");
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    
    // SSL settings
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 2L);
    
    connected_ = true;
    running_ = true;
    
    // Start SSE reading thread
    sse_thread_ = std::thread([this]() {
        WriteCallback callback;
        callback.transport = this;
        
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &callback);
        
        while (running_) {
            CURLcode res = curl_easy_perform(curl_);
            
            if (res != CURLE_OK) {
                if (running_ && onError) {
                    onError("SSE connection error: " + std::string(curl_easy_strerror(res)));
                }
                break;
            }
            
            // Check if we should reconnect
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    return true;
}

void SseTransport::disconnect() {
    running_ = false;
    connected_ = false;
    
    if (sse_thread_.joinable()) {
        sse_thread_.join();
    }
    
    if (curl_) {
        curl_easy_cleanup(curl_);
        curl_ = nullptr;
    }
}

bool SseTransport::isConnected() const {
    return connected_ && running_;
}

bool SseTransport::write(const std::string& data) {
    // SSE is unidirectional from server to client
    // Writing is not supported in SSE transport
    if (onError) {
        onError("SSE transport does not support writing");
    }
    return false;
}

std::string SseTransport::read() {
    // For SSE transport, reading is handled asynchronously
    // in the SSE thread. This method is mainly for compatibility.
    return "";
}

void SseTransport::setMessageCallback(std::function<void(const std::string&)> callback) {
    onMessage = callback;
}

void SseTransport::setErrorCallback(std::function<void(const std::string&)> callback) {
    onError = callback;
}

void SseTransport::processSseEvent(const std::string& eventData) {
    if (onMessage && !eventData.empty()) {
        // Remove trailing newline if present
        std::string cleanData = eventData;
        if (!cleanData.empty() && cleanData.back() == '\n') {
            cleanData.pop_back();
        }
        onMessage(cleanData);
    }
}

} // namespace mcp