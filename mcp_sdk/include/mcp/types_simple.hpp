#pragma once
#include <string>
#include <variant>
#include <optional>
#include <vector>
#include <map>

namespace mcp {

// Simple JSON-like structure for basic functionality
struct SimpleJson {
    enum Type { Null, Bool, Number, String, Array, Object };
    Type type = Null;
    
    std::variant<
        std::nullptr_t,
        bool,
        double,
        std::string,
        std::vector<SimpleJson>,
        std::map<std::string, SimpleJson>
    > value;
    
    SimpleJson() : type(Null), value(nullptr) {}
    SimpleJson(const SimpleJson& other) : type(other.type), value(other.value) {}
    explicit SimpleJson(bool b) : type(Bool), value(b) {}
    explicit SimpleJson(double d) : type(Number), value(d) {}
    explicit SimpleJson(int i) : type(Number), value(static_cast<double>(i)) {}
    explicit SimpleJson(const std::string& s) : type(String), value(s) {}
    explicit SimpleJson(const std::vector<SimpleJson>& arr) : type(Array), value(arr) {}
    explicit SimpleJson(const std::map<std::string, SimpleJson>& obj) : type(Object), value(obj) {}
    
    bool isNull() const { return type == Null; }
    bool isBool() const { return type == Bool; }
    bool isNumber() const { return type == Number; }
    bool isString() const { return type == String; }
    bool isArray() const { return type == Array; }
    bool isObject() const { return type == Object; }
    
    bool getBool() const { return std::get<bool>(value); }
    double getNumber() const { return std::get<double>(value); }
    const std::string& getString() const { return std::get<std::string>(value); }
    const std::vector<SimpleJson>& getArray() const { return std::get<std::vector<SimpleJson>>(value); }
    const std::map<std::string, SimpleJson>& getObject() const { return std::get<std::map<std::string, SimpleJson>>(value); }
    
    SimpleJson& operator[](const std::string& key) {
        if (type != Object) {
            type = Object;
            value = std::map<std::string, SimpleJson>();
        }
        return std::get<std::map<std::string, SimpleJson>>(value)[key];
    }
    
    // Assignment operators
    SimpleJson& operator=(const SimpleJson& other) {
        if (this != &other) {
            type = other.type;
            value = other.value;
        }
        return *this;
    }
    
    SimpleJson& operator=(bool b) {
        type = Bool;
        value = b;
        return *this;
    }
    
    SimpleJson& operator=(double d) {
        type = Number;
        value = d;
        return *this;
    }
    
    SimpleJson& operator=(int i) {
        type = Number;
        value = static_cast<double>(i);
        return *this;
    }
    
    SimpleJson& operator=(const std::string& s) {
        type = String;
        value = s;
        return *this;
    }
    
    const SimpleJson& operator[](const std::string& key) const {
        static SimpleJson nullJson;
        if (type == Object) {
            const auto& obj = std::get<std::map<std::string, SimpleJson>>(value);
            auto it = obj.find(key);
            if (it != obj.end()) {
                return it->second;
            }
        }
        return nullJson;
    }
    
    SimpleJson& operator[](size_t index) {
        if (type != Array) {
            type = Array;
            value = std::vector<SimpleJson>();
        }
        auto& arr = std::get<std::vector<SimpleJson>>(value);
        if (index >= arr.size()) {
            arr.resize(index + 1);
        }
        return arr[index];
    }
    
    const SimpleJson& operator[](size_t index) const {
        static SimpleJson nullJson;
        if (type == Array) {
            const auto& arr = std::get<std::vector<SimpleJson>>(value);
            if (index < arr.size()) {
                return arr[index];
            }
        }
        return nullJson;
    }
    
    size_t size() const {
        if (type == Array) return std::get<std::vector<SimpleJson>>(value).size();
        if (type == Object) return std::get<std::map<std::string, SimpleJson>>(value).size();
        return 0;
    }
    
    bool contains(const std::string& key) const {
        if (type == Object) {
            const auto& obj = std::get<std::map<std::string, SimpleJson>>(value);
            return obj.find(key) != obj.end();
        }
        return false;
    }
    
    std::string dump(int indent = 0) const;
};

// Request ID
struct RequestId {
    std::variant<std::nullptr_t, int, std::string> id;
    
    RequestId() : id(nullptr) {}
    RequestId(int i) : id(i) {}
    RequestId(const std::string& s) : id(s) {}
    
    std::string toString() const;
    bool operator==(const RequestId& other) const;
    bool operator<(const RequestId& other) const;
};

// Progress Token
struct ProgressToken {
    std::variant<std::nullptr_t, int, std::string> token;
    
    ProgressToken() : token(nullptr) {}
    ProgressToken(int i) : token(i) {}
    ProgressToken(const std::string& s) : token(s) {}
    
    std::string toString() const;
};

// Client and Server Info
struct ClientInfo {
    std::string name;
    std::string version;
};

struct ServerInfo {
    std::string name;
    std::string version;
};

// Content types
struct TextContent {
    std::string text;
    
    TextContent() = default;
    TextContent(const std::string& t) : text(t) {}
    
    std::string getText() const;
};

struct ImageContent {
    std::string mimeType;
    std::string data;
    
    ImageContent() = default;
    ImageContent(const std::string& mime, const std::string& d) : mimeType(mime), data(d) {}
    
    std::string getDataUri() const;
};

struct EmbeddedResource {
    std::string uri;
    
    EmbeddedResource() = default;
    EmbeddedResource(const std::string& u) : uri(u) {}
    
    std::string getUri() const;
};

struct TextResourceContents {
    std::string text;
    
    TextResourceContents() = default;
    TextResourceContents(const std::string& t) : text(t) {}
    
    std::string getText() const;
};

struct BlobResourceContents {
    std::string mimeType;
    std::string blob;
    
    BlobResourceContents() = default;
    BlobResourceContents(const std::string& mime, const std::string& b) : mimeType(mime), blob(b) {}
    
    std::string getDataUri() const;
};

} // namespace mcp