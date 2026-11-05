#include "../TestHarness.h"

#include <Message/BasicMessage.h>
#include <Message/Notification.h>
#include <Public/PublicDef.h>

#include <string>

using namespace MCP;

namespace
{
	ProtocolContext MakeLegacyContext()
	{
		return ProtocolContext();
	}

	ProtocolContext MakeModernContext()
	{
		ProtocolContext ctxProtocol;
		const ProtocolProfile* pProfile = CProtocolRegistry::GetGlobalRegistry().Find(PROTOCOL_VER_2026_07_28);
		if (pProfile)
		{
			ctxProtocol.strVersion = pProfile->strVersion;
			ctxProtocol.pProfile = pProfile;
		}
		return ctxProtocol;
	}

	RequestId MakeRequestId(int iId)
	{
		RequestId requestId;
		requestId.eIdDataType = DataType_Integer;
		requestId.iId = iId;
		return requestId;
	}

	Json::Value SerializeNotification(const Message& msg)
	{
		Json::Value jMsg(Json::objectValue);
		int iErrCode = msg.DoSerialize(jMsg);
		CHECK_EQ(ERRNO_OK, iErrCode);
		return jMsg;
	}
}

TEST_CASE("unit.notification")
{
	const ProtocolContext ctxLegacy = MakeLegacyContext();
	const ProtocolContext ctxModern = MakeModernContext();

	// subscriptions/acknowledged carries the granted subset plus the
	// subscription id; a string id keeps its JSON type.
	SubscriptionsAcknowledgedNotification ack(false);
	ack.strMethod = METHOD_NOTIFICATION_SUBSCRIPTIONS_ACKNOWLEDGED;
	ack.ctxProtocol = ctxModern;
	ack.bHasSubscriptionId = true;
	ack.jSubscriptionId = Json::Value("sub-1");
	ack.jNotifications[MSG_KEY_TOOLS_LIST_CHANGED] = true;

	Json::Value jAck = SerializeNotification(ack);
	CHECK_EQ(std::string("sub-1"), jAck[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asString());
	CHECK(jAck[MSG_KEY_PARAMS][MSG_KEY_NOTIFICATIONS][MSG_KEY_TOOLS_LIST_CHANGED].asBool());

	SubscriptionsAcknowledgedNotification parsed(false);
	parsed.ctxProtocol = ctxModern;
	CHECK_EQ(ERRNO_OK, parsed.DoDeserialize(jAck));
	CHECK(parsed.bHasSubscriptionId);
	CHECK_EQ(std::string("sub-1"), parsed.jSubscriptionId.asString());

	// Integer subscription ids also survive the round trip.
	ack.jSubscriptionId = Json::Value(99);
	Json::Value jIntAck = SerializeNotification(ack);
	CHECK(jIntAck[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].isInt());
	CHECK_EQ(99, jIntAck[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asInt());

	// Legacy output never carries a subscription id.
	ack.ctxProtocol = ctxLegacy;
	Json::Value jLegacyAck = SerializeNotification(ack);
	CHECK(!jLegacyAck[MSG_KEY_PARAMS].isMember(MSG_KEY_META));
	CHECK(jLegacyAck[MSG_KEY_PARAMS].isMember(MSG_KEY_NOTIFICATIONS));

	// Resource update notifications gain params._meta.subscriptionId only in
	// the stateless era.
	ResourceUpdatedNotification updated(false);
	updated.strMethod = METHOD_NOTIFICATION_RESOURCES_UPDATED;
	updated.strUri = "file:///one.txt";
	updated.bHasSubscriptionId = true;
	updated.jSubscriptionId = Json::Value("sub-2");
	updated.ctxProtocol = ctxModern;

	Json::Value jUpdated = SerializeNotification(updated);
	CHECK_EQ(std::string("file:///one.txt"), jUpdated[MSG_KEY_PARAMS][MSG_KEY_URI].asString());
	CHECK_EQ(std::string("sub-2"), jUpdated[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asString());

	updated.ctxProtocol = ctxLegacy;
	Json::Value jLegacyUpdated = SerializeNotification(updated);
	CHECK(!jLegacyUpdated[MSG_KEY_PARAMS].isMember(MSG_KEY_META));

	// Cancelled notifications keep the original params and add the id.
	CancelledNotification cancelled(false);
	cancelled.strMethod = METHOD_NOTIFICATION_CANCELLED;
	cancelled.requestId = MakeRequestId(3);
	cancelled.bHasSubscriptionId = true;
	cancelled.jSubscriptionId = Json::Value("sub-3");
	cancelled.ctxProtocol = ctxModern;

	Json::Value jCancelled = SerializeNotification(cancelled);
	CHECK_EQ(3, jCancelled[MSG_KEY_PARAMS][MSG_KEY_ID].asInt());
	CHECK_EQ(std::string("sub-3"), jCancelled[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asString());

	// notifications/tools/list_changed uses the dedicated method constant.
	ToolListChangedNotification toolChanged(false);
	toolChanged.strMethod = METHOD_NOTIFICATION_TOOLS_LIST_CHANGED;
	CHECK(toolChanged.IsValid());

	// Logging notifications keep their level/logger/data layout untouched.
	LoggingMessageNotification logging(false);
	logging.strMethod = METHOD_NOTIFICATION_MESSAGE;
	logging.level.eLevel = LoggingLevelValue::LoggingLevel_Info;
	logging.strLogger = "tinymcp";
	logging.jData["message"] = "hello";
	logging.ctxProtocol = ctxLegacy;

	Json::Value jLogging = SerializeNotification(logging);
	CHECK_EQ(std::string(LOGGING_LEVEL_INFO), jLogging[MSG_KEY_PARAMS][MSG_KEY_LEVEL].asString());
	CHECK_EQ(std::string("tinymcp"), jLogging[MSG_KEY_PARAMS][MSG_KEY_LOGGER].asString());
	CHECK_EQ(std::string("hello"), jLogging[MSG_KEY_PARAMS][MSG_KEY_DATA]["message"].asString());
	CHECK(!jLogging[MSG_KEY_PARAMS].isMember(MSG_KEY_META));
}
