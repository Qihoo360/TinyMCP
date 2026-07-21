cmake_minimum_required(VERSION 3.10)

project(MCPServer VERSION 1.0.0.1001)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ALL_SRC_FILES "")
file(GLOB_RECURSE EXAMPLE_SRC_FILES "../../Source/*.cpp")
list(APPEND ALL_SRC_FILES ${EXAMPLE_SRC_FILES})
file(GLOB_RECURSE EXTERNAL_SRC_FILES "../../../../Source/External/jsoncpp/src/lib_json/*.cpp")
list(APPEND ALL_SRC_FILES ${EXTERNAL_SRC_FILES})
file(GLOB_RECURSE PROTOCOL_SRC_FILES "../../../../Source/Protocol/*/*.cpp")
list(APPEND ALL_SRC_FILES ${PROTOCOL_SRC_FILES})

add_executable(MCPServer ${ALL_SRC_FILES})

target_include_directories(MCPServer PRIVATE
    ../../Source
    ../../../../Source/Protocol
    ../../../../Source/External/jsoncpp/include
    ../../../../Source/External/cpp-httplib
)

find_package(Threads REQUIRED)
target_link_libraries(MCPServer PRIVATE Threads::Threads)
