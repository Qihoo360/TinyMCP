#include "mcp/message.hpp"
#include "mcp/error.hpp"
#include <sstream>
#include <chrono>

namespace mcp {

// Base Message implementation
Message::Message() : id_(std::nullopt) {}

Message::Message(const RequestId& id) : id_(id) {}

std::optional<RequestId> Message::getId() const {
    return id_;
}

void Message::setId(const RequestId& id) {
    id_ = id;
}

std::string Message::getJsonRpcVersion() const {
    return "2.0";
}

// Request implementation
Request::Request() : Message() {}

Request::Request(const RequestId& id, const std::string& method, const nlohmann::json& params)
    : Message(id), method_(method), params_(params) {}

std::string Request::getMethod() const {
    return method_;
}

void Request::setMethod(const std::string& method) {
    method_ = method;
}

nlohmann::json Request::getParams() const {
    return params_;
}

void Request::setParams(const nlohmann::json& params) {
    params_ = params;
}

std::string Request::serialize() const {
    nlohmann::json json;
    json["jsonrpc"] = getJsonRpcVersion();
    json["method"] = method_;
    
    if (id_.has_value()) {
        if (std::holds_alternative<int>(id_.value().id)) {
            json["id"] = std::get<int>(id_.value().id);
        } else if (std::holds_alternative<std::string>(id_.value().id)) {
            json["id"] = std::get<std::string>(id_.value().id);
        }
    }
    
    if (!params_.is_null()) {
        json["params"] = params_;
    }
    
    return json.dump();
}

std::unique_ptr<Request> Request::deserialize(const std::string& jsonStr) {
    try {
        nlohmann::json json = nlohmann::json::parse(jsonStr);
        
        if (!json.contains("jsonrpc") || json["jsonrpc"] != "2.0") {
            throw MCPError(ErrorCode::InvalidRequest, "Invalid JSON-RPC version");
        }
        
        if (!json.contains("method")) {
            throw MCPError(ErrorCode::InvalidRequest, "Missing method");
        }
        
        RequestId id;
        if (json.contains("id")) {
            if (json["id"].is_number_integer()) {
                id.id = json["id"].get<int>();
            } else if (json["id"].is_string()) {
                id.id = json["id"].get<std::string>();
            } else {
                throw MCPError(ErrorCode::InvalidRequest, "Invalid id type");
            }
        }
        
        std::string method = json["method"].get<std::string>();
        nlohmann::json params = json.contains("params") ? json["params"] : nlohmann::json::object();
        
        return std::make_unique<Request>(id, method, params);
    } catch (const nlohmann::json::exception& e) {
        throw MCPError(ErrorCode::ParseError, "Failed to parse JSON: " + std::string(e.what()));
    }
}

// Response implementation
Response::Response() : Message(), result_(nlohmann::json::object()) {}

Response::Response(const RequestId& id, const nlohmann::json& result)
    : Message(id), result_(result) {}

Response::Response(const RequestId& id, const MCPError& error)
    : Message(id), result_(nlohmann::json::object()), error_(error) {}

nlohmann::json Response::getResult() const {
    return result_;
}

void Response::setResult(const nlohmann::json& result) {
    result_ = result;
    error_.reset();
}

std::optional<MCPError> Response::getError() const {
    return error_;
}

void Response::setError(const MCPError& error) {
    error_ = error;
    result_ = nlohmann::json::object();
}

bool Response::isError() const {
    return error_.has_value();
}

std::string Response::serialize() const {
    nlohmann::json json;
    json["jsonrpc"] = getJsonRpcVersion();
    
    if (id_.has_value()) {
        if (std::holds_alternative<int>(id_.value().id)) {
            json["id"] = std::get<int>(id_.value().id);
        } else if (std::holds_alternative<std::string>(id_.value().id)) {
            json["id"] = std::get<std::string>(id_.value().id);
        }
    }
    
    if (error_.has_value()) {
        json["error"] = error_.value().toJson();
    } else {
        json["result"] = result_;
    }
    
    return json.dump();
}

std::unique_ptr<Response> Response::deserialize(const std::string& jsonStr) {
    try {
        nlohmann::json json = nlohmann::json::parse(jsonStr);
        
        if (!json.contains("jsonrpc") || json["jsonrpc"] != "2.0") {
            throw MCPError(ErrorCode::InvalidRequest, "Invalid JSON-RPC version");
        }
        
        RequestId id;
        if (json.contains("id")) {
            if (json["id"].is_number_integer()) {
                id.id = json["id"].get<int>();
            } else if (json["id"].is_string()) {
                id.id = json["id"].get<std::string>();
            }
        }
        
        if (json.contains("error")) {
            nlohmann::json errorJson = json["error"];
            ErrorCode code = errorCodeFromInt(errorJson["code"].get<int>());
            std::string message = errorJson.contains("message") ? errorJson["message"].get<std::string>() : "";
            nlohmann::json data = errorJson.contains("data") ? errorJson["data"] : nlohmann::json::object();
            
            MCPError error(code, message, data);
            return std::make_unique<Response>(id, error);
        } else if (json.contains("result")) {
            return std::make_unique<Response>(id, json["result"]);
        } else {
            throw MCPError(ErrorCode::InvalidRequest, "Missing result or error");
        }
    } catch (const nlohmann::json::exception& e) {
        throw MCPError(ErrorCode::ParseError, "Failed to parse JSON: " + std::string(e.what()));
    }
}

// Notification implementation
Notification::Notification() : Message() {}

Notification::Notification(const std::string& method, const nlohmann::json& params)
    : Message(), method_(method), params_(params) {}

std::string Notification::getMethod() const {
    return method_;
}

void Notification::setMethod(const std::string& method) {
    method_ = method;
}

nlohmann::json Notification::getParams() const {
    return params_;
}

void Notification::setParams(const nlohmann::json& params) {
    params_ = params;
}

std::string Notification::serialize() const {
    nlohmann::json json;
    json["jsonrpc"] = getJsonRpcVersion();
    json["method"] = method_;
    
    if (!params_.is_null()) {
        json["params"] = params_;
    }
    
    return json.dump();
}

std::unique_ptr<Notification> Notification::deserialize(const std::string& jsonStr) {
    try {
        nlohmann::json json = nlohmann::json::parse(jsonStr);
        
        if (!json.contains("jsonrpc") || json["jsonrpc"] != "2.0") {
            throw MCPError(ErrorCode::InvalidRequest, "Invalid JSON-RPC version");
        }
        
        if (!json.contains("method")) {
            throw MCPError(ErrorCode::InvalidRequest, "Missing method");
        }
        
        if (json.contains("id")) {
            throw MCPError(ErrorCode::InvalidRequest, "Notifications cannot have id");
        }
        
        std::string method = json["method"].get<std::string>();
        nlohmann::json params = json.contains("params") ? json["params"] : nlohmann::json::object();
        
        return std::make_unique<Notification>(method, params);
    } catch (const nlohmann::json::exception& e) {
        throw MCPError(ErrorCode::ParseError, "Failed to parse JSON: " + std::string(e.what()));
    }
}

} // namespace mcp