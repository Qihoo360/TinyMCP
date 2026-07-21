cmake_minimum_required(VERSION 3.20)
project(MCP_SDK VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4 /WX)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Build type options
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find Threads
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files (simplified - only core components)
set(MCP_SDK_SOURCES
    src/core/types_simple.cpp
    src/core/error_simple.cpp
    src/utils/logger_simple.cpp
    src/utils/thread_pool_simple.cpp
)

# Create the main library
add_library(mcp_sdk STATIC ${MCP_SDK_SOURCES})

# Set target properties
set_target_properties(mcp_sdk PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Include directories
target_include_directories(mcp_sdk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link libraries
target_link_libraries(mcp_sdk Threads::Threads)

# Test executable
add_executable(mcp_tests
    tests/test_simple.cpp
)

target_link_libraries(mcp_tests mcp_sdk)

add_test(NAME MCPTests COMMAND mcp_tests)

# Install targets
install(TARGETS mcp_sdk mcp_tests
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install headers
install(DIRECTORY include/ DESTINATION include
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# CPack configuration
set(CPACK_PACKAGE_NAME "MCP-SDK")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Model Context Protocol C++ SDK")
set(CPACK_PACKAGE_VENDOR "TinyMCP Team")
set(CPACK_PACKAGE_CONTACT "contact@tinymcp.org")

include(CPack)