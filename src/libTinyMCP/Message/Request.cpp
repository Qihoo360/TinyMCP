#include "Request.h"
#include <json/writer.h>

namespace MCP
{
	////////////////////////////////////////////////////////////////////////////////////////
	// Request
	int Request::DoSerialize(Json::Value& jMsg) const
	{
		if (!IsValid())
			return ERRNO_INVALID_REQUEST;

		Json::Value jRPC(JSON_RPC_VER);
		jMsg[MSG_KEY_JSONRPC] = jRPC;

		Json::Value jMethod(strMethod);
		jMsg[MSG_KEY_METHOD] = jMethod;

		if (progressToken.IsValid())
		{
			Json::Value jParams(Json::objectValue);
			if (jMsg.isMember(MCP::MSG_KEY_PARAMS) && jMsg[MCP::MSG_KEY_PARAMS].isObject())
				jParams = jMsg[MCP::MSG_KEY_PARAMS];

			Json::Value jMeta(Json::objectValue);
			progressToken.DoSerialize(jMeta);
			jParams[MSG_KEY_META] = jMeta;
			jMsg[MSG_KEY_PARAMS] = jParams;
		}

		return requestId.DoSerialize(jMsg);
	}

	int Request::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_JSONRPC) || !jMsg[MSG_KEY_JSONRPC].isString())
			return ERRNO_INVALID_REQUEST;
		auto strRPCVer = jMsg[MSG_KEY_JSONRPC].asString();
		if (strRPCVer.compare(JSON_RPC_VER) != 0)
			return ERRNO_INVALID_REQUEST;

		if (!jMsg.isMember(MSG_KEY_METHOD))
			return ERRNO_INVALID_REQUEST;
		strMethod = jMsg[MSG_KEY_METHOD].asString();
		if (strMethod.empty())
			return ERRNO_INVALID_REQUEST;

		if (jMsg.isMember(MSG_KEY_PARAMS) && jMsg[MSG_KEY_PARAMS].isObject())
		{
			auto& jParams = jMsg[MSG_KEY_PARAMS];
			if (jParams.isMember(MSG_KEY_META) && jParams[MSG_KEY_META].isObject())
			{
				auto& jMeta = jParams[MSG_KEY_META];
				progressToken.DoDeserialize(jMeta);
			}
		}

		return requestId.DoDeserialize(jMsg);
	}

	bool Request::IsValid() const
	{
		if (!requestId.IsValid())
			return false;

		if (strMethod.empty())
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// InitializeRequest
	int InitializeRequest::DoSerialize(Json::Value& jMsg) const
	{
		return Request::DoSerialize(jMsg);
	}

	int InitializeRequest::DoDeserialize(const Json::Value& jMsg)
	{
		int iErrCode = Request::DoDeserialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		if (!jMsg.isMember(MSG_KEY_PARAMS) || !jMsg[MSG_KEY_PARAMS].isObject())
			return ERRNO_INVALID_REQUEST;
		auto& jParams = jMsg[MSG_KEY_PARAMS];

		if (!jParams.isMember(MSG_KEY_PROTOCOL_VERSION))
			return ERRNO_INVALID_REQUEST;
		strProtocolVer = jParams[MSG_KEY_PROTOCOL_VERSION].asString();

		if (!jParams.isMember(MSG_KEY_CLIENT_INFO) || !jParams[MSG_KEY_CLIENT_INFO].isObject())
			return ERRNO_INVALID_REQUEST;
		auto& jClientInfo = jParams[MSG_KEY_CLIENT_INFO];

		return clientInfo.DoDeserialize(jClientInfo);
	}

	bool InitializeRequest::IsValid() const
	{
		if (!Request::IsValid())
			return false;

		if (strProtocolVer.empty() || !clientInfo.IsValid())
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ListToolsRequest
	int ListToolsRequest::DoSerialize(Json::Value& jMsg) const
	{
		return Request::DoSerialize(jMsg);
	}

	int ListToolsRequest::DoDeserialize(const Json::Value& jMsg)
	{
		int iErrCode = Request::DoDeserialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		if (jMsg.isMember(MSG_KEY_PARAMS) && jMsg[MSG_KEY_PARAMS].isObject())
		{
			auto& jParams = jMsg[MSG_KEY_PARAMS];

			if (jParams.isMember(MSG_KEY_CURSOR) && jParams[MSG_KEY_CURSOR].isString())
			{
				strCursor = jParams[MSG_KEY_CURSOR].asString();
			}
		}

		return ERRNO_OK;
	}

	bool ListToolsRequest::IsValid() const
	{
		if (!Request::IsValid())
			return false;

		if (strMethod.compare(METHOD_TOOLS_LIST) != 0)
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// CallToolRequest
	int CallToolRequest::DoSerialize(Json::Value& jMsg) const
	{
		return Request::DoSerialize(jMsg);
	}

	int CallToolRequest::DoDeserialize(const Json::Value& jMsg)
	{
		int iErrCode = Request::DoDeserialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		if (!jMsg.isMember(MSG_KEY_PARAMS) || !jMsg[MSG_KEY_PARAMS].isObject())
			return ERRNO_INVALID_REQUEST;
		auto& jParams = jMsg[MSG_KEY_PARAMS];
		if (!jParams.isMember(MSG_KEY_NAME) || !jParams[MSG_KEY_NAME].isString())
			return ERRNO_INVALID_REQUEST;
		strName = jParams[MSG_KEY_NAME].asString();

		if (jParams.isMember(MSG_KEY_ARGUMENTS) && jParams[MSG_KEY_ARGUMENTS].isObject())
		{
			jArguments = jParams[MSG_KEY_ARGUMENTS];
		}

		return ERRNO_OK;
	}

	bool CallToolRequest::IsValid() const
	{
		if (!Request::IsValid())
			return false;

		if (strMethod.compare(METHOD_TOOLS_CALL) != 0)
			return false;

		if (strName.empty())
			return false;

		return true;
	}
}