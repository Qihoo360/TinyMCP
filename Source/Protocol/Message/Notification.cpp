#include "Notification.h"
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

		if (!jMsg.isMember(MSG_KEY_METHOD))
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
		Json::Value jProgress(iProgress);
		jParams[MSG_KEY_PROGRESS] = jProgress;
		if (iTotal != -1)
		{
			Json::Value jTotal(iTotal);
			jParams[MSG_KEY_TOTAL] = jTotal;
		}
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
		if (!jParams.isMember(MSG_KEY_PROGRESS) || !jParams[MSG_KEY_PROGRESS].isInt())
			return ERRNO_INVALID_NOTIFICATION;
		iProgress = jParams[MSG_KEY_PROGRESS].asInt();
		if (jParams.isMember(MSG_KEY_TOTAL) && jParams[MSG_KEY_TOTAL].isInt())
		{
			iTotal = jParams[MSG_KEY_TOTAL].asInt();
		}

		return progressToken.DoDeserialize(jParams);
	}

	bool ProgressNotification::IsValid() const
	{
		if (!Notification::IsValid())
			return false;

		if (strMethod.compare(METHOD_NOTIFICATION_PROGRESS) != 0)
			return false;

		if (-1 == iProgress)
			return false;

		return progressToken.IsValid();
	}
}