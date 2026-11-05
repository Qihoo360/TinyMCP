cmake_minimum_required(VERSION 3.10)
project(TinyMCPTests CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

set(TINYMCP_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

file(GLOB_RECURSE TINYMCP_PROTOCOL_SOURCES "${TINYMCP_ROOT_DIR}/Source/Protocol/*.cpp")
list(FILTER TINYMCP_PROTOCOL_SOURCES EXCLUDE REGEX "SSETransport\\.cpp$")

file(GLOB TINYMCP_JSONCPP_SOURCES "${TINYMCP_ROOT_DIR}/Source/External/jsoncpp/src/lib_json/*.cpp")

add_executable(TinyMCPTests
	TestMain.cpp
	UnitTests/TestHarnessSelfCheck.cpp
	UnitTests/TestConstants.cpp
	UnitTests/TestVersionRegistry.cpp
	UnitTests/TestBasicMessage.cpp
	UnitTests/TestRequest.cpp
	UnitTests/TestResponse.cpp
	UnitTests/TestNotification.cpp
	IntegrationTests/MemoryTransport.cpp
	IntegrationTests/TestServer.cpp
	IntegrationTests/Driver.cpp
	IntegrationTests/Cases/TestLegacyCase.cpp
	IntegrationTests/Cases/TestSessionCase.cpp
	IntegrationTests/Cases/TestModernCase.cpp
	${TINYMCP_PROTOCOL_SOURCES}
	${TINYMCP_JSONCPP_SOURCES}
)

target_include_directories(TinyMCPTests PRIVATE
	"${TINYMCP_ROOT_DIR}/Source/Protocol"
	"${TINYMCP_ROOT_DIR}/Source/External/jsoncpp/include"
)

target_compile_definitions(TinyMCPTests PRIVATE
	TINYMCP_GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/IntegrationTests/Golden/legacy"
)

target_link_libraries(TinyMCPTests PRIVATE Threads::Threads)

enable_testing()

add_test(NAME unit.all COMMAND TinyMCPTests --unit)
add_test(NAME unit.constants COMMAND TinyMCPTests --unit constants)
add_test(NAME unit.version_registry COMMAND TinyMCPTests --unit version_registry)
add_test(NAME unit.message COMMAND TinyMCPTests --unit message)
add_test(NAME unit.content_blocks COMMAND TinyMCPTests --unit content_blocks)
add_test(NAME unit.meta COMMAND TinyMCPTests --unit meta)
add_test(NAME unit.request COMMAND TinyMCPTests --unit request)
add_test(NAME unit.response COMMAND TinyMCPTests --unit response)
add_test(NAME unit.notification COMMAND TinyMCPTests --unit notification)
add_test(NAME integration.legacy COMMAND TinyMCPTests --case legacy)
set_tests_properties(integration.legacy PROPERTIES TIMEOUT 30)
add_test(NAME unit.session COMMAND TinyMCPTests --unit session)
set_tests_properties(unit.session PROPERTIES TIMEOUT 30)
add_test(NAME unit.tasks COMMAND TinyMCPTests --unit tasks)
set_tests_properties(unit.tasks PROPERTIES TIMEOUT 30)
add_test(NAME unit.caching COMMAND TinyMCPTests --unit caching)
set_tests_properties(unit.caching PROPERTIES TIMEOUT 30)
add_test(NAME unit.mrtr COMMAND TinyMCPTests --unit mrtr)
set_tests_properties(unit.mrtr PROPERTIES TIMEOUT 30)
add_test(NAME unit.subscriptions COMMAND TinyMCPTests --unit subscriptions)
set_tests_properties(unit.subscriptions PROPERTIES TIMEOUT 30)
