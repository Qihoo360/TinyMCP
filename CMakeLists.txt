cmake_minimum_required(VERSION 3.21)

PROJECT(TinyMCP)

LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
INCLUDE(cmake/ModuleImport.cmake)
INCLUDE(cmake/ModuleCompileOptions.cmake)
ModuleSetCompileOptions()
ModuleSetWinCompilerFlags()

if(WIN32)
    add_compile_definitions(-D_UNICODE)
endif()

LibImport("jsoncpp" "src/jsoncpp")
LibImport("libTinyMCP" "src/libTinyMCP")

ExeImport("Example" "libTinyMCP;jsoncpp")

AddInstall("MCPServer")