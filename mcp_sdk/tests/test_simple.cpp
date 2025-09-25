#include <cassert>
#include <iostream>
#include "mcp/types_simple.hpp"
#include "mcp/error_simple.hpp"

int main() {
    std::cout << "Testing MCP Simple Components..." << std::endl;
    
    // Test SimpleJson
    std::cout << "Testing SimpleJson..." << std::endl;
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
    std::cout << "Testing RequestId..." << std::endl;
    mcp::RequestId id1;
    id1.id = 123;
    assert(id1.toString() == "123");
    
    mcp::RequestId id2;
    id2.id = std::string("test-id");
    assert(id2.toString() == "test-id");
    
    // Test ProgressToken
    std::cout << "Testing ProgressToken..." << std::endl;
    mcp::ProgressToken token1;
    token1.token = 456;
    assert(token1.toString() == "456");
    
    mcp::ProgressToken token2;
    token2.token = std::string("progress-token");
    assert(token2.toString() == "progress-token");
    
    // Test TextContent
    std::cout << "Testing Content types..." << std::endl;
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
    
    // Test MCPError
    std::cout << "Testing MCPError..." << std::endl;
    mcp::MCPError error(mcp::ErrorCode::MethodNotFound, "Test error message");
    assert(error.getCode() == mcp::ErrorCode::MethodNotFound);
    assert(std::string(error.what()) == "Test error message");
    
    // Test error code string conversion
    assert(error.getCodeString() == "MethodNotFound");
    
    // Test error to JSON conversion
    mcp::SimpleJson errorJson = error.toJson();
    assert(errorJson["code"].getNumber() == static_cast<double>(mcp::ErrorCode::MethodNotFound));
    assert(errorJson["message"].getString() == "Test error message");
    
    // Test error code conversion functions
    assert(mcp::errorCodeFromInt(-32601) == mcp::ErrorCode::MethodNotFound);
    assert(mcp::errorCodeToInt(mcp::ErrorCode::MethodNotFound) == -32601);
    
    // Test standard JSON-RPC error codes
    assert(mcp::JSON_RPC_PARSE_ERROR == -32700);
    assert(mcp::JSON_RPC_INVALID_REQUEST == -32600);
    assert(mcp::JSON_RPC_METHOD_NOT_FOUND == -32601);
    assert(mcp::JSON_RPC_INVALID_PARAMS == -32602);
    assert(mcp::JSON_RPC_INTERNAL_ERROR == -32603);
    
    // Test MCP-specific error codes
    assert(mcp::MCP_INVALID_METHOD == -32604);
    assert(mcp::MCP_INVALID_PARAMS == -32605);
    assert(mcp::MCP_RESOURCE_NOT_FOUND == -32606);
    assert(mcp::MCP_RESOURCE_NOT_AVAILABLE == -32607);
    assert(mcp::MCP_RESOURCE_EXHAUSTED == -32608);
    assert(mcp::MCP_CONTENT_MODIFIED == -32609);
    
    std::cout << "All simple tests passed!" << std::endl;
    return 0;
}