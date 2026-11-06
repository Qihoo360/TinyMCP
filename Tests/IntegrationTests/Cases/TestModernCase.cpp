#include "../../TestHarness.h"
#include "../Driver.h"
#include "../TestServer.h"

#include <Message/Request.h>
#include <Public/PublicDef.h>
#include <Internal/Protocol.h>
#include <Session/Session.h>

#include <map>
#include <string>
#include <vector>

using namespace MCP;
using namespace MCP::Test;

namespace
{
	const int WAIT_TIMEOUT_MS = 5000;
	const int QUIET_WAIT_MS = 300;

	std::string MakeRequest(int iId, const std::string& strMethod, const std::string& strParams)
	{
		std::string strRequest = "{\"jsonrpc\":\"2.0\",\"id\":" + std::to_string(iId) + ",\"method\":\"" + strMethod + "\"";
		if (!strParams.empty())
			strRequest += ",\"params\":" + strParams;
		strRequest += "}";

		return strRequest;
	}

	std::string MakeModernParams(const std::string& strExtra = std::string(), const std::string& strCapabilities = "{}")
	{
		std::string strParams = "{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\","
			"\"io.modelcontextprotocol/clientCapabilities\":" + strCapabilities + "}";
		if (!strExtra.empty())
			strParams += "," + strExtra.substr(1);
		else
			strParams += "}";

		return strParams;
	}

	bool ParseJson(const std::string& strText, Json::Value& jValue)
	{
		Json::Reader reader;
		return reader.parse(strText, jValue) && jValue.isObject();
	}

	bool ExpectResponse(CDriver& driver, const std::string& strRequest, Json::Value& jResponse)
	{
		size_t nBefore = driver.GetOutputCount();
		if (ERRNO_OK != driver.Send(strRequest))
			return false;

		if (!driver.WaitForOutputCount(nBefore + 1, WAIT_TIMEOUT_MS))
			return false;

		return ParseJson(driver.GetOutput(nBefore), jResponse);
	}

	bool ExpectError(CDriver& driver, const std::string& strRequest, int iId, int iErrorCode, Json::Value& jResponse)
	{
		if (!ExpectResponse(driver, strRequest, jResponse))
			return false;

		CHECK_EQ(iId, jResponse[MSG_KEY_ID].asInt());
		CHECK(jResponse.isMember(MSG_KEY_ERROR));
		if (!jResponse.isMember(MSG_KEY_ERROR))
			return false;

		CHECK_EQ(iErrorCode, jResponse[MSG_KEY_ERROR][MSG_KEY_CODE].asInt());

		return iErrorCode == jResponse[MSG_KEY_ERROR][MSG_KEY_CODE].asInt();
	}

	bool StartServer(CDriver& driver)
	{
		// Every scenario starts from a pristine client state so that tests can run
		// in any order inside a single process (--unit).
		CMCPSession::GetInstance().ResetForNewClient();

		int iErrCode = CTestServer::s_Instance.Initialize();
		CHECK_EQ(ERRNO_OK, iErrCode);
		if (ERRNO_OK != iErrCode)
			return false;

		iErrCode = driver.Start(WAIT_TIMEOUT_MS);
		CHECK_EQ(ERRNO_OK, iErrCode);
		return ERRNO_OK == iErrCode;
	}

	std::string MakeLegacyInitialize(int iId)
	{
		return MakeRequest(iId, METHOD_INITIALIZE,
			"{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},\"clientInfo\":{\"name\":\"client\",\"version\":\"1.0.0\"}}");
	}

	void CompleteLegacyHandshake(CDriver& driver, int iId)
	{
		Json::Value jResponse;
		CHECK(ExpectResponse(driver, MakeLegacyInitialize(iId), jResponse));
		CHECK_EQ(ERRNO_OK, driver.Send("{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}"));
	}

	bool SendAndCapture(CDriver& driver, const std::string& strMessage, Json::Value& jMessage)
	{
		size_t nBefore = driver.GetOutputCount();
		if (ERRNO_OK != driver.Send(strMessage))
			return false;

		if (!driver.WaitForOutputCount(nBefore + 1, WAIT_TIMEOUT_MS))
			return false;

		return ParseJson(driver.GetOutput(nBefore), jMessage);
	}
}

