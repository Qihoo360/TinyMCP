#pragma once

#include "types.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <iostream>

namespace mcp {
namespace utils {

/**
 * @brief JSON utility functions for MCP
 */
class JsonUtils {
public:
    /**
     * @brief Serialize JsonValue to JSON string
     */
    static std::string serialize(const JsonValue& value);
    
    /**
     * @brief Deserialize JSON string to JsonValue
     */
    static JsonValue deserialize(const std::string& json);
    
    /**
     * @brief Pretty print JsonValue
     */
    static std::string prettyPrint(const JsonValue& value, int indent = 2);
    
    /**
     * @brief Validate JSON string
     */
    static bool isValidJson(const std::string& json);
    
    /**
     * @brief Get JSON value type
     */
    static std::string getType(const JsonValue& value);
    
    /**
     * @brief Check if JsonValue is null
     */
    static bool isNull(const JsonValue& value);
    
    /**
     * @brief Check if JsonValue is string
     */
    static bool isString(const JsonValue& value);
    
    /**
     * @brief Check if JsonValue is number
     */
    static bool isNumber(const JsonValue& value);
    
    /**
     * @brief Check if JsonValue is boolean
     */
    static bool isBoolean(const JsonValue& value);
    
    /**
     * @brief Check if JsonValue is array
     */
    static bool isArray(const JsonValue& value);
    
    /**
     * @brief Check if JsonValue is object
     */
    static bool isObject(const JsonValue& value);
    
    /**
     * @brief Get string value
     */
    static std::string getString(const JsonValue& value);
    
    /**
     * @brief Get number value (as double)
     */
    static double getNumber(const JsonValue& value);
    
    /**
     * @brief Get integer value
     */
    static int64_t getInteger(const JsonValue& value);
    
    /**
     * @brief Get boolean value
     */
    static bool getBoolean(const JsonValue& value);
    
    /**
     * @brief Get array value
     */
    static std::vector<JsonValue> getArray(const JsonValue& value);
    
    /**
     * @brief Get object value
     */
    static std::unordered_map<std::string, JsonValue> getObject(const JsonValue& value);
    
    /**
     * @brief Get object field value
     */
    static std::optional<JsonValue> getField(const JsonValue& value, const std::string& field);
    
    /**
     * @brief Set object field value
     */
    static void setField(std::unordered_map<std::string, JsonValue>& object, 
                        const std::string& field, const JsonValue& value);
    
    /**
     * @brief Remove object field
     */
    static void removeField(std::unordered_map<std::string, JsonValue>& object, 
                           const std::string& field);
    
    /**
     * @brief Check if object has field
     */
    static bool hasField(const JsonValue& value, const std::string& field);
    
    /**
     * @brief Get array size
     */
    static size_t getArraySize(const JsonValue& value);
    
    /**
     * @brief Get array element
     */
    static std::optional<JsonValue> getArrayElement(const JsonValue& value, size_t index);
    
    /**
     * @brief Set array element
     */
    static void setArrayElement(std::vector<JsonValue>& array, size_t index, const JsonValue& value);
    
    /**
     * @brief Add array element
     */
    static void addArrayElement(std::vector<JsonValue>& array, const JsonValue& value);
    
    /**
     * @brief Remove array element
     */
    static void removeArrayElement(std::vector<JsonValue>& array, size_t index);
    
    /**
     * @brief Deep copy JsonValue
     */
    static JsonValue deepCopy(const JsonValue& value);
    
    /**
     * @brief Merge two JsonValue objects
     */
    static JsonValue merge(const JsonValue& base, const JsonValue& override);
    
    /**
     * @brief Convert JsonValue to string representation
     */
    static std::string toString(const JsonValue& value);
    
    /**
     * @brief Create JsonValue from string
     */
    static JsonValue fromString(const std::string& str);
    
    /**
     * @brief Create JsonValue from number
     */
    static JsonValue fromNumber(double num);
    
    /**
     * @brief Create JsonValue from integer
     */
    static JsonValue fromInteger(int64_t num);
    
    /**
     * @brief Create JsonValue from boolean
     */
    static JsonValue fromBoolean(bool value);
    
    /**
     * @brief Create JsonValue from array
     */
    static JsonValue fromArray(const std::vector<JsonValue>& array);
    
