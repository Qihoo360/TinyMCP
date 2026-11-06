#include "Request.h"
#include "../Internal/Protocol.h"
#include "../Internal/VersionExtras.h"
#include <json/writer.h>

namespace MCP
{
	////////////////////////////////////////////////////////////////////////////////////////
	// Request
	int Request::DoSerialize(Json::Value& jMsg) const
	{
		const ProtocolContext& ctxProtocol = GetProtocolContext();

		if (!IsValid())
			return ERRNO_INVALID_REQUEST;

		Json::Value jRPC(JSON_RPC_VER);
		jMsg[MSG_KEY_JSONRPC] = jRPC;

		Json::Value jMethod(strMethod);
		jMsg[MSG_KEY_METHOD] = jMethod;

		Json::Value jParams(Json::objectValue);
		bool bHasParams = false;
		if (jMsg.isMember(MCP::MSG_KEY_PARAMS) && jMsg[MCP::MSG_KEY_PARAMS].isObject())
		{
			jParams = jMsg[MCP::MSG_KEY_PARAMS];
			bHasParams = true;
		}

		Json::Value jMeta(Json::objectValue);
		meta.DoSerialize(jMeta);

		// Per-request logLevel only exists when the profile declares it; emitting
		// it on a legacy request would introduce an undefined key.
		if (!ctxProtocol.Supports(Feature_LogLevelInMeta))
			jMeta.removeMember(META_KEY_LOG_LEVEL);

		VersionExtrasAccess::Serialize(meta, jMeta, ctxProtocol);
		if (progressToken.IsValid())
			progressToken.DoSerialize(jMeta);

		if (!jMeta.empty())
		{
			jParams[MSG_KEY_META] = jMeta;
			bHasParams = true;
		}

		if (ctxProtocol.Supports(Feature_Mrtr))
		{
			if (jInputResponses.isObject() && !jInputResponses.empty())
			{
				jParams[MSG_KEY_INPUT_RESPONSES] = jInputResponses;
				bHasParams = true;
			}

			if (!strRequestState.empty())
			{
				jParams[MSG_KEY_REQUEST_STATE] = strRequestState;
				bHasParams = true;
			}
		}

		if (bHasParams)
			jMsg[MSG_KEY_PARAMS] = jParams;

		return requestId.DoSerialize(jMsg);
	}

	int Request::DoDeserialize(const Json::Value& jMsg)
	{
		const ProtocolContext& ctxProtocol = GetProtocolContext();

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
				meta.DoDeserialize(jMeta);
				VersionExtrasAccess::Deserialize(meta, jMeta, ctxProtocol);

				// Per-request logLevel only exists when the profile declares it.
				if (!ctxProtocol.Supports(Feature_LogLevelInMeta))
					meta.strLogLevel.clear();
			}

			if (ctxProtocol.Supports(Feature_Mrtr))
			{
				if (jParams.isMember(MSG_KEY_INPUT_RESPONSES) && jParams[MSG_KEY_INPUT_RESPONSES].isObject())
					jInputResponses = jParams[MSG_KEY_INPUT_RESPONSES];

				if (jParams.isMember(MSG_KEY_REQUEST_STATE) && jParams[MSG_KEY_REQUEST_STATE].isString())
					strRequestState = jParams[MSG_KEY_REQUEST_STATE].asString();
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

	////////////////////////////////////////////////////////////////////////////////////////
	// DiscoverRequest
	int DiscoverRequest::DoSerialize(Json::Value& jMsg) const
	{
		return Request::DoSerialize(jMsg);
	}

	int DiscoverRequest::DoDeserialize(const Json::Value& jMsg)
	{
		return Request::DoDeserialize(jMsg);
	}

	bool DiscoverRequest::IsValid() const
	{
		if (!Request::IsValid())
			return false;

		if (strMethod.compare(METHOD_SERVER_DISCOVER) != 0)
			return false;

		// params._meta is mandatory once the stateless era is negotiated.
		if (GetProtocolContext().Supports(Feature_Discover) && !meta.HasValue())
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// SubscriptionsListenRequest
	int SubscriptionsListenRequest::DoSerialize(Json::Value& jMsg) const
	{
		int iErrCode = Request::DoSerialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		Json::Value jParams(Json::objectValue);
		if (jMsg.isMember(MSG_KEY_PARAMS) && jMsg[MSG_KEY_PARAMS].isObject())
			jParams = jMsg[MSG_KEY_PARAMS];

		Json::Value jNotifications(Json::objectValue);
		if (bToolsListChanged)
		{
			Json::Value jChanged(true);
			jNotifications[MSG_KEY_TOOLS_LIST_CHANGED] = jChanged;
		}
		if (bPromptsListChanged)
		{
			Json::Value jChanged(true);
			jNotifications[MSG_KEY_PROMPTS_LIST_CHANGED] = jChanged;
		}
		if (bResourcesListChanged)
		{
			Json::Value jChanged(true);
			jNotifications[MSG_KEY_RESOURCES_LIST_CHANGED] = jChanged;
		}
		if (!vecResourceSubscriptions.empty())
		{
			Json::Value jSubscriptions(Json::arrayValue);
			for (const auto& strUri : vecResourceSubscriptions)
				jSubscriptions.append(strUri);
			jNotifications[MSG_KEY_RESOURCE_SUBSCRIPTIONS] = jSubscriptions;
		}

		if (!jNotifications.empty())
		{
			jParams[MSG_KEY_NOTIFICATIONS] = jNotifications;
			jMsg[MSG_KEY_PARAMS] = jParams;
		}

		return ERRNO_OK;
	}

	int SubscriptionsListenRequest::DoDeserialize(const Json::Value& jMsg)
	{
		int iErrCode = Request::DoDeserialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		if (!jMsg.isMember(MSG_KEY_PARAMS) || !jMsg[MSG_KEY_PARAMS].isObject())
			return ERRNO_OK;

		auto& jParams = jMsg[MSG_KEY_PARAMS];
		if (!jParams.isMember(MSG_KEY_NOTIFICATIONS) || !jParams[MSG_KEY_NOTIFICATIONS].isObject())
			return ERRNO_OK;

		bHasNotifications = true;
		auto& jNotifications = jParams[MSG_KEY_NOTIFICATIONS];
		bToolsListChanged = jNotifications.isMember(MSG_KEY_TOOLS_LIST_CHANGED)
			&& jNotifications[MSG_KEY_TOOLS_LIST_CHANGED].asBool();
		bPromptsListChanged = jNotifications.isMember(MSG_KEY_PROMPTS_LIST_CHANGED)
			&& jNotifications[MSG_KEY_PROMPTS_LIST_CHANGED].asBool();
		bResourcesListChanged = jNotifications.isMember(MSG_KEY_RESOURCES_LIST_CHANGED)
			&& jNotifications[MSG_KEY_RESOURCES_LIST_CHANGED].asBool();

		if (jNotifications.isMember(MSG_KEY_RESOURCE_SUBSCRIPTIONS)
			&& jNotifications[MSG_KEY_RESOURCE_SUBSCRIPTIONS].isArray())
		{
			vecResourceSubscriptions.clear();
			for (const auto& jUri : jNotifications[MSG_KEY_RESOURCE_SUBSCRIPTIONS])
			{
				if (jUri.isString())
					vecResourceSubscriptions.push_back(jUri.asString());
			}
		}

		return ERRNO_OK;
	}

	bool SubscriptionsListenRequest::IsValid() const
	{
		if (!Request::IsValid())
			return false;

		return strMethod.compare(METHOD_SUBSCRIPTIONS_LISTEN) == 0;
	}
}