TEST_CASE("unit.tasks")
{
	CDriver driver;
	if (!StartServer(driver))
		return;


	// Legacy requests keep their established resource-not-found code.
	CompleteLegacyHandshake(driver, 1);

	Json::Value jResponse;
	CHECK(ExpectError(driver, MakeRequest(2, METHOD_RESOURCES_READ,
		"{\"uri\":\"" + std::string(CTestServer::MISSING_RESOURCE_URI) + "\"}"),
		2, ERRNO_RESOURCE_NOT_FOUND, jResponse));

	// discover publishes the registry versions and the server capabilities.
	CMCPSession::GetInstance().SetServerInstructions("test instructions");
	CHECK(ExpectResponse(driver, MakeRequest(3, METHOD_SERVER_DISCOVER, MakeModernParams()), jResponse));

	const Json::Value& jResult = jResponse[MSG_KEY_RESULT];
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE), jResult[MSG_KEY_RESULT_TYPE].asString());
	CHECK_EQ(0, jResult[MSG_KEY_TTL_MS].asInt());
	CHECK_EQ(std::string(CACHE_SCOPE_PRIVATE), jResult[MSG_KEY_CACHE_SCOPE].asString());
	CHECK_EQ(std::string("test instructions"), jResult[MSG_KEY_INSTRUCTIONS].asString());
	CHECK_EQ(std::string(CTestServer::SERVER_NAME),
		jResult[MSG_KEY_META][META_KEY_SERVER_INFO][MSG_KEY_NAME].asString());

	std::vector<std::string> vecVersions = CMCPSession::GetInstance().GetProtocolRegistry().GetSupportedVersions();
	CHECK_EQ(vecVersions.size(), static_cast<size_t>(jResult[MSG_KEY_SUPPORTED_VERSIONS].size()));
	for (Json::ArrayIndex i = 0; i < jResult[MSG_KEY_SUPPORTED_VERSIONS].size() && i < vecVersions.size(); ++i)
		CHECK_EQ(vecVersions[i], jResult[MSG_KEY_SUPPORTED_VERSIONS][i].asString());

	const Json::Value& jCapabilities = jResult[MSG_KEY_CAPABILITIES];
	CHECK(jCapabilities.isMember(MSG_KEY_TOOLS));
	CHECK(jCapabilities.isMember(MSG_KEY_PROMPTS));
	CHECK(jCapabilities.isMember(MSG_KEY_RESOURCES));
	CHECK(jCapabilities.isMember(MSG_KEY_LOGGING));
	CHECK(!jCapabilities.isMember(MSG_KEY_EXPERIMENTAL));
	CHECK(jCapabilities[MSG_KEY_RESOURCES].isObject());
	CHECK(!jCapabilities[MSG_KEY_RESOURCES].isMember(MSG_KEY_SUBSCRIBE));
	CHECK(!jCapabilities.isMember("completions"));

	CMCPSession::GetInstance().SetServerInstructions(std::string());

	// An invalid cursor must produce a JSON-RPC error that still carries the id.
	CMCPSession::GetInstance().SetServerToolsPagination(true);
	CHECK(ExpectError(driver, MakeRequest(4, METHOD_TOOLS_LIST, MakeModernParams("{\"cursor\":\"9\"}")),
		4, ERRNO_INVALID_PARAMS, jResponse));
	CMCPSession::GetInstance().SetServerToolsPagination(false);

	// An unknown resource takes the stateless resource-not-found code.
	CHECK(ExpectError(driver, MakeRequest(5, METHOD_RESOURCES_READ,
		MakeModernParams("{\"uri\":\"test://echo/unknown\"}")),
		5, ERRNO_INVALID_PARAMS, jResponse));

	driver.Stop();
}