    /**
     * @brief Create JsonValue from object
     */
    static JsonValue fromObject(const std::unordered_map<std::string, JsonValue>& object);
    
    /**
     * @brief Create null JsonValue
     */
    static JsonValue null();
    
    /**
     * @brief Compare two JsonValues for equality
     */
    static bool equals(const JsonValue& a, const JsonValue& b);
    
    /**
     * @brief Get JSON schema for JsonValue
     */
    static std::unordered_map<std::string, JsonValue> getSchema(const JsonValue& value);
    
    /**
     * @brief Validate JsonValue against schema
     */
    static bool validateSchema(const JsonValue& value, const std::unordered_map<std::string, JsonValue>& schema);
    
    /**
     * @brief Escape JSON string
     */
    static std::string escapeString(const std::string& str);
    
    /**
     * @brief Unescape JSON string
     */
    static std::string unescapeString(const std::string& str);
    
    /**
     * @brief Parse JSON from stream
     */
    static JsonValue parseFromStream(std::istream& stream);
    
    /**
     * @brief Write JSON to stream
     */
    static void writeToStream(std::ostream& stream, const JsonValue& value, bool pretty = false);
    
    /**
     * @brief Parse JSON from file
     */
    static JsonValue parseFromFile(const std::string& filename);
    
    /**
     * @brief Write JSON to file
     */
    static void writeToFile(const std::string& filename, const JsonValue& value, bool pretty = false);
};

/**
 * @brief JSON schema validator
 */
class JsonSchemaValidator {
private:
    std::unordered_map<std::string, std::unordered_map<std::string, JsonValue>> schemas_;
    
public:
    /**
     * @brief Add schema
     */
    void addSchema(const std::string& name, const std::unordered_map<std::string, JsonValue>& schema);
    
    /**
     * @brief Remove schema
     */
    void removeSchema(const std::string& name);
    
    /**
     * @brief Validate value against schema
     */
    bool validate(const JsonValue& value, const std::string& schemaName);
    
    /**
     * @brief Validate value against schema object
     */
    bool validate(const JsonValue& value, const std::unordered_map<std::string, JsonValue>& schema);
    
    /**
     * @brief Get validation errors
     */
    std::vector<std::string> getValidationErrors(const JsonValue& value, const std::unordered_map<std::string, JsonValue>& schema);
    
    /**
     * @brief Check if schema exists
     */
    bool hasSchema(const std::string& name) const;
    
    /**
     * @brief Get schema
     */
    std::optional<std::unordered_map<std::string, JsonValue>> getSchema(const std::string& name) const;
};

/**
 * @brief JSON path for accessing nested values
 */
class JsonPath {
private:
    std::vector<std::string> path_;
    
public:
    JsonPath(const std::string& path);
    JsonPath(const std::vector<std::string>& path);
    
    /**
     * @brief Get value at path
     */
    std::optional<JsonValue> getValue(const JsonValue& root) const;
    
    /**
     * @brief Set value at path
     */
    void setValue(std::unordered_map<std::string, JsonValue>& root, const JsonValue& value);
    
    /**
     * @brief Check if path exists
     */
    bool exists(const JsonValue& root) const;
    
    /**
     * @brief Remove value at path
     */
    void remove(std::unordered_map<std::string, JsonValue>& root);
    
    /**
     * @brief Get path as string
     */
    std::string toString() const;
    
    /**
     * @brief Get path components
     */
    const std::vector<std::string>& getComponents() const;
    
private:
    void parsePath(const std::string& path);
};

/**
 * @brief JSON pointer (RFC 6901)
 */
class JsonPointer {
public:
    /**
     * @brief Resolve JSON pointer
     */
    static std::optional<JsonValue> resolve(const JsonValue& root, const std::string& pointer);
    
    /**
     * @brief Set value at JSON pointer
     */
    static void setValue(std::unordered_map<std::string, JsonValue>& root, const std::string& pointer, const JsonValue& value);
    
    /**
     * @brief Remove value at JSON pointer
     */
    static void remove(std::unordered_map<std::string, JsonValue>& root, const std::string& pointer);
    
    /**
     * @brief Escape JSON pointer reference
     */
    static std::string escape(const std::string& reference);
    
    /**
     * @brief Unescape JSON pointer reference
     */
    static std::string unescape(const std::string& reference);
};

} // namespace utils
} // namespace mcp