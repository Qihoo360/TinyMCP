# Basic test setup (can be expanded with GoogleTest later)
add_executable(test_basic test_basic.cpp)
target_link_libraries(test_basic TinyMCP)

add_executable(test_mcp_integration test_mcp_integration.cpp)
target_link_libraries(test_mcp_integration TinyMCP)
add_test(NAME BasicTest COMMAND test_basic)
add_test(NAME MCPIntegrationTest COMMAND test_mcp_integration)
