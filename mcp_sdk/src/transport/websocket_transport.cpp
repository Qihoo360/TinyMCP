#include "mcp/transport.hpp"
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

#ifdef MCP_USE_LIBWEBSOCKETS
#include <libwebsockets.h>
#endif

namespace mcp {

WebSocketTransport::WebSocketTransport(const std::string& url) 
    : url_(url), connected_(false), running_(false) {
#ifdef MCP_USE_LIBWEBSOCKETS
    // Initialize libwebsockets context
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = websocket_protocols;
    info.gid = -1;
    info.uid = -1;
    
    context_ = lws_create_context(&info);
    if (!context_) {
        throw std::runtime_error("Failed to create libwebsockets context");
    }
#endif
}

WebSocketTransport::~WebSocketTransport() {
    disconnect();
#ifdef MCP_USE_LIBWEBSOCKETS
    if (context_) {
        lws_context_destroy(context_);
    }
#endif
}

bool WebSocketTransport::connect() {
    if (connected_) {
        return true;
    }
    
#ifdef MCP_USE_LIBWEBSOCKETS
    // Parse URL
    struct lws_client_connect_info info;
    memset(&info, 0, sizeof(info));
    
    // Simple URL parsing (assuming ws:// or wss://)
    if (url_.substr(0, 5) == "ws://") {
        info.address = url_.substr(5).c_str();
        info.port = 80;
        info.ssl_connection = 0;
    } else if (url_.substr(0, 6) == "wss://") {
        info.address = url_.substr(6).c_str();
        info.port = 443;
        info.ssl_connection = 2; // LCCSCF_USE_SSL
    } else {
        if (onError) {
            onError("Invalid WebSocket URL");
        }
        return false;
    }
    
    info.context = context_;
    info.protocol = websocket_protocols[0].name;
    info.path = "/";
    
    wsi_ = lws_client_connect_via_info(&info);
    if (!wsi_) {
        if (onError) {
            onError("Failed to create WebSocket connection");
        }
        return false;
    }
    
    connected_ = true;
    running_ = true;
    
    // Start service thread
    service_thread_ = std::thread([this]() {
        while (running_) {
            lws_service(context_, 50); // 50ms timeout
        }
    });
    
    return true;
#else
    if (onError) {
        onError("WebSocket support not compiled in (libwebsockets not found)");
    }
    return false;
#endif
}

void WebSocketTransport::disconnect() {
    running_ = false;
    connected_ = false;
    
#ifdef MCP_USE_LIBWEBSOCKETS
    if (wsi_) {
        lws_close_reason(wsi_, LWS_CLOSE_STATUS_NORMAL, (unsigned char*)"Disconnect", 10);
        wsi_ = nullptr;
    }
    
    if (service_thread_.joinable()) {
        service_thread_.join();
    }
#endif
}

bool WebSocketTransport::isConnected() const {
    return connected_ && running_;
}

bool WebSocketTransport::write(const std::string& data) {
    if (!isConnected()) {
        return false;
    }
    
#ifdef MCP_USE_LIBWEBSOCKETS
    if (!wsi_) {
        return false;
    }
    
    // Queue the message for sending
    {
        std::lock_guard<std::mutex> lock(write_queue_mutex_);
        write_queue_.push(data);
    }
    
    // Signal that we have data to send
    lws_callback_on_writable(wsi_);
    
    return true;
#else
    if (onError) {
        onError("WebSocket support not compiled in");
    }
    return false;
#endif
}

std::string WebSocketTransport::read() {
    // For WebSocket transport, reading is handled asynchronously
    // in the callback functions. This method is mainly for compatibility.
    return "";
}

void WebSocketTransport::setMessageCallback(std::function<void(const std::string&)> callback) {
    onMessage = callback;
}

void WebSocketTransport::setErrorCallback(std::function<void(const std::string&)> callback) {
    onError = callback;
}

#ifdef MCP_USE_LIBWEBSOCKETS
// WebSocket protocol callbacks
int WebSocketTransport::websocket_callback(struct lws* wsi, enum lws_callback_reasons reason,
                                          void* user, void* in, size_t len) {
    WebSocketTransport* transport = static_cast<WebSocketTransport*>(user);
    
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            transport->connected_ = true;
            break;
            
        case LWS_CALLBACK_CLIENT_RECEIVE:
            if (transport->onMessage && in && len > 0) {
                std::string message(static_cast<char*>(in), len);
                transport->onMessage(message);
            }
            break;
            
        case LWS_CALLBACK_CLIENT_WRITEABLE: {
            std::lock_guard<std::mutex> lock(transport->write_queue_mutex_);
            if (!transport->write_queue_.empty()) {
                std::string data = transport->write_queue_.front();
                transport->write_queue_.pop();
                
                unsigned char* buf = new unsigned char[LWS_PRE + data.length()];
                memcpy(&buf[LWS_PRE], data.c_str(), data.length());
                
                int ret = lws_write(wsi, &buf[LWS_PRE], data.length(), LWS_WRITE_TEXT);
                delete[] buf;
                
                if (ret < 0) {
                    if (transport->onError) {
                        transport->onError("WebSocket write failed");
                    }
                }
                
                // Schedule another write if there's more data
                if (!transport->write_queue_.empty()) {
                    lws_callback_on_writable(wsi);
                }
            }
            break;
        }
        
        case LWS_CALLBACK_CLIENT_CLOSED:
            transport->connected_ = false;
            break;
            
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            transport->connected_ = false;
            if (transport->onError) {
                transport->onError("WebSocket connection error");
            }
            break;
            
        default:
            break;
    }
    
    return 0;
}

// Static protocol array for libwebsockets
struct lws_protocols WebSocketTransport::websocket_protocols[] = {
    {
        "mcp-websocket",
        &websocket_callback,
        0,
        4096,
    },
    { nullptr, nullptr, 0, 0 }
};
#endif

} // namespace mcp