#pragma once
// Internal to the SDK. The protocol registry is derived from the versions the
// server declares, so it is not part of the session's public surface. The same
// applies to the result helpers and diagnostics used by the task adapter and by
// the integration tests.

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

	struct SessionAccess
	{
		static void PrepareResult(CMCPSession& session, const std::shared_ptr<MCP::Request>& spRequest,
			const std::shared_ptr<MCP::Response>& spResponse)
		{
			session.PrepareResult(spRequest, spResponse);
		}

		static size_t GetMessageCount(const CMCPSession& session, MessageCategory eCategory)
		{
			return session.GetMessageCount(eCategory);
		}

		static int SendToolListChanged(CMCPSession& session)
		{
			return session.SendToolListChanged();
		}

		static int SendPromptListChanged(CMCPSession& session)
		{
			return session.SendPromptListChanged();
		}

		static int SendResourceListChanged(CMCPSession& session)
		{
			return session.SendResourceListChanged();
		}

		static int SendSubscriptionNotification(CMCPSession& session,
			const MCP::ResourceUpdatedNotification& notification, const std::string& strMatchUri)
		{
			return session.SendSubscriptionNotification(notification, strMatchUri);
		}
	};
}
