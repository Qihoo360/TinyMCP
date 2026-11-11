#pragma once
// Test host built on top of MCP::CMCPServer. Only the legacy subset of the
// protocol is registered here; feature-specific scenarios extend it later.

#include <Entity/Server.h>
#include <Task/BasicTask.h>

#include <atomic>

namespace MCP
{
	namespace Test
	{
		// Observed by unit.keepalive: the echo tool's "keepalive" action replies
		// from a background thread and keeps using the task afterwards. The
		// "track" action only counts destruction (unit.stop_releases_tasks), and
		// "hang" keeps running in a background thread until it is cancelled.
		struct KeepAliveProbe
		{
			static std::atomic<int> s_nDestroyed;
			static std::atomic<bool> s_bRelease;
			static std::atomic<bool> s_bTouchedAfterResult;
			static std::atomic<bool> s_bHangStarted;
			static std::atomic<bool> s_bCancelRequested;
		};

		class CTestServer : public MCP::CMCPServer<CTestServer>
		{
		public:
			static constexpr const char* SERVER_NAME = "TinyMCPTests";
			static constexpr const char* SERVER_VERSION = "1.0.0";
			static constexpr const char* TOOL_NAME = "echo";
			static constexpr const char* RESOURCE_URI = "test://echo/resource";
			static constexpr const char* MISSING_RESOURCE_URI = "test://echo/missing";
			static constexpr const char* PROMPT_NAME = "echo_prompt";
			static constexpr const char* APP_TOOL_NAME = "echo_app";
			static constexpr const char* UI_RESOURCE_URI = "ui://echo/app.html";
			static constexpr const char* MRTR_RESOURCE_URI = "test://echo/mrtr";

			// Registers the extra modern fixtures (app tool/UI resource/extra read
			// task). Off by default so the legacy registration stays byte-identical.
			static void SetRegisterModernFixtures(bool bRegister);

			int Initialize() override;
			int Uninitialize() override;

			static CTestServer s_Instance;
		};
	}
}
