#pragma once
// Ϊ��֤�ϺõĿ�ƽ̨���ԣ�MCP�����ռ�Ĵ����ʹ�ñ�׼c++����
// �Ǳ�Ҫ����£���ֹʹ���ض�ϵͳƽ̨API

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

		}

		std::string strName;
		Json::Value jArguments;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};
}
