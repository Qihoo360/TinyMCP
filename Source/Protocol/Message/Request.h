#pragma once
// To ensure good cross-platform compatibility, all code within the MCP namespace is written in standard C++.
// The use of platform-specific system APIs is prohibited unless necessary.

#include <string>
#include <json/json.h>
#include "BasicMessage.h"

namespace MCP
{
	struct Request : public MCP::Message
	{
	public:
		Request(MessageType eMsgType, bool bNeedIdentity)
			: Message(eMsgType, MessageCategory_Request, bNeedIdentity)
		{

		}

		MCP::RequestId requestId;
		std::string strMethod;
		MCP::ProgressToken progressToken;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct PingRequest : public MCP::Request
	{
		PingRequest(bool bNeedIdentity)
			: Request(MessageType_PingRequest, bNeedIdentity)
		{

		}

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct InitializeRequest : public MCP::Request
	{
	public:
		InitializeRequest(bool bNeedIdentity)
			: Request(MessageType_InitializeRequest, bNeedIdentity)
		{

		}

		std::string strProtocolVer;
		Implementation clientInfo;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ListToolsRequest : public MCP::Request
	{
	public:
		ListToolsRequest(bool bNeedIdentity)
			: Request(MessageType_ListToolsRequest, bNeedIdentity)
		{

		}

		std::string strCursor;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct CallToolRequest : public MCP::Request
	{
	public:
		CallToolRequest(bool bNeedIdentity)
			: Request(MessageType_CallToolRequest, bNeedIdentity)
		{
			toolCallId.SetMsgKey(MCP::MSG_KEY_TOOL_CALL_ID);
		}

		std::string strName;
		Json::Value jArguments;
		RequestId toolCallId;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};
}
