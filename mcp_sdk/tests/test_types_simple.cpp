#include <cassert>
#include <iostream>
#include "mcp/types_simple.hpp"

int main() {
    std::cout << "Testing MCP Simple Types..." << std::endl;
    
    // Test SimpleJson
    mcp::SimpleJson json;
    assert(json.isNull());
    
    json = mcp::SimpleJson(true);
    assert(json.isBool());
    assert(json.getBool() == true);
    
    json = mcp::SimpleJson(42.5);
    assert(json.isNumber());
    assert(json.getNumber() == 42.5);
    
    json = mcp::SimpleJson("test string");
    assert(json.isString());
    assert(json.getString() == "test string");
    
    // Test array
    std::vector<mcp::SimpleJson> arr = {mcp::SimpleJson(1), mcp::SimpleJson(2), mcp::SimpleJson(3)};
    json = mcp::SimpleJson(arr);
    assert(json.isArray());
    assert(json.size() == 3);
    assert(json[0].getNumber() == 1);
    assert(json[1].getNumber() == 2);
    assert(json[2].getNumber() == 3);
    
    // Test object
    std::map<std::string, mcp::SimpleJson> obj;
    obj["key1"] = mcp::SimpleJson("value1");
    obj["key2"] = mcp::SimpleJson(123);
    json = mcp::SimpleJson(obj);
    assert(json.isObject());
    assert(json.contains("key1"));
    assert(json.contains("key2"));
    assert(json["key1"].getString() == "value1");
    assert(json["key2"].getNumber() == 123);
    
    // Test RequestId
    mcp::RequestId id1;
    id1.id = 123;
    assert(id1.toString() == "123");
    
    mcp::RequestId id2;
    id2.id = std::string("test-id");
    assert(id2.toString() == "test-id");
    
    // Test ProgressToken
    mcp::ProgressToken token1;
    token1.token = 456;
    assert(token1.toString() == "456");
    
    mcp::ProgressToken token2;
    token2.token = std::string("progress-token");
    assert(token2.toString() == "progress-token");
    
    // Test TextContent
    mcp::TextContent text("Hello, World!");
    assert(text.getText() == "Hello, World!");
    
    // Test ImageContent
    mcp::ImageContent image("image/png", "base64data");
    std::string dataUri = image.getDataUri();
    assert(dataUri.find("data:image/png;base64,base64data") != std::string::npos);
    
    // Test EmbeddedResource
    mcp::EmbeddedResource resource("file:///path/to/resource");
    assert(resource.getUri() == "file:///path/to/resource");
    
    // Test TextResourceContents
    mcp::TextResourceContents textResource("Resource content");
    assert(textResource.getText() == "Resource content");
    
    // Test BlobResourceContents
    mcp::BlobResourceContents blobResource("application/octet-stream", "binarydata");
    std::string blobDataUri = blobResource.getDataUri();
    assert(blobDataUri.find("data:application/octet-stream;base64,binarydata") != std::string::npos);
    
    std::cout << "All simple type tests passed!" << std::endl;
    return 0;
}