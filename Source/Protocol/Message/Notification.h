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
		MCP::RequestId subscriptionId;
		bool bHasSubscriptionId{ false };

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;

	protected:
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

		// Adds params._meta.subscriptionId when a subscription is attached.
		void WriteVersionParamsMeta(Json::Value& jParams, const ProtocolContext& ctxProtocol) const;
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
		int DoSerialize(Json::Value& jMsg) const override;
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

	struct PromptListChangedNotification : public MCP::Notification
	{
	public:
		PromptListChangedNotification(bool bNeedIdentity)
			: Notification(MessageType_PromptListChangedNotification, bNeedIdentity)
		{

		}

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
	};

	struct ToolListChangedNotification : public MCP::Notification
	{
	public:
		ToolListChangedNotification(bool bNeedIdentity)
			: Notification(MessageType_ToolListChangedNotification, bNeedIdentity)
		{

		}

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
	};

	// The filters a subscriptions/listen acknowledgment granted. Unset booleans
	// are omitted, so an empty grant serializes as an empty object.
	struct SubscriptionFilters
	{
	public:
		bool bToolsListChanged{ false };
		bool bPromptsListChanged{ false };
		bool bResourcesListChanged{ false };
		std::vector<std::string> vecResourceSubscriptions;

		bool HasValue() const;
		bool IsValid() const;
		int DoSerialize(Json::Value& jMsg) const;
		int DoDeserialize(const Json::Value& jMsg);
	};

	// 2026-07-28 subscriptions/acknowledged notification.
	struct SubscriptionsAcknowledgedNotification : public MCP::Notification
	{
	public:
		SubscriptionsAcknowledgedNotification(bool bNeedIdentity)
			: Notification(MessageType_SubscriptionsAcknowledgedNotification, bNeedIdentity)
		{

		}

		MCP::SubscriptionFilters filters;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct LoggingMessageNotification : public MCP::Notification
	{
	public:
		LoggingMessageNotification(bool bNeedIdentity)
			: Notification(MessageType_LoggingMessageNotification, bNeedIdentity)
		{

		}

		MCP::LoggingLevel level;
		std::string strLogger;
		Json::Value jData;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};
}
