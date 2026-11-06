#pragma once
// Internal to the SDK. The protocol registry is derived from the versions the
// server declares, so it is not part of the session's public surface.

#include "../Session/Session.h"
#include "Protocol.h"

namespace MCP
{
	struct ProtocolRegistryAccess
	{
		static void Set(CMCPSession& session, const CProtocolRegistry& registry)
		{
			session.SetProtocolRegistry(registry);
		}

		static const CProtocolRegistry& Get(const CMCPSession& session)
		{
			return session.GetProtocolRegistry();
		}
	};
}
