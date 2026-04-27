#pragma once
// To ensure good cross-platform compatibility, all code within the MCP namespace is written in standard C++.
// The use of platform-specific system APIs is prohibited unless necessary.

#include <json/json.h>
#include "BasicMessage.h"

namespace MCP
{
	struct Notification : public MCP::Message
	{
	public:
		Notification(MessageType eMsgType, bool bNeedIdentity)
			: Message(eMsgType, MessageCategory_Notification, bNeedIdentity)
		{

		}

		std::string strMethod;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct InitializedNotification : public MCP::Notification
	{
	public:
		InitializedNotification(bool bNeedIdentity)
			: Notification(MessageType_InitializedNotification, bNeedIdentity)
		{

		}

		bool IsValid() const override;
	};

	struct CancelledNotification : public MCP::Notification
	{
	public:
		CancelledNotification(bool bNeedIdentity)
			: Notification(MessageType_CancelledNotification, bNeedIdentity)
		{

		}

		MCP::RequestId requestId;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ProgressNotification : public MCP::Notification
	{
	public:
		ProgressNotification(bool bNeedIdentity)
			: Notification(MessageType_ProgressNotification, bNeedIdentity)
		{

		}

		MCP::ProgressToken progressToken;
		int iProgress{ -1 };
		int iTotal{ -1 };

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ResourceListChangedNotification : public MCP::Notification
	{
	public:
		ResourceListChangedNotification(bool bNeedIdentity)
			: Notification(MessageType_ResourceListChangedNotification, bNeedIdentity)
		{

		}

		bool IsValid() const override;
	};

	struct ResourceUpdatedNotification : public MCP::Notification
	{
	public:
		ResourceUpdatedNotification(bool bNeedIdentity)
			: Notification(MessageType_ResourceUpdatedNotification, bNeedIdentity)
		{

		}

		std::string strUri;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};
}
