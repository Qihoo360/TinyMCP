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

	struct ListResourcesRequest : public MCP::Request
	{
	public:
		ListResourcesRequest(bool bNeedIdentity)
			: Request(MessageType_ListResourcesRequest, bNeedIdentity)
		{

		}

		std::string strCursor;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ReadResourceRequest : public MCP::Request
	{
	public:
		ReadResourceRequest(bool bNeedIdentity)
			: Request(MessageType_ReadResourceRequest, bNeedIdentity)
		{

		}

		std::string strUri;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct SubscribeResourceRequest : public MCP::Request
	{
	public:
		SubscribeResourceRequest(bool bNeedIdentity)
			: Request(MessageType_SubscribeResourceRequest, bNeedIdentity)
		{

		}

		std::string strUri;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct UnsubscribeResourceRequest : public MCP::Request
	{
	public:
		UnsubscribeResourceRequest(bool bNeedIdentity)
			: Request(MessageType_UnsubscribeResourceRequest, bNeedIdentity)
		{

		}

		std::string strUri;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ListResourceTemplatesRequest : public MCP::Request
	{
	public:
		ListResourceTemplatesRequest(bool bNeedIdentity)
			: Request(MessageType_ListResourceTemplatesRequest, bNeedIdentity)
		{

		}

		std::string strCursor;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ListPromptsRequest : public MCP::Request
	{
	public:
		ListPromptsRequest(bool bNeedIdentity)
			: Request(MessageType_ListPromptsRequest, bNeedIdentity)
		{

		}

		std::string strCursor;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct GetPromptRequest : public MCP::Request
	{
	public:
		GetPromptRequest(bool bNeedIdentity)
			: Request(MessageType_GetPromptRequest, bNeedIdentity)
		{

		}

		std::string strName;
		Json::Value jArguments;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct CompleteRequest : public MCP::Request
	{
	public:
		CompleteRequest(bool bNeedIdentity)
			: Request(MessageType_CompleteRequest, bNeedIdentity)
		{

		}

		MCP::PromptReference promptRef;
		MCP::ResourceReference resourceRef;
		std::string strRefType;
		std::string strArgumentName;
		std::string strArgumentValue;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct SetLevelRequest : public MCP::Request
	{
	public:
		SetLevelRequest(bool bNeedIdentity)
			: Request(MessageType_SetLevelRequest, bNeedIdentity)
		{

		}

		MCP::LoggingLevel level;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};
}
