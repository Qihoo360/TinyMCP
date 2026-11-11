#include "../TestHarness.h"

#include <Message/Request.h>
#include <Public/PublicDef.h>
#include <Internal/Protocol.h>
#include <Internal/MessageAccess.h>

#include <string>
#include <vector>

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

	Json::Value SerializeRequest(const Message& msg)
	{
		Json::Value jMsg(Json::objectValue);
		int iErrCode = msg.DoSerialize(jMsg);
		CHECK_EQ(ERRNO_OK, iErrCode);
		return jMsg;
	}
}

TEST_CASE("unit.request")
{
	const ProtocolContext ctxLegacy = MakeLegacyContext();
	const ProtocolContext ctxModern = MakeModernContext();

	// Legacy requests do not emit params._meta nor the MRTR fields.
	ListToolsRequest legacyRequest(false);
	legacyRequest.requestId = MakeRequestId(1);
	legacyRequest.strMethod = METHOD_TOOLS_LIST;
	MessageContextAccess::Set(legacyRequest, ctxLegacy);
	Json::Value jLegacyResponses(Json::objectValue);
	jLegacyResponses["confirm"] = "value";
	legacyRequest.inputResponses.Set(jLegacyResponses);
	legacyRequest.strRequestState = "legacy-state";

	Json::Value jLegacy = SerializeRequest(legacyRequest);
	CHECK(!jLegacy.isMember(MSG_KEY_PARAMS));

	// The same object with a modern context exposes the MRTR payload.
	MessageContextAccess::Set(legacyRequest, ctxModern);
	Json::Value jModernOut = SerializeRequest(legacyRequest);
	CHECK_EQ(std::string("value"), jModernOut[MSG_KEY_PARAMS][MSG_KEY_INPUT_RESPONSES]["confirm"].asString());
	CHECK_EQ(std::string("legacy-state"), jModernOut[MSG_KEY_PARAMS][MSG_KEY_REQUEST_STATE].asString());

	// Modern request metadata: version, client capabilities/info, logLevel and
	// unknown application keys are all parsed from params._meta.
	Json::Value jCapabilities(Json::objectValue);
	jCapabilities[MSG_KEY_EXTENSIONS] = Json::Value(Json::objectValue);

	Json::Value jClientInfo(Json::objectValue);
	jClientInfo[MSG_KEY_NAME] = "client";
	jClientInfo[MSG_KEY_VERSION] = "1.0.0";
	jClientInfo[MSG_KEY_TITLE] = "Test Client";

	Json::Value jMeta(Json::objectValue);
	jMeta[META_KEY_PROTOCOL_VERSION] = PROTOCOL_VER_2026_07_28;
	jMeta[META_KEY_CLIENT_CAPABILITIES] = jCapabilities;
	jMeta[META_KEY_CLIENT_INFO] = jClientInfo;
	jMeta[META_KEY_LOG_LEVEL] = LOGGING_LEVEL_DEBUG;
	jMeta["application/trace"] = "trace-1";

	Json::Value jInputResponses(Json::objectValue);
	jInputResponses["confirm"] = "yes";

	Json::Value jParams(Json::objectValue);
	jParams[MSG_KEY_META] = jMeta;
	jParams[MSG_KEY_INPUT_RESPONSES] = jInputResponses;
	jParams[MSG_KEY_REQUEST_STATE] = "state-token";

	Json::Value jMsg(Json::objectValue);
	jMsg[MSG_KEY_JSONRPC] = JSON_RPC_VER;
	jMsg[MSG_KEY_ID] = 2;
	jMsg[MSG_KEY_METHOD] = METHOD_TOOLS_LIST;
	jMsg[MSG_KEY_PARAMS] = jParams;

	ListToolsRequest modernRequest(false);
	MessageContextAccess::Set(modernRequest, ctxModern);
	CHECK_EQ(ERRNO_OK, modernRequest.DoDeserialize(jMsg));
	CHECK_EQ(std::string(PROTOCOL_VER_2026_07_28), modernRequest.meta.strProtocolVersion);
	CHECK(modernRequest.meta.bHasClientCapabilities);
	CHECK_EQ(std::string("client"), modernRequest.meta.clientInfo.strName);
	CHECK_EQ(std::string("Test Client"), modernRequest.meta.clientInfo.strTitle);
	CHECK_EQ(std::string(LOGGING_LEVEL_DEBUG), modernRequest.meta.strLogLevel);
	CHECK_EQ(std::string("trace-1"), modernRequest.meta.jExtra["application/trace"].asString());
	CHECK_EQ(std::string("yes"), modernRequest.inputResponses.Get()["confirm"].asString());
	CHECK_EQ(std::string("state-token"), modernRequest.strRequestState);

	// A handshake context drops the per-request logLevel and the MRTR fields.
	ListToolsRequest legacyParsed(false);
	MessageContextAccess::Set(legacyParsed, ctxLegacy);
	CHECK_EQ(ERRNO_OK, legacyParsed.DoDeserialize(jMsg));
	CHECK(legacyParsed.meta.strLogLevel.empty());
	CHECK(!legacyParsed.inputResponses.HasValue());
	CHECK(legacyParsed.strRequestState.empty());

	// server/discover requires params._meta once the profile exposes discover.
	DiscoverRequest discover(false);
	discover.requestId = MakeStringRequestId("discover-1");
	discover.strMethod = METHOD_SERVER_DISCOVER;
	MessageContextAccess::Set(discover, ctxModern);
	CHECK(!discover.IsValid());

	discover.meta.strProtocolVersion = PROTOCOL_VER_2026_07_28;
	CHECK(discover.IsValid());
	Json::Value jDiscover = SerializeRequest(discover);
	CHECK_EQ(std::string(PROTOCOL_VER_2026_07_28),
		jDiscover[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_PROTOCOL_VERSION].asString());

	// Without a stateless profile the same method stays a plain fallback request.
	DiscoverRequest legacyDiscover(false);
	legacyDiscover.requestId = MakeStringRequestId("discover-2");
	legacyDiscover.strMethod = METHOD_SERVER_DISCOVER;
	MessageContextAccess::Set(legacyDiscover, ctxLegacy);
	CHECK(legacyDiscover.IsValid());
	CHECK(!SerializeRequest(legacyDiscover).isMember(MSG_KEY_PARAMS));

	// subscriptions/listen filters are nested inside params.notifications.
	SubscriptionsListenRequest listen(false);
	listen.requestId = MakeStringRequestId("listen-1");
	listen.strMethod = METHOD_SUBSCRIPTIONS_LISTEN;
	MessageContextAccess::Set(listen, ctxModern);
	listen.meta.strProtocolVersion = PROTOCOL_VER_2026_07_28;
	listen.bToolsListChanged = true;
	listen.bResourcesListChanged = true;
	listen.vecResourceSubscriptions.push_back("file:///one.txt");

	Json::Value jListen = SerializeRequest(listen);
	const Json::Value& jNotifications = jListen[MSG_KEY_PARAMS][MSG_KEY_NOTIFICATIONS];
	CHECK(jNotifications[MSG_KEY_TOOLS_LIST_CHANGED].asBool());
	CHECK(jNotifications[MSG_KEY_RESOURCES_LIST_CHANGED].asBool());
	CHECK(!jNotifications.isMember(MSG_KEY_PROMPTS_LIST_CHANGED));
	CHECK_EQ(std::string("file:///one.txt"), jNotifications[MSG_KEY_RESOURCE_SUBSCRIPTIONS][0].asString());

	SubscriptionsListenRequest parsed(false);
	MessageContextAccess::Set(parsed, ctxModern);
	CHECK_EQ(ERRNO_OK, parsed.DoDeserialize(jListen));
	CHECK(parsed.bToolsListChanged);
	CHECK(!parsed.bPromptsListChanged);
	CHECK(parsed.bResourcesListChanged);
	CHECK_EQ(static_cast<size_t>(1), parsed.vecResourceSubscriptions.size());
	// Legacy initialize keeps params.capabilities.extensions queryable.
	Json::Value jLegacyCapabilities(Json::objectValue);
	jLegacyCapabilities[MSG_KEY_EXTENSIONS]["example/echo"] = Json::Value(Json::objectValue);

	Json::Value jInitializeParams(Json::objectValue);
	jInitializeParams[MSG_KEY_PROTOCOL_VERSION] = PROTOCOL_VER_2024_11_05;
	jInitializeParams[MSG_KEY_CAPABILITIES] = jLegacyCapabilities;
	jInitializeParams[MSG_KEY_CLIENT_INFO][MSG_KEY_NAME] = "legacy-client";
	jInitializeParams[MSG_KEY_CLIENT_INFO][MSG_KEY_VERSION] = "1.0.0";

	Json::Value jInitialize(Json::objectValue);
	jInitialize[MSG_KEY_JSONRPC] = JSON_RPC_VER;
	jInitialize[MSG_KEY_ID] = 3;
	jInitialize[MSG_KEY_METHOD] = METHOD_INITIALIZE;
	jInitialize[MSG_KEY_PARAMS] = jInitializeParams;

	InitializeRequest initialize(false);
	MessageContextAccess::Set(initialize, ctxLegacy);
	CHECK_EQ(ERRNO_OK, initialize.DoDeserialize(jInitialize));
	CHECK(initialize.clientCapabilities.SupportsExtension("example/echo"));
	CHECK(!initialize.clientCapabilities.SupportsExtension("example/missing"));

	// An MRTR retry keeps params._meta.toolCallId across a serialize/parse cycle.
	CallToolRequest retry(false);
	retry.requestId = MakeRequestId(4);
	retry.strMethod = METHOD_TOOLS_CALL;
	retry.strName = "echo";
	retry.toolCallId.eIdDataType = DataType_Integer;
	retry.toolCallId.iId = 77;
	MessageContextAccess::Set(retry, ctxModern);
	const Json::Value jRetry = SerializeRequest(retry);
	CHECK_EQ(77, jRetry[MSG_KEY_PARAMS][MSG_KEY_META][MSG_KEY_TOOL_CALL_ID].asInt());

	CallToolRequest parsedRetry(false);
	MessageContextAccess::Set(parsedRetry, ctxModern);
	CHECK_EQ(ERRNO_OK, parsedRetry.DoDeserialize(jRetry));
	CHECK_EQ(77.0, parsedRetry.toolCallId.iId);

	// JSON-RPC ids are string or number; fractional values are preserved and an
	// integral number keeps the legacy integer spelling.
	Json::Value jFractionalId(Json::objectValue);
	jFractionalId[MSG_KEY_ID] = 1.5;
	RequestId fractionalId;
	CHECK_EQ(ERRNO_OK, fractionalId.DoDeserialize(jFractionalId));
	CHECK_EQ(DataType_Integer, fractionalId.eIdDataType);
	Json::Value jFractionalIdOut(Json::objectValue);
	CHECK_EQ(ERRNO_OK, fractionalId.DoSerialize(jFractionalIdOut));
	CHECK_EQ(1.5, jFractionalIdOut[MSG_KEY_ID].asDouble());

	Json::Value jIntegralId(Json::objectValue);
	jIntegralId[MSG_KEY_ID] = 2.0;
	RequestId integralId;
	CHECK_EQ(ERRNO_OK, integralId.DoDeserialize(jIntegralId));
	Json::Value jIntegralIdOut(Json::objectValue);
	CHECK_EQ(ERRNO_OK, integralId.DoSerialize(jIntegralIdOut));
	CHECK(jIntegralIdOut[MSG_KEY_ID].isInt());
}
