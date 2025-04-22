#pragma once
// 为保证较好的跨平台特性，MCP命名空间的代码均使用标准c++编码
// 非必要情况下，禁止使用特定系统平台API

#include "Message.h"

namespace MCP
{
	struct RequestId : public MCP::Message
	{
	public:
		RequestId()
			: Message(MessageType_RequestId, MessageCategory_Basic, false)
		{

		}

		DataType eIdDataType{ DataType_Unknown };
		int iId{ -1 };
		std::string strId;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;

		inline void SetMsgKey(const std::string& strMsgKey)
		{
			_strMsgKey = strMsgKey;
		}
		inline bool IsEqual(const MCP::RequestId& rhs)
		{
			return eIdDataType == rhs.eIdDataType
				&& iId == rhs.iId
				&& strId == rhs.strId;
		}

	private:
		std::string _strMsgKey;
	};

	struct Implementation : public MCP::Message
	{
	public:
		Implementation()
			: Message(MessageType_Implementation, MessageCategory_Basic, false)
		{

		}

		std::string strName;
		std::string strVersion;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct Prompts : public MCP::Message
	{
	public:
		Prompts()
			: Message(MessageType_Prompts, MessageCategory_Basic, false)
		{

		}

		bool bListChanged{ false };

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct Resources : public MCP::Message
	{
	public:
		Resources()
			: Message(MessageType_Resources, MessageCategory_Basic, false)
		{

		}

		bool bListChanged{ false };
		bool bSubscribe{ false };

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct Tools : public MCP::Message
	{
	public:
		Tools()
			: Message(MessageType_Tools, MessageCategory_Basic, false)
		{

		}

		bool bListChanged{ false };

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ServerCapabilities : public MCP::Message
	{
	public:
		ServerCapabilities()
			: Message(MessageType_ServerCapabilities, MessageCategory_Basic, false)
		{
			prompts.bExist = false;
			resources.bExist = false;
			tools.bExist = false;
		}

		MCP::Prompts prompts;
		MCP::Resources resources;
		MCP::Tools tools;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct Tool : public MCP::Message
	{
		Tool()
			: Message(MessageType_Tool, MessageCategory_Basic, false)
		{

		}

		std::string strName;
		std::string strDescription;
		Json::Value jInputSchema;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct TextContent : public MCP::Message
	{
		TextContent()
			: Message(MessageType_TextContent, MessageCategory_Basic, false)
		{

		}

		std::string strText;
		std::string strType;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ImageContent : public MCP::Message
	{
		ImageContent()
			: Message(MessageType_ImageContent, MessageCategory_Basic, false)
		{

		}

		std::string strType;
		std::string strMimeType;
		std::string strData;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct TextResourceContents : public MCP::Message
	{
		TextResourceContents()
			: Message(MessageType_TextResourceContents, MessageCategory_Basic, false)
		{

		}

		std::string strText;
		std::string strUri;
		std::string strMimeType;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct BlobResourceContents : public MCP::Message
	{
		BlobResourceContents()
			: Message(MessageType_BlobResourceContents, MessageCategory_Basic, false)
		{

		}

		std::string strBlob;
		std::string strUri;
		std::string strMimeType;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct EmbeddedResource : public MCP::Message
	{
		EmbeddedResource()
			: Message(MessageType_EmbeddedResource, MessageCategory_Basic, false)
		{

		}

		std::string strType;
		TextResourceContents textResource;
		BlobResourceContents blobResource;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ProgressToken : public MCP::Message
	{
		ProgressToken()
			: Message(MessageType_ProgressToken, MessageCategory_Basic, false)
		{

		}

		DataType eTokenDataType{ DataType_Unknown };
		int iToken{ -1 };
		std::string strToken;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;

		inline bool IsEqual(const MCP::ProgressToken& rhs)
		{
			return eTokenDataType == rhs.eTokenDataType
				&& iToken == rhs.iToken
				&& strToken == rhs.strToken;
		}
	};
}