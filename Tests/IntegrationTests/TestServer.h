#pragma once
// Test host built on top of MCP::CMCPServer. Only the legacy subset of the
// protocol is registered here; feature-specific scenarios extend it later.

#include <Entity/Server.h>
#include <Task/BasicTask.h>

namespace MCP
{
	namespace Test
	{
		class CTestServer : public MCP::CMCPServer<CTestServer>
		{
		public:
			static constexpr const char* SERVER_NAME = "TinyMCPTests";
			static constexpr const char* SERVER_VERSION = "1.0.0";
			static constexpr const char* TOOL_NAME = "echo";
			static constexpr const char* RESOURCE_URI = "test://echo/resource";
			static constexpr const char* MISSING_RESOURCE_URI = "test://echo/missing";
			static constexpr const char* PROMPT_NAME = "echo_prompt";

			int Initialize() override;
			int Uninitialize() override;

			static CTestServer s_Instance;
		};
	}
}
