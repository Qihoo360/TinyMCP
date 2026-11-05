#include "../TestHarness.h"

#include <Public/PublicDef.h>

#include <string>

using namespace MCP;

TEST_CASE("unit.constants")
{
	// The legacy version string must stay untouched for existing consumers.
	CHECK_EQ(std::string("2024-11-05"), std::string(PROTOCOL_VER));
	CHECK_EQ(std::string("2024-11-05"), std::string(PROTOCOL_VER_2024_11_05));
	CHECK_EQ(std::string("2026-07-28"), std::string(PROTOCOL_VER_2026_07_28));

	// Modern error codes defined by the 2026-07-28 schema.
	CHECK_EQ(-32020, ERRNO_HEADER_MISMATCH_MODERN);
	CHECK_EQ(-32021, ERRNO_MISSING_REQUIRED_CLIENT_CAPABILITY);
	CHECK_EQ(-32022, ERRNO_UNSUPPORTED_PROTOCOL_VERSION);

	// Legacy error codes must keep their existing values.
	CHECK_EQ(-32001, ERRNO_HEADER_MISMATCH);
	CHECK_EQ(-32002, ERRNO_RESOURCE_NOT_FOUND);

	// Method and metadata names.
	CHECK_EQ(std::string("server/discover"), std::string(METHOD_SERVER_DISCOVER));
	CHECK_EQ(std::string("subscriptions/listen"), std::string(METHOD_SUBSCRIPTIONS_LISTEN));
	CHECK_EQ(std::string("io.modelcontextprotocol/protocolVersion"),
		std::string(META_KEY_PROTOCOL_VERSION));
	CHECK_EQ(std::string("io.modelcontextprotocol/clientCapabilities"),
		std::string(META_KEY_CLIENT_CAPABILITIES));
	CHECK_EQ(std::string("io.modelcontextprotocol/serverInfo"),
		std::string(META_KEY_SERVER_INFO));
	CHECK_EQ(std::string("io.modelcontextprotocol/subscriptionId"),
		std::string(META_KEY_SUBSCRIPTION_ID));

	// Result and content keys introduced by 2026-07-28.
	CHECK_EQ(std::string("resultType"), std::string(MSG_KEY_RESULT_TYPE));
	CHECK_EQ(std::string("inputRequests"), std::string(MSG_KEY_INPUT_REQUESTS));
	CHECK_EQ(std::string("inputResponses"), std::string(MSG_KEY_INPUT_RESPONSES));
	CHECK_EQ(std::string("requestState"), std::string(MSG_KEY_REQUEST_STATE));
	CHECK_EQ(std::string("ttlMs"), std::string(MSG_KEY_TTL_MS));
	CHECK_EQ(std::string("cacheScope"), std::string(MSG_KEY_CACHE_SCOPE));
	CHECK_EQ(std::string("structuredContent"), std::string(MSG_KEY_STRUCTURED_CONTENT));
	CHECK_EQ(std::string("resource_link"), std::string(CONST_RESOURCE_LINK));
	CHECK_EQ(std::string("tool_use"), std::string(CONST_TOOL_USE));
	CHECK_EQ(std::string("tool_result"), std::string(CONST_TOOL_RESULT));
	CHECK_EQ(std::string("audio"), std::string(CONST_AUDIO));

	// MCP Apps extension identifiers.
	CHECK_EQ(std::string("io.modelcontextprotocol/ui"), std::string(EXTENSION_UI));
	CHECK_EQ(std::string("text/html;profile=mcp-app"), std::string(MIME_TYPE_MCP_APP));

	// Result type and cache scope literals from the schema.
	CHECK_EQ(std::string("complete"), std::string(RESULT_TYPE_COMPLETE));
	CHECK_EQ(std::string("input_required"), std::string(RESULT_TYPE_INPUT_REQUIRED));
	CHECK_EQ(std::string("public"), std::string(CACHE_SCOPE_PUBLIC));
	CHECK_EQ(std::string("private"), std::string(CACHE_SCOPE_PRIVATE));
}