TEST_CASE("unit.caching")
{
	CDriver driver;
	if (!StartServer(driver))
		return;


	Json::Value jResponse;

	// Server defaults are ttlMs = 0 and cacheScope = private.
	CHECK(ExpectResponse(driver, MakeRequest(10, METHOD_RESOURCES_READ,
		MakeModernParams("{\"uri\":\"" + std::string(CTestServer::RESOURCE_URI) + "\"}")), jResponse));
	CHECK_EQ(0, jResponse[MSG_KEY_RESULT][MSG_KEY_TTL_MS].asInt());
	CHECK_EQ(std::string(CACHE_SCOPE_PRIVATE), jResponse[MSG_KEY_RESULT][MSG_KEY_CACHE_SCOPE].asString());

	// Explicit server options flow into cacheable results.
	CMCPSession::GetInstance().SetServerCacheOptions(120, CACHE_SCOPE_PUBLIC);
	CHECK(ExpectResponse(driver, MakeRequest(11, METHOD_TOOLS_LIST, MakeModernParams()), jResponse));
	CHECK_EQ(120, jResponse[MSG_KEY_RESULT][MSG_KEY_TTL_MS].asInt());
	CHECK_EQ(std::string(CACHE_SCOPE_PUBLIC), jResponse[MSG_KEY_RESULT][MSG_KEY_CACHE_SCOPE].asString());

	// An MRTR retry must not be cached.
	CHECK(ExpectResponse(driver, MakeRequest(12, METHOD_RESOURCES_READ,
		MakeModernParams("{\"uri\":\"" + std::string(CTestServer::RESOURCE_URI) +
			"\",\"inputResponses\":{\"confirm\":{\"action\":\"accept\"}},\"requestState\":\"state\"}")), jResponse));
	CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_TTL_MS));
	CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_CACHE_SCOPE));

	// tools/call is never cacheable.
	CHECK(ExpectResponse(driver, MakeRequest(13, METHOD_TOOLS_CALL,
		MakeModernParams("{\"name\":\"echo\",\"arguments\":{\"input\":\"hello\"}}")), jResponse));
	CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_TTL_MS));
	CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_CACHE_SCOPE));

	CMCPSession::GetInstance().SetServerCacheOptions(0, CACHE_SCOPE_PRIVATE);

	driver.Stop();
}

TEST_CASE("unit.mrtr")
{
	CDriver driver;
	if (!StartServer(driver))
		return;


	Json::Value jResponse;

	// Without the elicitation capability the server reports what is missing.
	CHECK(ExpectError(driver, MakeRequest(20, METHOD_TOOLS_CALL,
		MakeModernParams("{\"name\":\"echo\",\"arguments\":{\"action\":\"confirm\"}}")),
		20, ERRNO_MISSING_REQUIRED_CLIENT_CAPABILITY, jResponse));
	CHECK(jResponse[MSG_KEY_ERROR][MSG_KEY_DATA][MSG_KEY_REQUIRED_CAPABILITIES][MSG_KEY_ELICITATION][MSG_KEY_FORM].isObject());

	// With the capability, the first round trip returns input_required.
	const std::string strCaps = "{\"elicitation\":{\"form\":{}}}";
	CHECK(ExpectResponse(driver, MakeRequest(21, METHOD_TOOLS_CALL,
		MakeModernParams("{\"name\":\"echo\",\"arguments\":{\"action\":\"confirm\"}}", strCaps)), jResponse));
	CHECK_EQ(std::string(RESULT_TYPE_INPUT_REQUIRED), jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
	CHECK_EQ(std::string(METHOD_ELICITATION_CREATE),
		jResponse[MSG_KEY_RESULT][MSG_KEY_INPUT_REQUESTS]["confirm"][MSG_KEY_METHOD].asString());
	CHECK_EQ(std::string("echo-confirm-state"), jResponse[MSG_KEY_RESULT][MSG_KEY_REQUEST_STATE].asString());
	CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_TTL_MS));
	CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_CACHE_SCOPE));

	// Retrying with inputResponses completes the call.
	CHECK(ExpectResponse(driver, MakeRequest(22, METHOD_TOOLS_CALL,
		MakeModernParams("{\"name\":\"echo\",\"arguments\":{\"action\":\"confirm\"},"
			"\"inputResponses\":{\"confirm\":{\"action\":\"accept\"}},\"requestState\":\"echo-confirm-state\"}", strCaps)), jResponse));
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE), jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
	CHECK_EQ(std::string("confirmed"), jResponse[MSG_KEY_RESULT][MSG_KEY_CONTENT][0][MSG_KEY_TEXT].asString());

	// An MRTR result with neither part is rejected as invalid params.
	CHECK(ExpectError(driver, MakeRequest(23, METHOD_TOOLS_CALL,
		MakeModernParams("{\"name\":\"echo\",\"arguments\":{\"action\":\"confirm_empty\"}}", strCaps)),
		23, ERRNO_INVALID_PARAMS, jResponse));

	driver.Stop();
}

