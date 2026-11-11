#include "Notification.h"
#include "../Internal/Protocol.h"
#include <string>

namespace MCP
{
	////////////////////////////////////////////////////////////////////////////////////////
	// Notification
	int Notification::DoSerialize(Json::Value& jMsg) const
	{
		if (!IsValid())
			return ERRNO_INVALID_NOTIFICATION;

		Json::Value jRPC(JSON_RPC_VER);
		jMsg[MSG_KEY_JSONRPC] = jRPC;

		Json::Value jMethod(strMethod);
		jMsg[MSG_KEY_METHOD] = jMethod;

		return ERRNO_OK;
	}

	int Notification::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_JSONRPC) || !jMsg[MSG_KEY_JSONRPC].isString())
			return ERRNO_INVALID_NOTIFICATION;
		auto strRPCVer = jMsg[MSG_KEY_JSONRPC].asString();
		if (strRPCVer.compare(JSON_RPC_VER) != 0)
			return ERRNO_INVALID_NOTIFICATION;

		if (!jMsg.isMember(MSG_KEY_METHOD) || !jMsg[MSG_KEY_METHOD].isString())
			return ERRNO_INVALID_NOTIFICATION;
		strMethod = jMsg[MSG_KEY_METHOD].asString();
		if (strMethod.empty())
			return ERRNO_INVALID_NOTIFICATION;

		return ERRNO_OK;
	}

	bool Notification::IsValid() const
	{
		if (strMethod.empty())
			return false;

		return true;
	}

	void Notification::WriteVersionParamsMeta(Json::Value& jParams, const ProtocolContext& ctxProtocol) const
	{
		if (!ctxProtocol.Supports(Feature_ListenSubscriptions))
			return;

		if (!bHasSubscriptionId || !subscriptionId.IsValid())
			return;

		Json::Value jValue;
		if (ERRNO_OK != subscriptionId.WriteValue(jValue) || jValue.isNull())
			return;

		Json::Value jMeta(Json::objectValue);
		if (jParams.isMember(MSG_KEY_META) && jParams[MSG_KEY_META].isObject())
			jMeta = jParams[MSG_KEY_META];

		jMeta[META_KEY_SUBSCRIPTION_ID] = jValue;
		jParams[MSG_KEY_META] = jMeta;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// InitializedNotification
	bool InitializedNotification::IsValid() const
	{
		if (strMethod.compare(METHOD_NOTIFICATION_INITIALIZED) != 0)
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// CancelledNotification
	int CancelledNotification::DoSerialize(Json::Value& jMsg) const
	{
		int iErrCode = Notification::DoSerialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		Json::Value jParams(Json::objectValue);
		requestId.DoSerialize(jParams);
		WriteVersionParamsMeta(jParams, GetProtocolContext());
		jMsg[MSG_KEY_PARAMS] = jParams;

		return ERRNO_OK;
	}

	int CancelledNotification::DoDeserialize(const Json::Value& jMsg)
	{
		int iErrCode = Notification::DoDeserialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;
		
		if (!jMsg.isMember(MSG_KEY_PARAMS) || !jMsg[MSG_KEY_PARAMS].isObject())
			return ERRNO_INVALID_NOTIFICATION;
		auto& jParams = jMsg[MSG_KEY_PARAMS];

		requestId.SetMsgKey(MSG_KEY_REQUEST_ID);
		return requestId.DoDeserialize(jParams);
	}

	bool CancelledNotification::IsValid() const
	{
		if (!Notification::IsValid())
			return false;

		if (strMethod.compare(METHOD_NOTIFICATION_CANCELLED) != 0)
			return false;

		return requestId.IsValid();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProgressNotification
	int ProgressNotification::DoSerialize(Json::Value& jMsg) const
	{
		int iErrCode = Notification::DoSerialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		Json::Value jParams(Json::objectValue);
		progressToken.DoSerialize(jParams);
		// Serialized as a JSON number; integral values keep their legacy spelling.
		jParams[MSG_KEY_PROGRESS] = MakeJsonNumber(iProgress);
		if (iTotal >= 0)
			jParams[MSG_KEY_TOTAL] = MakeJsonNumber(iTotal);
		WriteVersionParamsMeta(jParams, GetProtocolContext());
		jMsg[MSG_KEY_PARAMS] = jParams;
		
		return ERRNO_OK;
	}

	int ProgressNotification::DoDeserialize(const Json::Value& jMsg)
	{
		int iErrCode = Notification::DoDeserialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		if (!jMsg.isMember(MSG_KEY_PARAMS) || !jMsg[MSG_KEY_PARAMS].isObject())
			return ERRNO_INVALID_NOTIFICATION;
		auto& jParams = jMsg[MSG_KEY_PARAMS];
		if (!jParams.isMember(MSG_KEY_PROGRESS) || !jParams[MSG_KEY_PROGRESS].isNumeric())
			return ERRNO_INVALID_NOTIFICATION;
		iProgress = jParams[MSG_KEY_PROGRESS].asDouble();
		if (jParams.isMember(MSG_KEY_TOTAL) && jParams[MSG_KEY_TOTAL].isNumeric())
		{
			iTotal = jParams[MSG_KEY_TOTAL].asDouble();
		}

		return progressToken.DoDeserialize(jParams);
	}

	bool ProgressNotification::IsValid() const
	{
		if (!Notification::IsValid())
			return false;

		if (strMethod.compare(METHOD_NOTIFICATION_PROGRESS) != 0)
			return false;

		if (iProgress < 0)
			return false;

		return progressToken.IsValid();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ResourceListChangedNotification
	bool ResourceListChangedNotification::IsValid() const
	{
		if (!Notification::IsValid())
			return false;

		if (strMethod.compare(METHOD_NOTIFICATION_RESOURCES_LIST_CHANGED) != 0)
			return false;

		return true;
	}

	int ResourceListChangedNotification::DoSerialize(Json::Value& jMsg) const
	{
		int iErrCode = Notification::DoSerialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		Json::Value jParams(Json::objectValue);
		WriteVersionParamsMeta(jParams, GetProtocolContext());
		if (!jParams.empty())
			jMsg[MSG_KEY_PARAMS] = jParams;

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ResourceUpdatedNotification
	int ResourceUpdatedNotification::DoSerialize(Json::Value& jMsg) const
	{
		int iErrCode = Notification::DoSerialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		Json::Value jParams(Json::objectValue);
		Json::Value jUri(strUri);
		jParams[MSG_KEY_URI] = jUri;
		WriteVersionParamsMeta(jParams, GetProtocolContext());
		jMsg[MSG_KEY_PARAMS] = jParams;

		return ERRNO_OK;
	}

	int ResourceUpdatedNotification::DoDeserialize(const Json::Value& jMsg)
	{
		int iErrCode = Notification::DoDeserialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		if (!jMsg.isMember(MSG_KEY_PARAMS) || !jMsg[MSG_KEY_PARAMS].isObject())
			return ERRNO_INVALID_NOTIFICATION;
		auto& jParams = jMsg[MSG_KEY_PARAMS];
		if (!jParams.isMember(MSG_KEY_URI) || !jParams[MSG_KEY_URI].isString())
			return ERRNO_INVALID_NOTIFICATION;
		strUri = jParams[MSG_KEY_URI].asString();

		return ERRNO_OK;
	}

	bool ResourceUpdatedNotification::IsValid() const
	{
		if (!Notification::IsValid())
			return false;

		if (strMethod.compare(METHOD_NOTIFICATION_RESOURCES_UPDATED) != 0)
			return false;

		if (strUri.empty())
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// PromptListChangedNotification
	bool PromptListChangedNotification::IsValid() const
	{
		if (!Notification::IsValid())
			return false;

		if (strMethod.compare(METHOD_NOTIFICATION_PROMPTS_LIST_CHANGED) != 0)
			return false;

		return true;
	}

	int PromptListChangedNotification::DoSerialize(Json::Value& jMsg) const
	{
		int iErrCode = Notification::DoSerialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		Json::Value jParams(Json::objectValue);
		WriteVersionParamsMeta(jParams, GetProtocolContext());
		if (!jParams.empty())
			jMsg[MSG_KEY_PARAMS] = jParams;

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ToolListChangedNotification
	bool ToolListChangedNotification::IsValid() const
	{
		if (!Notification::IsValid())
			return false;

		if (strMethod.compare(METHOD_NOTIFICATION_TOOLS_LIST_CHANGED) != 0)
			return false;

		return true;
	}

	int ToolListChangedNotification::DoSerialize(Json::Value& jMsg) const
	{
		int iErrCode = Notification::DoSerialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		Json::Value jParams(Json::objectValue);
		WriteVersionParamsMeta(jParams, GetProtocolContext());
		if (!jParams.empty())
			jMsg[MSG_KEY_PARAMS] = jParams;

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// SubscriptionsAcknowledgedNotification
	int SubscriptionsAcknowledgedNotification::DoSerialize(Json::Value& jMsg) const
	{
		int iErrCode = Notification::DoSerialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		// notifications is a required member of the acknowledgment params, even
		// when the server grants nothing.
		Json::Value jParams(Json::objectValue);
		Json::Value jNotifications(Json::objectValue);
		filters.DoSerialize(jNotifications);
		jParams[MSG_KEY_NOTIFICATIONS] = jNotifications;

		WriteVersionParamsMeta(jParams, GetProtocolContext());

		jMsg[MSG_KEY_PARAMS] = jParams;

		return ERRNO_OK;
	}

	int SubscriptionsAcknowledgedNotification::DoDeserialize(const Json::Value& jMsg)
	{
		int iErrCode = Notification::DoDeserialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		if (!jMsg.isMember(MSG_KEY_PARAMS) || !jMsg[MSG_KEY_PARAMS].isObject())
			return ERRNO_OK;

		auto& jParams = jMsg[MSG_KEY_PARAMS];
		if (jParams.isMember(MSG_KEY_NOTIFICATIONS) && jParams[MSG_KEY_NOTIFICATIONS].isObject())
			filters.DoDeserialize(jParams[MSG_KEY_NOTIFICATIONS]);

		if (jParams.isMember(MSG_KEY_META) && jParams[MSG_KEY_META].isObject())
		{
			auto& jMeta = jParams[MSG_KEY_META];
			if (jMeta.isMember(META_KEY_SUBSCRIPTION_ID))
			{
				bHasSubscriptionId = true;
				subscriptionId.SetMsgKey(META_KEY_SUBSCRIPTION_ID);
				subscriptionId.DoDeserialize(jMeta);
			}
		}

		return ERRNO_OK;
	}

	bool SubscriptionsAcknowledgedNotification::IsValid() const
	{
		if (!Notification::IsValid())
			return false;

		return strMethod.compare(METHOD_NOTIFICATION_SUBSCRIPTIONS_ACKNOWLEDGED) == 0;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// LoggingMessageNotification
	int LoggingMessageNotification::DoSerialize(Json::Value& jMsg) const
	{
		int iErrCode = Notification::DoSerialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		Json::Value jParams(Json::objectValue);
		level.DoSerialize(jParams);
		if (!strLogger.empty())
		{
			Json::Value jLogger(strLogger);
			jParams[MSG_KEY_LOGGER] = jLogger;
		}
		if (!jData.isNull())
		{
			jParams[MSG_KEY_DATA] = jData;
		}
		WriteVersionParamsMeta(jParams, GetProtocolContext());
		jMsg[MSG_KEY_PARAMS] = jParams;

		return ERRNO_OK;
	}

	int LoggingMessageNotification::DoDeserialize(const Json::Value& jMsg)
	{
		return Notification::DoDeserialize(jMsg);
	}

	bool LoggingMessageNotification::IsValid() const
	{
		if (!Notification::IsValid())
			return false;

		if (strMethod.compare(METHOD_NOTIFICATION_MESSAGE) != 0)
			return false;

		if (!level.IsValid())
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// SubscriptionFilters
	bool SubscriptionFilters::HasValue() const
	{
		return bToolsListChanged
			|| bPromptsListChanged
			|| bResourcesListChanged
			|| !vecResourceSubscriptions.empty();
	}

	bool SubscriptionFilters::IsValid() const
	{
		return true;
	}

	int SubscriptionFilters::DoSerialize(Json::Value& jMsg) const
	{
		if (!jMsg.isObject())
			jMsg = Json::Value(Json::objectValue);

		if (bToolsListChanged)
			jMsg[MSG_KEY_TOOLS_LIST_CHANGED] = true;
		if (bPromptsListChanged)
			jMsg[MSG_KEY_PROMPTS_LIST_CHANGED] = true;
		if (bResourcesListChanged)
			jMsg[MSG_KEY_RESOURCES_LIST_CHANGED] = true;

		if (!vecResourceSubscriptions.empty())
		{
			Json::Value jSubscriptions(Json::arrayValue);
			for (const auto& strUri : vecResourceSubscriptions)
				jSubscriptions.append(strUri);
			jMsg[MSG_KEY_RESOURCE_SUBSCRIPTIONS] = jSubscriptions;
		}

		return ERRNO_OK;
	}

	int SubscriptionFilters::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isObject())
			return ERRNO_PARSE_ERROR;

		bToolsListChanged = jMsg.isMember(MSG_KEY_TOOLS_LIST_CHANGED)
			&& jMsg[MSG_KEY_TOOLS_LIST_CHANGED].isBool()
			&& jMsg[MSG_KEY_TOOLS_LIST_CHANGED].asBool();
		bPromptsListChanged = jMsg.isMember(MSG_KEY_PROMPTS_LIST_CHANGED)
			&& jMsg[MSG_KEY_PROMPTS_LIST_CHANGED].isBool()
			&& jMsg[MSG_KEY_PROMPTS_LIST_CHANGED].asBool();
		bResourcesListChanged = jMsg.isMember(MSG_KEY_RESOURCES_LIST_CHANGED)
			&& jMsg[MSG_KEY_RESOURCES_LIST_CHANGED].isBool()
			&& jMsg[MSG_KEY_RESOURCES_LIST_CHANGED].asBool();

		vecResourceSubscriptions.clear();
		if (jMsg.isMember(MSG_KEY_RESOURCE_SUBSCRIPTIONS) && jMsg[MSG_KEY_RESOURCE_SUBSCRIPTIONS].isArray())
		{
			for (const auto& jUri : jMsg[MSG_KEY_RESOURCE_SUBSCRIPTIONS])
			{
				if (jUri.isString())
					vecResourceSubscriptions.push_back(jUri.asString());
			}
		}

		return ERRNO_OK;
	}
}