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
	UnitTests/TestLegacyCompat.cpp
	IntegrationTests/MemoryTransport.cpp
	IntegrationTests/TestServer.cpp
	IntegrationTests/Driver.cpp
	IntegrationTests/Cases/TestLegacyCase.cpp
	IntegrationTests/Cases/TestSessionCase.cpp
	IntegrationTests/Cases/TestModernCase.cpp
	IntegrationTests/Cases/TestProtocolCase.cpp
	E2ETests/StdioClient.cpp
	E2ETests/ServerMode.cpp
	E2ETests/Cases/TestE2ECase.cpp
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
add_test(NAME unit.legacy_compat COMMAND TinyMCPTests --unit legacy_compat)
set_tests_properties(unit.legacy_compat PROPERTIES TIMEOUT 30)
add_test(NAME integration.legacy COMMAND TinyMCPTests --case legacy)
set_tests_properties(integration.legacy PROPERTIES TIMEOUT 30)
add_test(NAME integration.discover COMMAND TinyMCPTests --case discover)
set_tests_properties(integration.discover PROPERTIES TIMEOUT 30)
add_test(NAME integration.version_fallback COMMAND TinyMCPTests --case version_fallback)
set_tests_properties(integration.version_fallback PROPERTIES TIMEOUT 30)
add_test(NAME integration.version_extensibility COMMAND TinyMCPTests --case version_extensibility)
set_tests_properties(integration.version_extensibility PROPERTIES TIMEOUT 30)
add_test(NAME integration.handshake_version_fallback COMMAND TinyMCPTests --case handshake_version_fallback)
set_tests_properties(integration.handshake_version_fallback PROPERTIES TIMEOUT 30)
add_test(NAME integration.supported_versions COMMAND TinyMCPTests --case supported_versions)
set_tests_properties(integration.supported_versions PROPERTIES TIMEOUT 30)
add_test(NAME integration.modern_core COMMAND TinyMCPTests --case modern_core)
set_tests_properties(integration.modern_core PROPERTIES TIMEOUT 30)
add_test(NAME integration.mrtr COMMAND TinyMCPTests --case mrtr)
set_tests_properties(integration.mrtr PROPERTIES TIMEOUT 30)
add_test(NAME integration.subscriptions COMMAND TinyMCPTests --case subscriptions)
set_tests_properties(integration.subscriptions PROPERTIES TIMEOUT 30)
add_test(NAME integration.apps COMMAND TinyMCPTests --case apps)
set_tests_properties(integration.apps PROPERTIES TIMEOUT 30)
add_test(NAME integration.dual_era COMMAND TinyMCPTests --case dual_era)
set_tests_properties(integration.dual_era PROPERTIES TIMEOUT 30)
add_test(NAME unit.session COMMAND TinyMCPTests --unit session)
set_tests_properties(unit.session PROPERTIES TIMEOUT 30)
add_test(NAME unit.async_cancel COMMAND TinyMCPTests --unit async_cancel)
set_tests_properties(unit.async_cancel PROPERTIES TIMEOUT 30)
add_test(NAME unit.tasks COMMAND TinyMCPTests --unit tasks)
set_tests_properties(unit.tasks PROPERTIES TIMEOUT 30)
add_test(NAME unit.caching COMMAND TinyMCPTests --unit caching)
set_tests_properties(unit.caching PROPERTIES TIMEOUT 30)
add_test(NAME unit.mrtr COMMAND TinyMCPTests --unit mrtr)
set_tests_properties(unit.mrtr PROPERTIES TIMEOUT 30)
add_test(NAME unit.subscriptions COMMAND TinyMCPTests --unit subscriptions)
set_tests_properties(unit.subscriptions PROPERTIES TIMEOUT 30)
add_test(NAME unit.capabilities COMMAND TinyMCPTests --unit capabilities)
set_tests_properties(unit.capabilities PROPERTIES TIMEOUT 30)
add_test(NAME unit.apps COMMAND TinyMCPTests --unit apps)
set_tests_properties(unit.apps PROPERTIES TIMEOUT 30)
add_test(NAME e2e.legacy_stdio COMMAND TinyMCPTests --case e2e.legacy_stdio)
set_tests_properties(e2e.legacy_stdio PROPERTIES TIMEOUT 60)
add_test(NAME e2e.modern_stdio COMMAND TinyMCPTests --case e2e.modern_stdio)
set_tests_properties(e2e.modern_stdio PROPERTIES TIMEOUT 60)
add_test(NAME e2e.dual_era_stdio COMMAND TinyMCPTests --case e2e.dual_era_stdio)
set_tests_properties(e2e.dual_era_stdio PROPERTIES TIMEOUT 60)
