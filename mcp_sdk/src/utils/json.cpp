#include "mcp/utils/json.hpp"
#include <stdexcept>

namespace mcp {
namespace utils {

nlohmann::json parse(const std::string& jsonStr) {
    try {
        return nlohmann::json::parse(jsonStr);
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error("JSON parse error: " + std::string(e.what()));
    }
}

std::string stringify(const nlohmann::json& json) {
    return json.dump();
}

std::string stringify(const nlohmann::json& json, int indent) {
    return json.dump(indent);
}

std::optional<std::string> getString(const nlohmann::json& json, const std::string& key) {
    if (json.contains(key) && json[key].is_string()) {
        return json[key].get<std::string>();
    }
    return std::nullopt;
}

std::optional<int> getInt(const nlohmann::json& json, const std::string& key) {
    if (json.contains(key) && json[key].is_number_integer()) {
        return json[key].get<int>();
    }
    return std::nullopt;
}

std::optional<double> getDouble(const nlohmann::json& json, const std::string& key) {
    if (json.contains(key) && json[key].is_number()) {
        return json[key].get<double>();
    }
    return std::nullopt;
}

std::optional<bool> getBool(const nlohmann::json& json, const std::string& key) {
    if (json.contains(key) && json[key].is_boolean()) {
        return json[key].get<bool>();
    }
    return std::nullopt;
}

std::optional<nlohmann::json> getObject(const nlohmann::json& json, const std::string& key) {
    if (json.contains(key) && json[key].is_object()) {
        return json[key];
    }
    return std::nullopt;
}

std::optional<nlohmann::json> getArray(const nlohmann::json& json, const std::string& key) {
    if (json.contains(key) && json[key].is_array()) {
        return json[key];
    }
    return std::nullopt;
}

std::string getStringOrDefault(const nlohmann::json& json, const std::string& key, const std::string& defaultValue) {
    auto value = getString(json, key);
    return value.has_value() ? value.value() : defaultValue;
}

int getIntOrDefault(const nlohmann::json& json, const std::string& key, int defaultValue) {
    auto value = getInt(json, key);
    return value.has_value() ? value.value() : defaultValue;
}

double getDoubleOrDefault(const nlohmann::json& json, const std::string& key, double defaultValue) {
    auto value = getDouble(json, key);
    return value.has_value() ? value.value() : defaultValue;
}

bool getBoolOrDefault(const nlohmann::json& json, const std::string& key, bool defaultValue) {
    auto value = getBool(json, key);
    return value.has_value() ? value.value() : defaultValue;
}

nlohmann::json getObjectOrDefault(const nlohmann::json& json, const std::string& key, const nlohmann::json& defaultValue) {
    auto value = getObject(json, key);
    return value.has_value() ? value.value() : defaultValue;
}

nlohmann::json getArrayOrDefault(const nlohmann::json& json, const std::string& key, const nlohmann::json& defaultValue) {
    auto value = getArray(json, key);
    return value.has_value() ? value.value() : defaultValue;
}

bool validateSchema(const nlohmann::json& data, const nlohmann::json& schema) {
    // Basic schema validation implementation
    // For production use, consider using a proper JSON Schema library like nlohmann/json-schema-validator
    
    if (!schema.contains("type")) {
        return true; // No type specified, assume valid
    }
    
    std::string type = schema["type"].get<std::string>();
    
    switch (type[0]) {
        case 's': // string
            if (!data.is_string()) return false;
            break;
        case 'n': // number
            if (!data.is_number()) return false;
            break;
        case 'b': // boolean
            if (!data.is_boolean()) return false;
            break;
        case 'o': // object
            if (!data.is_object()) return false;
            break;
        case 'a': // array
            if (!data.is_array()) return false;
            break;
        case 'n': // null
            if (!data.is_null()) return false;
            break;
    }
    
    // Check required fields for objects
    if (type == "object" && schema.contains("required")) {
        for (const auto& requiredField : schema["required"]) {
            if (!data.contains(requiredField.get<std::string>())) {
                return false;
            }
        }
    }
    
    // Check properties for objects
    if (type == "object" && schema.contains("properties")) {
        for (const auto& [key, value] : schema["properties"].items()) {
            if (data.contains(key)) {
                if (!validateSchema(data[key], value)) {
                    return false;
                }
            }
        }
    }
    
    // Check items for arrays
    if (type == "array" && schema.contains("items")) {
        for (const auto& item : data) {
            if (!validateSchema(item, schema["items"])) {
                return false;
            }
        }
    }
    
    return true;
}

nlohmann::json mergeJson(const nlohmann::json& base, const nlohmann::json& override) {
    nlohmann::json result = base;
    
    for (const auto& [key, value] : override.items()) {
        if (result.contains(key) && result[key].is_object() && value.is_object()) {
            result[key] = mergeJson(result[key], value);
        } else {
            result[key] = value;
        }
    }
    
    return result;
}

std::string jsonPointerGet(const nlohmann::json& json, const std::string& pointer) {
    nlohmann::json current = json;
    
    if (pointer.empty() || pointer == "/") {
        return current.dump();
    }
    
    std::string path = pointer;
    if (path[0] == '/') {
        path = path.substr(1);
    }
    
    std::istringstream iss(path);
    std::string segment;
    
    while (std::getline(iss, segment, '/')) {
        // Unescape JSON Pointer escapes
        std::string unescaped;
        for (size_t i = 0; i < segment.length(); ++i) {
            if (segment[i] == '~') {
                if (i + 1 < segment.length()) {
                    if (segment[i + 1] == '0') {
                        unescaped += '~';
                    } else if (segment[i + 1] == '1') {
                        unescaped += '/';
                    }
                    ++i;
                } else {
                    unescaped += segment[i];
                }
            } else {
                unescaped += segment[i];
            }
        }
        
        if (current.is_object()) {
            if (!current.contains(unescaped)) {
                throw std::runtime_error("JSON Pointer path not found: " + pointer);
            }
            current = current[unescaped];
        } else if (current.is_array()) {
            try {
                size_t index = std::stoul(unescaped);
                if (index >= current.size()) {
                    throw std::runtime_error("JSON Pointer array index out of bounds: " + pointer);
                }
                current = current[index];
            } catch (const std::exception&) {
                throw std::runtime_error("Invalid JSON Pointer array index: " + pointer);
            }
        } else {
            throw std::runtime_error("JSON Pointer path not found: " + pointer);
        }
    }
    
    return current.dump();
}

} // namespace utils
} // namespace mcp