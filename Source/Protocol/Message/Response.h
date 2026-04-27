#pragma once
// To ensure good cross-platform compatibility, all code within the MCP namespace is written in standard C++.
// The use of platform-specific system APIs is prohibited unless necessary.

#include <json/json.h>
#include "BasicMessage.h"

namespace MCP
{
	struct Response : public MCP::Message
	{
	public:
		Response(MessageType eMsgType, bool bNeedIdentity)
			: Message(eMsgType, MessageCategory_Response, bNeedIdentity)
		{

		}

		MCP::RequestId requestId;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct EmptyResponse : public MCP::Response
	{
	public:
		EmptyResponse(bool bNeedIdentity)
			: Response(MessageType_EmptyResponse, bNeedIdentity)
		{

		}

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ErrorResponse : public MCP::Response
	{
	public:
		ErrorResponse(bool bNeedIdentity)
			: Response(MessageType_ErrorResponse, bNeedIdentity)
		{

		}

		int iCode{ 0 };
		std::string strMesage;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct InitializeResult : public MCP::Response
	{
	public:
		InitializeResult(bool bNeedIdentity)
			: Response(MessageType_InitializeResult, bNeedIdentity)
		{

		}

		std::string strProtocolVersion;
		MCP::ServerCapabilities capabilities;
		MCP::Implementation implServerInfo;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ListToolsResult : public MCP::Response
	{
	public:
		ListToolsResult(bool bNeedIdentity)
			: Response(MessageType_ListToolsResult, bNeedIdentity)
		{

		}

		std::vector<MCP::Tool> vecTools;
		std::string strNextCursor;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct CallToolResult : public MCP::Response
	{
	public:
		CallToolResult(bool bNeedIdentity)
			: Response(MessageType_CallToolResult, bNeedIdentity)
		{

		}

		bool bIsError{ false };
		std::vector<MCP::TextContent> vecTextContent;
		std::vector<MCP::ImageContent> vecImageContent;
		std::vector<MCP::EmbeddedResource> vecEmbeddedResource;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};
}