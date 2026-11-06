#include "ServerMode.h"

#include "../IntegrationTests/TestServer.h"

#include <Public/PublicDef.h>
#include <Session/Session.h>
#include <Transport/Transport.h>

#include <memory>

namespace MCP
{
	namespace Test
	{
		int RunE2EServer(bool bLegacyOnly)
		{
			CTestServer& server = CTestServer::s_Instance;

			// Legacy mode keeps the registration byte-identical to the golden
			// baseline; modern mode adds the Apps and completions surface.
			if (!bLegacyOnly)
			{
				CTestServer::SetRegisterModernFixtures(true);
				server.RegisterServerCompletionsCapability();
				server.RegisterServerAppsCapabilities();
				server.RegisterServerInstructions("TinyMCP E2E server");
			}

			// Install stdio before Start(). The declared versions decide which
			// protocol revisions are served; the legacy-only fixture asks for the
			// handshake era alone.
			CMCPSession::GetInstance().SetTransport(std::make_shared<MCP::CStdioTransport>());
			if (bLegacyOnly)
				server.SetSupportedProtocolVersions({ PROTOCOL_VER_2024_11_05 });

			int iErrCode = server.Initialize();
			if (ERRNO_OK != iErrCode)
				return iErrCode;

			iErrCode = server.Start();
			server.Stop();
			server.Uninitialize();

			// EOF on stdin is the normal shutdown path for a stdio client.
			return (ERRNO_INTERNAL_INPUT_TERMINATE == iErrCode) ? 0 : iErrCode;
		}
	}
}
