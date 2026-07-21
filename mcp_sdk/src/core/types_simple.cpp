#include "mcp/types_simple.hpp"
#include <sstream>
#include <iomanip>

namespace mcp {

std::string SimpleJson::dump(int indent) const {
    std::ostringstream oss;
    
    switch (type) {
        case Null:
            oss << "null";
            break;
        case Bool:
            oss << (std::get<bool>(value) ? "true" : "false");
            break;
        case Number:
            oss << std::get<double>(value);
            break;
        case String:
            oss << "\"" << std::get<std::string>(value) << "\"";
            break;
        case Array: {
            const auto& arr = std::get<std::vector<SimpleJson>>(value);
            oss << "[";
            for (size_t i = 0; i < arr.size(); ++i) {
                if (i > 0) oss << ",";
                oss << arr[i].dump(indent + 2);
            }
            oss << "]";
            break;
        }
        case Object: {
            const auto& obj = std::get<std::map<std::string, SimpleJson>>(value);
            oss << "{";
            bool first = true;
            for (const auto& [key, val] : obj) {
                if (!first) oss << ",";
                oss << "\"" << key << "\":" << val.dump(indent + 2);
                first = false;
            }
            oss << "}";
            break;
        }
    }
    
    return oss.str();
}

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