#pragma once
// Internal to the SDK. Public headers only forward declare the protocol
// context; access goes through this friend so the type stays opaque.

#include "../Message/Message.h"
#include "Protocol.h"

namespace MCP
{
	struct MessageContextAccess
	{
		static const ProtocolContext& Get(const Message& msg)
		{
			return msg.GetProtocolContext();
		}

		static void Set(Message& msg, const ProtocolContext& ctxProtocol)
		{
			msg.SetProtocolContext(ctxProtocol);
		}

		static std::shared_ptr<const ProtocolContext> GetShared(const Message& msg)
		{
			return msg.GetSharedProtocolContext();
		}

		static void Set(Message& msg, const std::shared_ptr<const ProtocolContext>& spProtocolContext)
		{
			msg.SetProtocolContext(spProtocolContext);
		}

		// Copies the context without re-allocating it; used when a result or an
		// error response inherits the era of the request it answers.
		static void Copy(Message& msg, const Message& source)
		{
			msg.SetProtocolContext(source.GetSharedProtocolContext());
		}
	};
}
