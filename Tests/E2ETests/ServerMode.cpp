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

			// Install stdio before Start(): CMCPServer::Start() only enables the
			// stateless era when it installs the transport itself.
			CMCPSession::GetInstance().SetTransport(std::make_shared<MCP::CStdioTransport>());
			CMCPSession::GetInstance().SetModernProtocolSupported(!bLegacyOnly);

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
