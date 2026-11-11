#include "../TestHarness.h"

#include <Message/BasicMessage.h>
#include <Message/Notification.h>
#include <Public/PublicDef.h>
#include <Internal/Protocol.h>
#include <Internal/MessageAccess.h>

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
		const std::shared_ptr<const ProtocolProfile> pProfile = CProtocolRegistry::GetGlobalRegistry().Find(PROTOCOL_VER_2026_07_28);
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

	RequestId MakeStringRequestId(const std::string& strId)
	{
		RequestId requestId;
		requestId.eIdDataType = DataType_String;
		requestId.strId = strId;
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
	MessageContextAccess::Set(ack, ctxModern);
	ack.bHasSubscriptionId = true;
	ack.subscriptionId = MakeStringRequestId("sub-1");
	ack.filters.bToolsListChanged = true;

	Json::Value jAck = SerializeNotification(ack);
	CHECK_EQ(std::string("sub-1"), jAck[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asString());
	CHECK(jAck[MSG_KEY_PARAMS][MSG_KEY_NOTIFICATIONS][MSG_KEY_TOOLS_LIST_CHANGED].asBool());

	SubscriptionsAcknowledgedNotification parsed(false);
	MessageContextAccess::Set(parsed, ctxModern);
	CHECK_EQ(ERRNO_OK, parsed.DoDeserialize(jAck));
	CHECK(parsed.bHasSubscriptionId);
	CHECK_EQ(std::string("sub-1"), parsed.subscriptionId.strId);
	CHECK(parsed.filters.bToolsListChanged);

	// Integer subscription ids also survive the round trip.
	ack.subscriptionId = MakeRequestId(99);
	Json::Value jIntAck = SerializeNotification(ack);
	CHECK(jIntAck[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].isInt());
	CHECK_EQ(99, jIntAck[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asInt());

	// Subscription filters only publish what was granted; an empty grant still
	// serializes the required notifications object.
	SubscriptionFilters emptyGrant;
	Json::Value jEmptyGrant(Json::objectValue);
	CHECK_EQ(ERRNO_OK, emptyGrant.DoSerialize(jEmptyGrant));
	CHECK(jEmptyGrant.empty());
	CHECK(!emptyGrant.HasValue());

	SubscriptionFilters filters;
	filters.bToolsListChanged = true;
	filters.vecResourceSubscriptions.push_back("file:///one.txt");
	Json::Value jFilters(Json::objectValue);
	CHECK_EQ(ERRNO_OK, filters.DoSerialize(jFilters));
	CHECK(jFilters[MSG_KEY_TOOLS_LIST_CHANGED].asBool());
	CHECK(!jFilters.isMember(MSG_KEY_PROMPTS_LIST_CHANGED));
	CHECK(!jFilters.isMember(MSG_KEY_RESOURCES_LIST_CHANGED));
	CHECK_EQ(std::string("file:///one.txt"), jFilters[MSG_KEY_RESOURCE_SUBSCRIPTIONS][0].asString());
	CHECK(filters.HasValue());

	SubscriptionFilters parsedFilters;
	CHECK_EQ(ERRNO_OK, parsedFilters.DoDeserialize(jFilters));
	CHECK(parsedFilters.bToolsListChanged);
	CHECK_EQ(static_cast<size_t>(1), parsedFilters.vecResourceSubscriptions.size());
	CHECK_EQ(ERRNO_PARSE_ERROR, parsedFilters.DoDeserialize(Json::Value(1)));

	// RequestId::WriteValue keeps the scalar type without wrapping it in a key.
	Json::Value jStringId;
	CHECK_EQ(ERRNO_OK, MakeStringRequestId("sub").WriteValue(jStringId));
	CHECK(jStringId.isString());
	CHECK_EQ(std::string("sub"), jStringId.asString());

	Json::Value jIntId;
	CHECK_EQ(ERRNO_OK, MakeRequestId(5).WriteValue(jIntId));
	CHECK(jIntId.isInt());
	CHECK_EQ(5, jIntId.asInt());

	// Legacy output never carries a subscription id.
	MessageContextAccess::Set(ack, ctxLegacy);
	Json::Value jLegacyAck = SerializeNotification(ack);
	CHECK(!jLegacyAck[MSG_KEY_PARAMS].isMember(MSG_KEY_META));
	CHECK(jLegacyAck[MSG_KEY_PARAMS].isMember(MSG_KEY_NOTIFICATIONS));

	// Resource update notifications gain params._meta.subscriptionId only in
	// the stateless era.
	ResourceUpdatedNotification updated(false);
	updated.strMethod = METHOD_NOTIFICATION_RESOURCES_UPDATED;
	updated.strUri = "file:///one.txt";
	updated.bHasSubscriptionId = true;
	updated.subscriptionId = MakeStringRequestId("sub-2");
	MessageContextAccess::Set(updated, ctxModern);

	Json::Value jUpdated = SerializeNotification(updated);
	CHECK_EQ(std::string("file:///one.txt"), jUpdated[MSG_KEY_PARAMS][MSG_KEY_URI].asString());
	CHECK_EQ(std::string("sub-2"), jUpdated[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asString());

	MessageContextAccess::Set(updated, ctxLegacy);
	Json::Value jLegacyUpdated = SerializeNotification(updated);
	CHECK(!jLegacyUpdated[MSG_KEY_PARAMS].isMember(MSG_KEY_META));

	// Cancelled notifications keep the original params and add the id.
	CancelledNotification cancelled(false);
	cancelled.strMethod = METHOD_NOTIFICATION_CANCELLED;
	cancelled.requestId = MakeRequestId(3);
	cancelled.bHasSubscriptionId = true;
	cancelled.subscriptionId = MakeStringRequestId("sub-3");
	MessageContextAccess::Set(cancelled, ctxModern);

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
	MessageContextAccess::Set(logging, ctxLegacy);

	Json::Value jLogging = SerializeNotification(logging);
	CHECK_EQ(std::string(LOGGING_LEVEL_INFO), jLogging[MSG_KEY_PARAMS][MSG_KEY_LEVEL].asString());
	CHECK_EQ(std::string("tinymcp"), jLogging[MSG_KEY_PARAMS][MSG_KEY_LOGGER].asString());
	CHECK_EQ(std::string("hello"), jLogging[MSG_KEY_PARAMS][MSG_KEY_DATA]["message"].asString());
	CHECK(!jLogging[MSG_KEY_PARAMS].isMember(MSG_KEY_META));
}
