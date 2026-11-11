#include "../TestHarness.h"

#include <Message/BasicMessage.h>
#include <Message/Response.h>
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

	Json::Value SerializeMessage(const Message& msg)
	{
		Json::Value jMsg(Json::objectValue);
		int iErrCode = msg.DoSerialize(jMsg);
		CHECK_EQ(ERRNO_OK, iErrCode);
		return jMsg;
	}
}

TEST_CASE("unit.response")
{
	const ProtocolContext ctxLegacy = MakeLegacyContext();
	const ProtocolContext ctxModern = MakeModernContext();

	// Legacy results must not publish any 2026-07-28 result field, even when the
	// modern members are populated by mistake.
	ListToolsResult legacyResult(false);
	legacyResult.requestId = MakeRequestId(7);
	MessageContextAccess::Set(legacyResult, ctxLegacy);
	legacyResult.iTtlMs = 1500;
	legacyResult.strCacheScope = CACHE_SCOPE_PRIVATE;
	legacyResult.jInputRequests[MSG_KEY_TYPE] = "confirm";
	legacyResult.strRequestState = "legacy-state";
	legacyResult.jServerInfo[MSG_KEY_NAME] = "tinymcp";

	Json::Value jLegacy = SerializeMessage(legacyResult);
	const Json::Value& jLegacyResult = jLegacy[MSG_KEY_RESULT];
	CHECK(!jLegacyResult.isMember(MSG_KEY_RESULT_TYPE));
	CHECK(!jLegacyResult.isMember(MSG_KEY_TTL_MS));
	CHECK(!jLegacyResult.isMember(MSG_KEY_CACHE_SCOPE));
	CHECK(!jLegacyResult.isMember(MSG_KEY_INPUT_REQUESTS));
	CHECK(!jLegacyResult.isMember(MSG_KEY_REQUEST_STATE));
	CHECK(!jLegacyResult.isMember(MSG_KEY_META));

	// The modern profile publishes resultType, cache hints, MRTR state and the
	// serverInfo merged into result._meta.
	ListToolsResult modernResult(false);
	modernResult.requestId = MakeRequestId(8);
	MessageContextAccess::Set(modernResult, ctxModern);
	modernResult.iTtlMs = 2500;
	modernResult.strCacheScope = CACHE_SCOPE_PUBLIC;
	modernResult.jInputRequests["confirm"] = "please";
	modernResult.strRequestState = "state-token";
	modernResult.jServerInfo[MSG_KEY_NAME] = "tinymcp";
	modernResult.jServerInfo[MSG_KEY_VERSION] = "1.0.0";
	modernResult.jResultMeta["application/trace"] = "trace-1";

	Json::Value jModern = SerializeMessage(modernResult);
	const Json::Value& jModernResult = jModern[MSG_KEY_RESULT];
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE), jModernResult[MSG_KEY_RESULT_TYPE].asString());
	CHECK_EQ(2500, jModernResult[MSG_KEY_TTL_MS].asInt());
	CHECK_EQ(std::string(CACHE_SCOPE_PUBLIC), jModernResult[MSG_KEY_CACHE_SCOPE].asString());
	CHECK_EQ(std::string("please"), jModernResult[MSG_KEY_INPUT_REQUESTS]["confirm"].asString());
	CHECK_EQ(std::string("state-token"), jModernResult[MSG_KEY_REQUEST_STATE].asString());
	CHECK_EQ(std::string("trace-1"), jModernResult[MSG_KEY_META]["application/trace"].asString());
	CHECK_EQ(std::string("tinymcp"), jModernResult[MSG_KEY_META][META_KEY_SERVER_INFO][MSG_KEY_NAME].asString());
	CHECK_EQ(std::string("1.0.0"), jModernResult[MSG_KEY_META][META_KEY_SERVER_INFO][MSG_KEY_VERSION].asString());

	// An MRTR retry round trip must not reuse the cache hints.
	modernResult.bMrtrRetry = true;
	Json::Value jRetry = SerializeMessage(modernResult);
	CHECK(!jRetry[MSG_KEY_RESULT].isMember(MSG_KEY_TTL_MS));
	CHECK(!jRetry[MSG_KEY_RESULT].isMember(MSG_KEY_CACHE_SCOPE));
	CHECK(jRetry[MSG_KEY_RESULT].isMember(MSG_KEY_INPUT_REQUESTS));

	// tools/call is not cacheable and structuredContent only exists in modern.
	CallToolResult toolResult(false);
	toolResult.requestId = MakeRequestId(9);
	MessageContextAccess::Set(toolResult, ctxModern);
	toolResult.iTtlMs = 1000;
	toolResult.strCacheScope = CACHE_SCOPE_PUBLIC;
	TextContent text;
	text.strText = "ok";
	text.strType = CONST_TEXT;
	toolResult.vecTextContent.push_back(text);
	Json::Value jStructured(Json::objectValue);
	jStructured[MSG_KEY_VALUE] = 42;
	toolResult.structuredContent.Set(jStructured);

	Json::Value jTool = SerializeMessage(toolResult);
	CHECK(!jTool[MSG_KEY_RESULT].isMember(MSG_KEY_TTL_MS));
	CHECK(!jTool[MSG_KEY_RESULT].isMember(MSG_KEY_CACHE_SCOPE));
	CHECK_EQ(42, jTool[MSG_KEY_RESULT][MSG_KEY_STRUCTURED_CONTENT][MSG_KEY_VALUE].asInt());

	CallToolResult legacyTool(toolResult);
	MessageContextAccess::Set(legacyTool, ctxLegacy);
	Json::Value jLegacyTool = SerializeMessage(legacyTool);
	CHECK(!jLegacyTool[MSG_KEY_RESULT].isMember(MSG_KEY_STRUCTURED_CONTENT));
	CHECK(!jLegacyTool[MSG_KEY_RESULT].isMember(MSG_KEY_RESULT_TYPE));

	// Reserved _meta keys owned by the SDK win over application supplied values,
	// while unrelated application keys survive the merge.
	SubscriptionsListenResult listenResult(false);
	listenResult.requestId = MakeRequestId(10);
	MessageContextAccess::Set(listenResult, ctxModern);
	listenResult.jSubscriptionId = Json::Value("sdk-subscription");
	listenResult.jResultMeta[META_KEY_SUBSCRIPTION_ID] = "application-subscription";
	listenResult.jResultMeta["application/key"] = "value";
	listenResult.jServerInfo[MSG_KEY_NAME] = "tinymcp";

	Json::Value jListen = SerializeMessage(listenResult);
	const Json::Value& jListenMeta = jListen[MSG_KEY_RESULT][MSG_KEY_META];
	CHECK_EQ(std::string("sdk-subscription"), jListenMeta[META_KEY_SUBSCRIPTION_ID].asString());
	CHECK_EQ(std::string("value"), jListenMeta["application/key"].asString());
	CHECK_EQ(std::string("tinymcp"), jListenMeta[META_KEY_SERVER_INFO][MSG_KEY_NAME].asString());

	// The subscription id keeps its original JSON type.
	listenResult.jSubscriptionId = Json::Value(77);
	Json::Value jIntListen = SerializeMessage(listenResult);
	CHECK(jIntListen[MSG_KEY_RESULT][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].isInt());
	CHECK_EQ(77, jIntListen[MSG_KEY_RESULT][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asInt());

	// Legacy results never carry a subscription id.
	MessageContextAccess::Set(listenResult, ctxLegacy);
	listenResult.jResultMeta = Json::Value();
	listenResult.jServerInfo = Json::Value();
	Json::Value jLegacyListen = SerializeMessage(listenResult);
	CHECK(!jLegacyListen[MSG_KEY_RESULT].isMember(MSG_KEY_META));

	// server/discover publishes the version list plus mandatory cache hints.
	DiscoverResult discover(false);
	discover.requestId = MakeRequestId(11);
	MessageContextAccess::Set(discover, ctxModern);
	discover.vecSupportedVersions.push_back(PROTOCOL_VER_2026_07_28);
	discover.iTtlMs = 5000;
	discover.strCacheScope = CACHE_SCOPE_PUBLIC;
	discover.strInstructions = "Use the modern profile.";
	discover.jServerInfo[MSG_KEY_NAME] = "tinymcp";
	CHECK(discover.IsValid());

	Json::Value jDiscover = SerializeMessage(discover);
	const Json::Value& jDiscoverResult = jDiscover[MSG_KEY_RESULT];
	CHECK_EQ(std::string(PROTOCOL_VER_2026_07_28), jDiscoverResult[MSG_KEY_SUPPORTED_VERSIONS][0].asString());
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE), jDiscoverResult[MSG_KEY_RESULT_TYPE].asString());
	CHECK_EQ(5000, jDiscoverResult[MSG_KEY_TTL_MS].asInt());
	CHECK_EQ(std::string(CACHE_SCOPE_PUBLIC), jDiscoverResult[MSG_KEY_CACHE_SCOPE].asString());
	CHECK_EQ(std::string("Use the modern profile."), jDiscoverResult[MSG_KEY_INSTRUCTIONS].asString());
	CHECK(jDiscoverResult[MSG_KEY_META].isMember(META_KEY_SERVER_INFO));

	// supportedVersions is mandatory.
	DiscoverResult emptyDiscover(false);
	emptyDiscover.requestId = MakeRequestId(12);
	CHECK(!emptyDiscover.IsValid());
}
