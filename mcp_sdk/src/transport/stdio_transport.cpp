#include "mcp/transport.hpp"
#include <iostream>
#include <thread>
#include <chrono>

namespace mcp {

StdioTransport::StdioTransport() : connected_(false), running_(false) {}

StdioTransport::~StdioTransport() {
    disconnect();
}

bool StdioTransport::connect() {
    if (connected_) {
        return true;
    }
    
    // For stdio, we're always "connected" to stdin/stdout
    connected_ = true;
    running_ = true;
    
    // Start reading thread
    read_thread_ = std::thread([this]() {
        std::string line;
        while (running_ && std::getline(std::cin, line)) {
            if (!line.empty() && onMessage) {
                onMessage(line);
            }
        }
    });
    
    return true;
}

void StdioTransport::disconnect() {
    running_ = false;
    connected_ = false;
    
    if (read_thread_.joinable()) {
        read_thread_.join();
    }
}

bool StdioTransport::isConnected() const {
    return connected_ && running_;
}

bool StdioTransport::write(const std::string& data) {
    if (!isConnected()) {
        return false;
    }
    
    try {
        std::cout << data << std::endl;
        std::cout.flush();
        return true;
    } catch (const std::exception& e) {
        if (onError) {
            onError("Write error: " + std::string(e.what()));
        }
        return false;
    }
}

std::string StdioTransport::read() {
    // For stdio transport, reading is handled asynchronously
    // in the read thread. This method is mainly for compatibility.
    return "";
}

void StdioTransport::setMessageCallback(std::function<void(const std::string&)> callback) {
    onMessage = callback;
}

void StdioTransport::setErrorCallback(std::function<void(const std::string&)> callback) {
    onError = callback;
}

} // namespace mcp