TEST_CASE("unit.subscriptions")
{
	CDriver driver;
	if (!StartServer(driver))
		return;


	// notifications is required by the listen schema.
	Json::Value jResponse;
	CHECK(ExpectError(driver, MakeRequest(30, METHOD_SUBSCRIPTIONS_LISTEN, MakeModernParams()),
		30, ERRNO_INVALID_PARAMS, jResponse));

	// A numeric listen id produces an acknowledgment before anything else.
	const std::string strListenParams = MakeModernParams(
		"{\"notifications\":{\"toolsListChanged\":true,\"promptsListChanged\":true,"
		"\"resourcesListChanged\":true,\"resourceSubscriptions\":[\"test://echo/resource\"]}}");

	CHECK(SendAndCapture(driver, MakeRequest(31, METHOD_SUBSCRIPTIONS_LISTEN, strListenParams), jResponse));
	CHECK_EQ(std::string(METHOD_NOTIFICATION_SUBSCRIPTIONS_ACKNOWLEDGED), jResponse[MSG_KEY_METHOD].asString());
	CHECK(jResponse[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].isInt());
	CHECK_EQ(31, jResponse[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asInt());
	CHECK(jResponse[MSG_KEY_PARAMS][MSG_KEY_NOTIFICATIONS][MSG_KEY_TOOLS_LIST_CHANGED].asBool());
	CHECK(jResponse[MSG_KEY_PARAMS][MSG_KEY_NOTIFICATIONS][MSG_KEY_PROMPTS_LIST_CHANGED].asBool());
	CHECK(jResponse[MSG_KEY_PARAMS][MSG_KEY_NOTIFICATIONS][MSG_KEY_RESOURCES_LIST_CHANGED].asBool());
	CHECK_EQ(static_cast<Json::ArrayIndex>(1),
		jResponse[MSG_KEY_PARAMS][MSG_KEY_NOTIFICATIONS][MSG_KEY_RESOURCE_SUBSCRIPTIONS].size());

	// A string listen id keeps its JSON type in both ack and notifications.
	const std::string strStringListen = "{\"jsonrpc\":\"2.0\",\"id\":\"listen-str\",\"method\":\"subscriptions/listen\",\"params\":"
		+ MakeModernParams("{\"notifications\":{\"toolsListChanged\":true}}") + "}";
	CHECK(SendAndCapture(driver, strStringListen, jResponse));
	CHECK(jResponse[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].isString());
	CHECK_EQ(std::string("listen-str"), jResponse[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asString());

	// List-changed events reach both active subscriptions.
	size_t nBefore = driver.GetOutputCount();
	CHECK_EQ(ERRNO_OK, CMCPSession::GetInstance().SendToolListChanged());
	CHECK(driver.WaitForOutputCount(nBefore + 2, WAIT_TIMEOUT_MS));

	bool bSawNumeric = false;
	bool bSawString = false;
	for (size_t i = nBefore; i < nBefore + 2; ++i)
	{
		Json::Value jNotification;
		CHECK(ParseJson(driver.GetOutput(i), jNotification));
		CHECK_EQ(std::string(METHOD_NOTIFICATION_TOOLS_LIST_CHANGED), jNotification[MSG_KEY_METHOD].asString());
		const Json::Value& jSubscriptionId = jNotification[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID];
		if (jSubscriptionId.isInt() && 31 == jSubscriptionId.asInt())
			bSawNumeric = true;
		if (jSubscriptionId.isString() && "listen-str" == jSubscriptionId.asString())
			bSawString = true;
	}
	CHECK(bSawNumeric);
	CHECK(bSawString);

	// Only the first subscription granted promptsListChanged, so the event
	// reaches just that subscription.
	nBefore = driver.GetOutputCount();
	CHECK_EQ(ERRNO_OK, CMCPSession::GetInstance().SendPromptListChanged());
	CHECK(driver.WaitForOutputCount(nBefore + 1, WAIT_TIMEOUT_MS));
	CHECK(ParseJson(driver.GetOutput(nBefore), jResponse));
	CHECK_EQ(std::string(METHOD_NOTIFICATION_PROMPTS_LIST_CHANGED),
		jResponse[MSG_KEY_METHOD].asString());
	CHECK_EQ(31, jResponse[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asInt());
	std::this_thread::sleep_for(std::chrono::milliseconds(QUIET_WAIT_MS));
	CHECK_EQ(nBefore + 1, driver.GetOutputCount());

	// Resource updates require an exact URI match.
	const std::string strChildUri = std::string(CTestServer::RESOURCE_URI) + "/child";
	nBefore = driver.GetOutputCount();
	Json::Value jUpdatedParams(Json::objectValue);
	jUpdatedParams[MSG_KEY_URI] = strChildUri;
	CMCPSession::GetInstance().SendSubscriptionNotification(METHOD_NOTIFICATION_RESOURCES_UPDATED, jUpdatedParams, strChildUri);
	CHECK(!driver.WaitForOutputCount(nBefore + 1, QUIET_WAIT_MS));

	jUpdatedParams[MSG_KEY_URI] = CTestServer::RESOURCE_URI;
	CMCPSession::GetInstance().SendSubscriptionNotification(METHOD_NOTIFICATION_RESOURCES_UPDATED, jUpdatedParams, CTestServer::RESOURCE_URI);
	CHECK(driver.WaitForOutputCount(nBefore + 1, WAIT_TIMEOUT_MS));
	CHECK(ParseJson(driver.GetOutput(nBefore), jResponse));
	CHECK_EQ(std::string(METHOD_NOTIFICATION_RESOURCES_UPDATED), jResponse[MSG_KEY_METHOD].asString());
	CHECK_EQ(31, jResponse[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asInt());

	// Cancelling the numeric listen id drops only that subscription.
	const std::string strCancel = "{\"jsonrpc\":\"2.0\",\"method\":\"" + std::string(METHOD_NOTIFICATION_CANCELLED) +
		"\",\"params\":{\"requestId\":31,\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\","
		"\"io.modelcontextprotocol/clientCapabilities\":{}}}}";
	CHECK_EQ(ERRNO_OK, driver.Send(strCancel));

	// A following request confirms the notification has been consumed.
	CHECK(ExpectResponse(driver, MakeRequest(32, METHOD_TOOLS_LIST, MakeModernParams()), jResponse));

	nBefore = driver.GetOutputCount();
	CHECK_EQ(ERRNO_OK, CMCPSession::GetInstance().SendToolListChanged());
	CHECK(driver.WaitForOutputCount(nBefore + 1, WAIT_TIMEOUT_MS));
	CHECK(ParseJson(driver.GetOutput(nBefore), jResponse));
	CHECK(jResponse[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].isString());
	CHECK_EQ(std::string("listen-str"), jResponse[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asString());

	// Terminating completes the surviving listen request.
	nBefore = driver.GetOutputCount();
	driver.Stop();
	CHECK(driver.WaitForOutputCount(nBefore + 1, WAIT_TIMEOUT_MS));
	CHECK(ParseJson(driver.GetOutput(nBefore), jResponse));
	CHECK_EQ(std::string("listen-str"), jResponse[MSG_KEY_ID].asString());
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE), jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
	CHECK(jResponse[MSG_KEY_RESULT][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].isString());
	CHECK_EQ(std::string("listen-str"), jResponse[MSG_KEY_RESULT][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asString());
}

TEST_CASE("unit.capabilities")
{
	// Register the modern capability surface before the server starts.
	CTestServer::s_Instance.RegisterServerCompletionsCapability();
	std::map<std::string, Json::Value> mapExtensions;
	mapExtensions["example/echo"] = Json::Value(Json::objectValue);
	CTestServer::s_Instance.RegisterServerExtensionsCapabilities(mapExtensions);
	CTestServer::s_Instance.RegisterServerAppsCapabilities();
	CTestServer::s_Instance.RegisterServerInstructions("discover instructions");
	CTestServer::s_Instance.SetServerCacheOptions(60, CACHE_SCOPE_PUBLIC);

	CDriver driver;
	if (!StartServer(driver))
		return;

	// Legacy initialize passes the explicit extensions through unchanged.
	Json::Value jResponse;
	CHECK(ExpectResponse(driver, MakeLegacyInitialize(40), jResponse));
	CHECK(jResponse[MSG_KEY_RESULT][MSG_KEY_CAPABILITIES][MSG_KEY_EXTENSIONS]["example/echo"].isObject());
	CHECK(jResponse[MSG_KEY_RESULT][MSG_KEY_CAPABILITIES][MSG_KEY_EXTENSIONS][EXTENSION_MCP_APPS_UI].isObject());
	CHECK(!jResponse[MSG_KEY_RESULT][MSG_KEY_CAPABILITIES].isMember(MSG_KEY_COMPLETIONS));
	CHECK_EQ(ERRNO_OK, driver.Send("{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}"));

	// Modern discover publishes completions, extensions, and caching options.
	CHECK(ExpectResponse(driver, MakeRequest(41, METHOD_SERVER_DISCOVER, MakeModernParams()), jResponse));

	const Json::Value& jCapabilities = jResponse[MSG_KEY_RESULT][MSG_KEY_CAPABILITIES];
	CHECK(jCapabilities[MSG_KEY_COMPLETIONS].isObject());
	CHECK(jCapabilities[MSG_KEY_EXTENSIONS]["example/echo"].isObject());
	CHECK(jCapabilities[MSG_KEY_EXTENSIONS][EXTENSION_MCP_APPS_UI].isObject());
	CHECK(!jCapabilities.isMember(MSG_KEY_EXPERIMENTAL));
	CHECK_EQ(60, jResponse[MSG_KEY_RESULT][MSG_KEY_TTL_MS].asInt());
	CHECK_EQ(std::string(CACHE_SCOPE_PUBLIC), jResponse[MSG_KEY_RESULT][MSG_KEY_CACHE_SCOPE].asString());
	CHECK_EQ(std::string("discover instructions"), jResponse[MSG_KEY_RESULT][MSG_KEY_INSTRUCTIONS].asString());

	CTestServer::s_Instance.SetServerCacheOptions(0, CACHE_SCOPE_PRIVATE);
	CTestServer::s_Instance.RegisterServerInstructions(std::string());

	driver.Stop();
}

TEST_CASE("unit.apps")
{
	const ProtocolProfile* pModernProfile = CMCPSession::GetInstance().GetProtocolRegistry().Find(PROTOCOL_VER_2026_07_28);
	CHECK(pModernProfile != nullptr);
	if (!pModernProfile)
		return;

	ProtocolContext ctxModern;
	ctxModern.strVersion = pModernProfile->strVersion;
	ctxModern.pProfile = pModernProfile;

	Tool tool;
	tool.strName = "echo_app";
	tool.jInputSchema = Json::Value(Json::objectValue);
	tool.SetUiResourceUri("ui://echo/app.html");

	Json::Value jTool;
	CHECK_EQ(ERRNO_OK, tool.DoSerialize(jTool));
	tool.DoSerializeVersionExtras(jTool, ctxModern);
	CHECK_EQ(std::string("ui://echo/app.html"),
		jTool[MSG_KEY_META][MSG_KEY_UI][MSG_KEY_RESOURCE_URI].asString());

	Resource resource;
	resource.strName = "Echo App";
	resource.strUri = "ui://echo/app.html";
	resource.strMimeType = "text/html;profile=mcp-app";
	resource.jMeta[MSG_KEY_UI][MSG_KEY_PERMISSIONS]["clipboardWrite"] = true;

	Json::Value jResource;
	CHECK_EQ(ERRNO_OK, resource.DoSerialize(jResource));
	CHECK_EQ(std::string("ui://echo/app.html"), jResource[MSG_KEY_URI].asString());
	CHECK_EQ(std::string("text/html;profile=mcp-app"), jResource[MSG_KEY_MIMETYPE].asString());
	resource.DoSerializeVersionExtras(jResource, ctxModern);
	CHECK(jResource[MSG_KEY_META][MSG_KEY_UI][MSG_KEY_PERMISSIONS]["clipboardWrite"].asBool());

	// Legacy serialization must not grow the UI metadata.
	ProtocolContext ctxLegacy;
	Json::Value jLegacyResource;
	CHECK_EQ(ERRNO_OK, resource.DoSerialize(jLegacyResource));
	resource.DoSerializeVersionExtras(jLegacyResource, ctxLegacy);
	CHECK(!jLegacyResource.isMember(MSG_KEY_META));
}
