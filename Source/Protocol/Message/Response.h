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
		std::string strResultType;
		Json::Value jInputRequests;
		std::string strRequestState;
		int iTtlMs{ -1 };
		std::string strCacheScope;
		Json::Value jResultMeta;
		Json::Value jServerInfo;
		bool bMrtrRetry{ false };

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;

		// Result caching only applies to the resource/tool/prompt read methods.
		virtual bool IsCacheableResult() const;

		// Writes every 2026-07-28 result level field behind its feature gate.
		void WriteVersionResultFields(Json::Value& jResult, const ProtocolContext& ctxProtocol, const Json::Value& jServerInfo) const;
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
		Json::Value jErrorData;

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
		bool IsCacheableResult() const override;
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
		Json::Value jStructuredContent;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ListResourcesResult : public MCP::Response
	{
	public:
		ListResourcesResult(bool bNeedIdentity)
			: Response(MessageType_ListResourcesResult, bNeedIdentity)
		{

		}

		std::vector<MCP::Resource> vecResources;
		std::string strNextCursor;

		bool IsValid() const override;
		bool IsCacheableResult() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ReadResourceResult : public MCP::Response
	{
	public:
		ReadResourceResult(bool bNeedIdentity)
			: Response(MessageType_ReadResourceResult, bNeedIdentity)
		{

		}

		std::vector<MCP::TextResourceContents> vecTextResourceContents;
		std::vector<MCP::BlobResourceContents> vecBlobResourceContents;

		bool IsValid() const override;
		bool IsCacheableResult() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ListResourceTemplatesResult : public MCP::Response
	{
	public:
		ListResourceTemplatesResult(bool bNeedIdentity)
			: Response(MessageType_ListResourceTemplatesResult, bNeedIdentity)
		{

		}

		std::vector<MCP::ResourceTemplate> vecResourceTemplates;
		std::string strNextCursor;

		bool IsValid() const override;
		bool IsCacheableResult() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ListPromptsResult : public MCP::Response
	{
	public:
		ListPromptsResult(bool bNeedIdentity)
			: Response(MessageType_ListPromptsResult, bNeedIdentity)
		{

		}

		std::vector<MCP::Prompt> vecPrompts;
		std::string strNextCursor;

		bool IsValid() const override;
		bool IsCacheableResult() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct GetPromptResult : public MCP::Response
	{
	public:
		GetPromptResult(bool bNeedIdentity)
			: Response(MessageType_GetPromptResult, bNeedIdentity)
		{

		}

		std::string strDescription;
		std::vector<MCP::PromptMessage> vecMessages;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct CompleteResult : public MCP::Response
	{
	public:
		CompleteResult(bool bNeedIdentity)
			: Response(MessageType_CompleteResult, bNeedIdentity)
		{

		}

		std::vector<std::string> vecValues;
		int iTotal{ -1 };
		bool bHasMore{ false };

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	// 2026-07-28 server/discover result.
	struct DiscoverResult : public MCP::Response
	{
	public:
		DiscoverResult(bool bNeedIdentity)
			: Response(MessageType_DiscoverResult, bNeedIdentity)
		{
		}

		std::vector<std::string> vecSupportedVersions;
		MCP::ServerCapabilities capabilities;
		std::string strInstructions;

		bool IsValid() const override;
		bool IsCacheableResult() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	// 2026-07-28 subscriptions/listen result. The subscription id keeps its
	// original JSON type because the schema allows string and integer ids.
	struct SubscriptionsListenResult : public MCP::Response
	{
	public:
		SubscriptionsListenResult(bool bNeedIdentity)
			: Response(MessageType_SubscriptionsListenResult, bNeedIdentity)
		{
		}

		Json::Value jSubscriptionId;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};
}