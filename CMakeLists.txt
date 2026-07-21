cmake_minimum_required(VERSION 3.20)
project(TinyMCP VERSION 1.0.0 LANGUAGES CXX)

# Use Clang as the compiler
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_COMPILER clang)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for optimization and warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
find_package(CURL REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${JSONCPP_INCLUDE_DIRS})

# Source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.hpp"
)

# Create main library
add_library(TinyMCP STATIC ${SOURCES})

# Link libraries
target_link_libraries(TinyMCP 
    ${JSONCPP_LIBRARIES}
    ${CURL_LIBRARIES}
    pthread
)

# Compile definitions
target_compile_definitions(TinyMCP PRIVATE ${JSONCPP_CFLAGS_OTHER})

# Example executables
add_executable(download_cli src/Examples/DownloadCLI.cpp)
target_link_libraries(download_cli TinyMCP)

add_executable(mcp_manager_cli src/Examples/MCPServerManagerCLI.cpp)
target_link_libraries(mcp_manager_cli TinyMCP)

# Install targets
install(TARGETS TinyMCP download_cli mcp_manager_cli
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install headers
install(DIRECTORY src/ DESTINATION include/TinyMCP
    FILES_MATCHING PATTERN "*.hpp"
)

# Testing
enable_testing()
add_subdirectory(tests OPTIONAL)
