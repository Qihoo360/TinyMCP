#include <cassert>
#include <iostream>
#include "mcp/types.hpp"

int main() {
    std::cout << "Testing MCP Types..." << std::endl;
    
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
    
    std::cout << "All type tests passed!" << std::endl;
    return 0;
}