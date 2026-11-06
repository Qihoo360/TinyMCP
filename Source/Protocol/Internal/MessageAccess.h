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
	};
}
