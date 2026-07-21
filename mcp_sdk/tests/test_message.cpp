#include <cassert>
#include <iostream>
#include "mcp/message.hpp"
#include "mcp/error.hpp"

int main() {
    std::cout << "Testing MCP Messages..." << std::endl;
    
    // Test Request serialization/deserialization
    mcp::RequestId id;
    id.id = 123;
    
    mcp::Request request(id, "test_method", nlohmann::json{{"param1", "value1"}});
    std::string serialized = request.serialize();
    
    auto deserialized = mcp::Request::deserialize(serialized);
    assert(deserialized->getMethod() == "test_method");
    assert(deserialized->getId().value().toString() == "123");
    assert(deserialized->getParams()["param1"] == "value1");
    
    // Test Response serialization/deserialization
    mcp::Response response(id, nlohmann::json{{"result", "success"}});
    std::string responseSerialized = response.serialize();
    
    auto responseDeserialized = mcp::Response::deserialize(responseSerialized);
    assert(!responseDeserialized->isError());
    assert(responseDeserialized->getResult()["result"] == "success");
    
    // Test Response with error
    mcp::MCPError error(mcp::ErrorCode::MethodNotFound, "Method not found");
    mcp::Response errorResponse(id, error);
    std::string errorSerialized = errorResponse.serialize();
    
    auto errorDeserialized = mcp::Response::deserialize(errorSerialized);
    assert(errorDeserialized->isError());
    assert(errorDeserialized->getError().value().getCode() == mcp::ErrorCode::MethodNotFound);
    
    // Test Notification serialization/deserialization
    mcp::Notification notification("test_notification", nlohmann::json{{"data", "test"}});
    std::string notificationSerialized = notification.serialize();
    
    auto notificationDeserialized = mcp::Notification::deserialize(notificationSerialized);
    assert(notificationDeserialized->getMethod() == "test_notification");
    assert(notificationDeserialized->getParams()["data"] == "test");
    
    std::cout << "All message tests passed!" << std::endl;
    return 0;
}