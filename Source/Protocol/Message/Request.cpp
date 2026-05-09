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
	// PingRequest
	int PingRequest::DoSerialize(Json::Value& jMsg) const
	{
		return Request::DoSerialize(jMsg);
	}

	int PingRequest::DoDeserialize(const Json::Value& jMsg)
	{
		return Request::DoDeserialize(jMsg);
	}

	bool PingRequest::IsValid() const
	{
		if (!Request::IsValid())
			return false;

		if (strMethod.compare(METHOD_PING) != 0)
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
		if (jParams.isMember(MSG_KEY_META) && jParams[MSG_KEY_META].isObject())
		{
			auto& jMeta = jParams[MSG_KEY_META];
			toolCallId.DoDeserialize(jMeta);
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

	////////////////////////////////////////////////////////////////////////////////////////
	// ListResourcesRequest
	int ListResourcesRequest::DoSerialize(Json::Value& jMsg) const
	{
		return Request::DoSerialize(jMsg);
	}

	int ListResourcesRequest::DoDeserialize(const Json::Value& jMsg)
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

	bool ListResourcesRequest::IsValid() const
	{
		if (!Request::IsValid())
			return false;

		if (strMethod.compare(METHOD_RESOURCES_LIST) != 0)
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ReadResourceRequest
	int ReadResourceRequest::DoSerialize(Json::Value& jMsg) const
	{
		return Request::DoSerialize(jMsg);
	}

	int ReadResourceRequest::DoDeserialize(const Json::Value& jMsg)
	{
		int iErrCode = Request::DoDeserialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		if (!jMsg.isMember(MSG_KEY_PARAMS) || !jMsg[MSG_KEY_PARAMS].isObject())
			return ERRNO_INVALID_REQUEST;
		auto& jParams = jMsg[MSG_KEY_PARAMS];
		if (!jParams.isMember(MSG_KEY_URI) || !jParams[MSG_KEY_URI].isString())
			return ERRNO_INVALID_REQUEST;
		strUri = jParams[MSG_KEY_URI].asString();

		return ERRNO_OK;
	}

	bool ReadResourceRequest::IsValid() const
	{
		if (!Request::IsValid())
			return false;

		if (strMethod.compare(METHOD_RESOURCES_READ) != 0)
			return false;

		if (strUri.empty())
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// SubscribeResourceRequest
	int SubscribeResourceRequest::DoSerialize(Json::Value& jMsg) const
	{
		return Request::DoSerialize(jMsg);
	}

	int SubscribeResourceRequest::DoDeserialize(const Json::Value& jMsg)
	{
		int iErrCode = Request::DoDeserialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		if (!jMsg.isMember(MSG_KEY_PARAMS) || !jMsg[MSG_KEY_PARAMS].isObject())
			return ERRNO_INVALID_REQUEST;
		auto& jParams = jMsg[MSG_KEY_PARAMS];
		if (!jParams.isMember(MSG_KEY_URI) || !jParams[MSG_KEY_URI].isString())
			return ERRNO_INVALID_REQUEST;
		strUri = jParams[MSG_KEY_URI].asString();

		return ERRNO_OK;
	}

	bool SubscribeResourceRequest::IsValid() const
	{
		if (!Request::IsValid())
			return false;

		if (strMethod.compare(METHOD_RESOURCES_SUBSCRIBE) != 0)
			return false;

		if (strUri.empty())
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// UnsubscribeResourceRequest
	int UnsubscribeResourceRequest::DoSerialize(Json::Value& jMsg) const
	{
		return Request::DoSerialize(jMsg);
	}

	int UnsubscribeResourceRequest::DoDeserialize(const Json::Value& jMsg)
	{
		int iErrCode = Request::DoDeserialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		if (!jMsg.isMember(MSG_KEY_PARAMS) || !jMsg[MSG_KEY_PARAMS].isObject())
			return ERRNO_INVALID_REQUEST;
		auto& jParams = jMsg[MSG_KEY_PARAMS];
		if (!jParams.isMember(MSG_KEY_URI) || !jParams[MSG_KEY_URI].isString())
			return ERRNO_INVALID_REQUEST;
		strUri = jParams[MSG_KEY_URI].asString();

		return ERRNO_OK;
	}

	bool UnsubscribeResourceRequest::IsValid() const
	{
		if (!Request::IsValid())
			return false;

		if (strMethod.compare(METHOD_RESOURCES_UNSUBSCRIBE) != 0)
			return false;

		if (strUri.empty())
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ListResourceTemplatesRequest
	int ListResourceTemplatesRequest::DoSerialize(Json::Value& jMsg) const
	{
		return Request::DoSerialize(jMsg);
	}

	int ListResourceTemplatesRequest::DoDeserialize(const Json::Value& jMsg)
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

	bool ListResourceTemplatesRequest::IsValid() const
	{
		if (!Request::IsValid())
			return false;

		if (strMethod.compare(METHOD_RESOURCES_TEMPLATES_LIST) != 0)
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ListPromptsRequest
	int ListPromptsRequest::DoSerialize(Json::Value& jMsg) const
	{
		return Request::DoSerialize(jMsg);
	}

	int ListPromptsRequest::DoDeserialize(const Json::Value& jMsg)
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

	bool ListPromptsRequest::IsValid() const
	{
		if (!Request::IsValid())
			return false;

		if (strMethod.compare(METHOD_PROMPTS_LIST) != 0)
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// GetPromptRequest
	int GetPromptRequest::DoSerialize(Json::Value& jMsg) const
	{
		return Request::DoSerialize(jMsg);
	}

	int GetPromptRequest::DoDeserialize(const Json::Value& jMsg)
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

	bool GetPromptRequest::IsValid() const
	{
		if (!Request::IsValid())
			return false;

		if (strMethod.compare(METHOD_PROMPTS_GET) != 0)
			return false;

		if (strName.empty())
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// CompleteRequest
	int CompleteRequest::DoSerialize(Json::Value& jMsg) const
	{
		return Request::DoSerialize(jMsg);
	}

	int CompleteRequest::DoDeserialize(const Json::Value& jMsg)
	{
		int iErrCode = Request::DoDeserialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		if (!jMsg.isMember(MSG_KEY_PARAMS) || !jMsg[MSG_KEY_PARAMS].isObject())
			return ERRNO_INVALID_REQUEST;
		auto& jParams = jMsg[MSG_KEY_PARAMS];

		if (!jParams.isMember(MSG_KEY_REF) || !jParams[MSG_KEY_REF].isObject())
			return ERRNO_INVALID_REQUEST;
		auto& jRef = jParams[MSG_KEY_REF];

		if (!jRef.isMember(MSG_KEY_TYPE) || !jRef[MSG_KEY_TYPE].isString())
			return ERRNO_INVALID_REQUEST;
		strRefType = jRef[MSG_KEY_TYPE].asString();

		if (strRefType == CONST_REF_PROMPT)
		{
			iErrCode = promptRef.DoDeserialize(jRef);
			if (ERRNO_OK != iErrCode)
				return iErrCode;
		}
		else if (strRefType == CONST_REF_RESOURCE)
		{
			iErrCode = resourceRef.DoDeserialize(jRef);
			if (ERRNO_OK != iErrCode)
				return iErrCode;
		}
		else
		{
			return ERRNO_INVALID_REQUEST;
		}

		if (!jParams.isMember(MSG_KEY_ARGUMENT) || !jParams[MSG_KEY_ARGUMENT].isObject())
			return ERRNO_INVALID_REQUEST;
		auto& jArgument = jParams[MSG_KEY_ARGUMENT];

		if (!jArgument.isMember(MSG_KEY_NAME) || !jArgument[MSG_KEY_NAME].isString())
			return ERRNO_INVALID_REQUEST;
		strArgumentName = jArgument[MSG_KEY_NAME].asString();

		if (!jArgument.isMember(MSG_KEY_VALUE) || !jArgument[MSG_KEY_VALUE].isString())
			return ERRNO_INVALID_REQUEST;
		strArgumentValue = jArgument[MSG_KEY_VALUE].asString();

		return ERRNO_OK;
	}

	bool CompleteRequest::IsValid() const
	{
		if (!Request::IsValid())
			return false;

		if (strMethod.compare(METHOD_COMPLETION_COMPLETE) != 0)
			return false;

		if (strRefType != CONST_REF_PROMPT && strRefType != CONST_REF_RESOURCE)
			return false;

		if (strArgumentName.empty())
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// SetLevelRequest
	int SetLevelRequest::DoSerialize(Json::Value& jMsg) const
	{
		return Request::DoSerialize(jMsg);
	}

	int SetLevelRequest::DoDeserialize(const Json::Value& jMsg)
	{
		int iErrCode = Request::DoDeserialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		if (!jMsg.isMember(MSG_KEY_PARAMS) || !jMsg[MSG_KEY_PARAMS].isObject())
			return ERRNO_INVALID_REQUEST;
		auto& jParams = jMsg[MSG_KEY_PARAMS];

		return level.DoDeserialize(jParams);
	}

	bool SetLevelRequest::IsValid() const
	{
		if (!Request::IsValid())
			return false;

		if (strMethod.compare(METHOD_LOGGING_SET_LEVEL) != 0)
			return false;

		if (!level.IsValid())
			return false;

		return true;
	}
}