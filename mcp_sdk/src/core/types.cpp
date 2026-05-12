#include "mcp/types.hpp"
#include <sstream>

namespace mcp {

std::string RequestId::toString() const {
    if (std::holds_alternative<int>(id)) {
        return std::to_string(std::get<int>(id));
    } else if (std::holds_alternative<std::string>(id)) {
        return std::get<std::string>(id);
    }
    return "null";
}

bool RequestId::operator==(const RequestId& other) const {
    if (id.index() != other.id.index()) {
        return false;
    }
    
    if (std::holds_alternative<int>(id)) {
        return std::get<int>(id) == std::get<int>(other.id);
    } else if (std::holds_alternative<std::string>(id)) {
        return std::get<std::string>(id) == std::get<std::string>(other.id);
    }
    
    return true; // both null
}

bool RequestId::operator<(const RequestId& other) const {
    if (id.index() != other.id.index()) {
        return id.index() < other.id.index();
    }
    
    if (std::holds_alternative<int>(id)) {
        return std::get<int>(id) < std::get<int>(other.id);
    } else if (std::holds_alternative<std::string>(id)) {
        return std::get<std::string>(id) < std::get<std::string>(other.id);
    }
    
    return false; // both null
}

std::string ProgressToken::toString() const {
    if (std::holds_alternative<int>(token)) {
        return std::to_string(std::get<int>(token));
    } else if (std::holds_alternative<std::string>(token)) {
        return std::get<std::string>(token);
    }
    return "null";
}

std::string TextContent::getText() const {
    return text;
}

std::string ImageContent::getDataUri() const {
    std::ostringstream oss;
    oss << "data:" << mimeType;
    if (!data.empty()) {
        oss << ";base64," << data;
    }
    return oss.str();
}

std::string EmbeddedResource::getUri() const {
    return uri;
}

std::string TextResourceContents::getText() const {
    return text;
}

std::string BlobResourceContents::getDataUri() const {
    std::ostringstream oss;
    oss << "data:" << mimeType;
    if (!blob.empty()) {
        oss << ";base64," << blob;
    }
    return oss.str();
}

} // namespace mcp