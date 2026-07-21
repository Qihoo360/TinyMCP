#include <cassert>
#include <iostream>
#include "mcp/error.hpp"

int main() {
    std::cout << "Testing MCP Errors..." << std::endl;
    
    // Test MCPError creation
    mcp::MCPError error(mcp::ErrorCode::MethodNotFound, "Test error message");
    assert(error.getCode() == mcp::ErrorCode::MethodNotFound);
    assert(std::string(error.what()) == "Test error message");
    
    // Test error code string conversion
    assert(error.getCodeString() == "MethodNotFound");
    
    // Test error to JSON conversion
    nlohmann::json errorJson = error.toJson();
    assert(errorJson["code"] == static_cast<int>(mcp::ErrorCode::MethodNotFound));
    assert(errorJson["message"] == "Test error message");
    
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
    
    std::cout << "All error tests passed!" << std::endl;
    return 0;
}