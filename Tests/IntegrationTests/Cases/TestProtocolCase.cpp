#include "../../TestHarness.h"
#include "../Driver.h"
#include "../TestServer.h"

#include <Message/Request.h>
#include <Public/PublicDef.h>
#include <Session/Session.h>

#include <algorithm>
#include <chrono>
#include <set>
#include <string>
#include <thread>
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

	std::string MakeModernParams(const std::string& strVersion, const std::string& strCapabilities,
		const std::string& strMetaExtra = std::string(), const std::string& strParamsExtra = std::string())
	{
		std::string strMeta = "{\"io.modelcontextprotocol/protocolVersion\":\"" + strVersion + "\"";
		if (!strCapabilities.empty())
			strMeta += ",\"io.modelcontextprotocol/clientCapabilities\":" + strCapabilities;
		if (!strMetaExtra.empty())
			strMeta += "," + strMetaExtra;
		strMeta += "}";

		std::string strParams = "{\"_meta\":" + strMeta;
		if (!strParamsExtra.empty())
			strParams += "," + strParamsExtra;
		strParams += "}";

		return strParams;
	}

	std::string MakeLegacyInitialize(int iId)
	{
		return MakeRequest(iId, METHOD_INITIALIZE,
			"{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},\"clientInfo\":{\"name\":\"client\",\"version\":\"1.0.0\"}}");
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
		CMCPSession::GetInstance().ResetForNewClient();

		int iErrCode = CTestServer::s_Instance.Initialize();
		CHECK_EQ(ERRNO_OK, iErrCode);
		if (ERRNO_OK != iErrCode)
			return false;

		iErrCode = driver.Start(WAIT_TIMEOUT_MS);
		CHECK_EQ(ERRNO_OK, iErrCode);
		return ERRNO_OK == iErrCode;
	}

	void CompleteLegacyHandshake(CDriver& driver, int iId)
	{
		Json::Value jResponse;
		CHECK(ExpectResponse(driver, MakeLegacyInitialize(iId), jResponse));
		CHECK_EQ(ERRNO_OK, driver.Send("{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}"));
	}

	void WaitQuiet()
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(QUIET_WAIT_MS));
	}

	bool Contains(const std::vector<std::string>& vecValues, const std::string& strValue)
	{
		return std::find(vecValues.begin(), vecValues.end(), strValue) != vecValues.end();
	}

	bool ReadStringArray(const Json::Value& jArray, std::vector<std::string>& vecValues)
	{
		if (!jArray.isArray())
			return false;

		for (Json::ArrayIndex i = 0; i < jArray.size(); ++i)
		{
			if (!jArray[i].isString())
				return false;
			vecValues.push_back(jArray[i].asString());
		}

		return true;
	}

	bool FindTool(const Json::Value& jResult, const std::string& strName, Json::Value& jTool)
	{
		const Json::Value& jTools = jResult[MSG_KEY_TOOLS];
		for (Json::ArrayIndex i = 0; i < jTools.size(); ++i)
		{
			if (jTools[i][MSG_KEY_NAME].asString() == strName)
			{
				jTool = jTools[i];
				return true;
			}
		}

		return false;
	}

	bool FindResource(const Json::Value& jResult, const std::string& strUri, Json::Value& jResource)
	{
		const Json::Value& jResources = jResult[MSG_KEY_RESOURCES];
		for (Json::ArrayIndex i = 0; i < jResources.size(); ++i)
		{
			if (jResources[i][MSG_KEY_URI].asString() == strUri)
			{
				jResource = jResources[i];
				return true;
			}
		}

		return false;
	}
}

TEST_CASE("integration.discover")
{
	CTestServer::s_Instance.RegisterServerCompletionsCapability();
	CTestServer::s_Instance.RegisterServerInstructions("integration instructions");

	CDriver driver;
	if (!StartServer(driver))
		return;

	CMCPSession::GetInstance().SetModernProtocolSupported(true);

	Json::Value jResponse;
	CHECK(ExpectResponse(driver, MakeRequest(1, METHOD_SERVER_DISCOVER,
		MakeModernParams("2026-07-28", "{}")), jResponse));

	const Json::Value& jResult = jResponse[MSG_KEY_RESULT];
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE), jResult[MSG_KEY_RESULT_TYPE].asString());
	CHECK(jResult.isMember(MSG_KEY_TTL_MS));
	CHECK_EQ(std::string(CACHE_SCOPE_PRIVATE), jResult[MSG_KEY_CACHE_SCOPE].asString());
	CHECK_EQ(std::string("integration instructions"), jResult[MSG_KEY_INSTRUCTIONS].asString());
	CHECK_EQ(std::string(CTestServer::SERVER_NAME),
		jResult[MSG_KEY_META][META_KEY_SERVER_INFO][MSG_KEY_NAME].asString());

	std::vector<std::string> vecVersions = CMCPSession::GetInstance().GetProtocolRegistry().GetSupportedVersions();
	CHECK_EQ(vecVersions.size(), static_cast<size_t>(jResult[MSG_KEY_SUPPORTED_VERSIONS].size()));
	for (Json::ArrayIndex i = 0; i < jResult[MSG_KEY_SUPPORTED_VERSIONS].size() && i < vecVersions.size(); ++i)
		CHECK_EQ(vecVersions[i], jResult[MSG_KEY_SUPPORTED_VERSIONS][i].asString());

	// Only declared capabilities are advertised.
	const Json::Value& jCapabilities = jResult[MSG_KEY_CAPABILITIES];
	CHECK(jCapabilities.isMember(MSG_KEY_TOOLS));
	CHECK(jCapabilities.isMember(MSG_KEY_RESOURCES));
	CHECK(jCapabilities.isMember(MSG_KEY_PROMPTS));
	CHECK(jCapabilities.isMember(MSG_KEY_LOGGING));
	CHECK(jCapabilities.isMember(MSG_KEY_COMPLETIONS));
	CHECK(!jCapabilities.isMember(MSG_KEY_EXPERIMENTAL));
	CHECK(!jCapabilities.isMember(MSG_KEY_EXTENSIONS));
	CHECK(!jCapabilities[MSG_KEY_RESOURCES].isMember(MSG_KEY_SUBSCRIBE));

	CTestServer::s_Instance.RegisterServerInstructions(std::string());

	driver.Stop();
}

TEST_CASE("integration.version_fallback")
{
	CDriver driver;
	if (!StartServer(driver))
		return;

	CMCPSession::GetInstance().SetModernProtocolSupported(true);

	Json::Value jResponse;

	// Unknown version reports both the supported list and the requested version.
	CHECK(ExpectError(driver, MakeRequest(1, METHOD_SERVER_DISCOVER,
		MakeModernParams("2025-06-18", "{}")),
		1, ERRNO_UNSUPPORTED_PROTOCOL_VERSION, jResponse));
	const Json::Value& jErrorData = jResponse[MSG_KEY_ERROR][MSG_KEY_DATA];
	CHECK(jErrorData.isObject());
	CHECK_EQ(std::string("2025-06-18"), jErrorData[MSG_KEY_REQUESTED].asString());

	std::vector<std::string> vecReported;
	CHECK(ReadStringArray(jErrorData[MSG_KEY_SUPPORTED], vecReported));
	std::vector<std::string> vecVersions = CMCPSession::GetInstance().GetProtocolRegistry().GetSupportedVersions();
	CHECK_EQ(vecVersions.size(), vecReported.size());
	for (auto& strVersion : vecVersions)
		CHECK(Contains(vecReported, strVersion));

	// Missing metadata is a parameter error, not an unsupported version.
	CHECK(ExpectError(driver, MakeRequest(2, METHOD_TOOLS_LIST,
		"{\"_meta\":{\"io.modelcontextprotocol/clientCapabilities\":{}}}"),
		2, ERRNO_INVALID_PARAMS, jResponse));
	CHECK(ExpectError(driver, MakeRequest(3, METHOD_TOOLS_LIST,
		"{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\"}}"),
		3, ERRNO_INVALID_PARAMS, jResponse));

	// progressToken does not opt into the stateless era.
	CompleteLegacyHandshake(driver, 4);
	CHECK(ExpectResponse(driver, MakeRequest(5, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"x\"},\"_meta\":{\"progressToken\":\"p-1\"}}"),
		jResponse));
	CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_RESULT_TYPE));

	// A legacy-only server rejects stateless requests and keeps the handshake.
	CMCPSession::GetInstance().SetModernProtocolSupported(false);
	CHECK_EQ(ERRNO_OK, CMCPSession::GetInstance().ResetForNewClient());

	CHECK(ExpectError(driver, MakeRequest(6, METHOD_SERVER_DISCOVER,
		MakeModernParams("2026-07-28", "{}")),
		6, ERRNO_INVALID_REQUEST, jResponse));

	CompleteLegacyHandshake(driver, 7);
	CHECK(ExpectResponse(driver, MakeRequest(8, METHOD_TOOLS_LIST, std::string()), jResponse));
	CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_RESULT_TYPE));

	driver.Stop();
}

TEST_CASE("integration.version_extensibility")
{
	CDriver driver;
	if (!StartServer(driver))
		return;

	CMCPSession::GetInstance().SetModernProtocolSupported(true);

	ProtocolProfile legacyProfile = *CProtocolRegistry::GetGlobalRegistry().GetDefaultHandshakeProfile();

	// A synthetic stateless revision is served from profile data only.
	{
		ProtocolProfile fakeProfile;
		fakeProfile.strVersion = "2099-01-01";
		fakeProfile.eEra = Era_Stateless;
		fakeProfile.bitsFeatures.set(Feature_ResultType);
		fakeProfile.bitsFeatures.set(Feature_ResultMeta);
		fakeProfile.bitsFeatures.set(Feature_Discover);
		fakeProfile.bitsFeatures.set(Feature_Caching);
		fakeProfile.setSupportedMethods.insert(METHOD_SERVER_DISCOVER);
		fakeProfile.setSupportedMethods.insert(METHOD_TOOLS_LIST);

		std::vector<ProtocolProfile> vecProfiles;
		vecProfiles.push_back(fakeProfile);
		vecProfiles.push_back(legacyProfile);
		CProtocolRegistry registry(vecProfiles);
		CMCPSession::GetInstance().SetProtocolRegistry(&registry);

		Json::Value jResponse;
		CHECK(ExpectResponse(driver, MakeRequest(1, METHOD_SERVER_DISCOVER,
			MakeModernParams("2099-01-01", "{}")), jResponse));

		std::vector<std::string> vecReported;
		CHECK(ReadStringArray(jResponse[MSG_KEY_RESULT][MSG_KEY_SUPPORTED_VERSIONS], vecReported));
		CHECK(Contains(vecReported, "2099-01-01"));
		CHECK(Contains(vecReported, legacyProfile.strVersion));

		CHECK(ExpectResponse(driver, MakeRequest(2, METHOD_TOOLS_LIST,
			MakeModernParams("2099-01-01", "{}")), jResponse));
		CHECK_EQ(std::string(RESULT_TYPE_COMPLETE),
			jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());

		// Methods not declared by the profile stay unavailable.
		CHECK(ExpectError(driver, MakeRequest(3, METHOD_TOOLS_CALL,
			MakeModernParams("2099-01-01", "{}", std::string(),
				"\"name\":\"echo\",\"arguments\":{\"input\":\"x\"}")),
			3, ERRNO_METHOD_NOT_FOUND, jResponse));

		// The unsupported-version payload is derived from the active registry.
		CHECK(ExpectError(driver, MakeRequest(4, METHOD_SERVER_DISCOVER,
			MakeModernParams("2025-06-18", "{}")),
			4, ERRNO_UNSUPPORTED_PROTOCOL_VERSION, jResponse));
		std::vector<std::string> vecSupported;
		CHECK(ReadStringArray(jResponse[MSG_KEY_ERROR][MSG_KEY_DATA][MSG_KEY_SUPPORTED], vecSupported));
		CHECK(Contains(vecSupported, "2099-01-01"));

		CMCPSession::GetInstance().SetProtocolRegistry(nullptr);
	}

	// A profile without Feature_ResultType omits resultType and cache hints.
	{
		ProtocolProfile plainProfile;
		plainProfile.strVersion = "2097-01-01";
		plainProfile.eEra = Era_Stateless;
		plainProfile.setSupportedMethods.insert(METHOD_TOOLS_LIST);

		std::vector<ProtocolProfile> vecProfiles;
		vecProfiles.push_back(plainProfile);
		vecProfiles.push_back(legacyProfile);
		CProtocolRegistry registry(vecProfiles);
		CMCPSession::GetInstance().SetProtocolRegistry(&registry);

		Json::Value jResponse;
		CHECK(ExpectResponse(driver, MakeRequest(5, METHOD_TOOLS_LIST,
			MakeModernParams("2097-01-01", "{}")), jResponse));

		const Json::Value& jResult = jResponse[MSG_KEY_RESULT];
		CHECK(!jResult.isMember(MSG_KEY_RESULT_TYPE));
		CHECK(!jResult.isMember(MSG_KEY_TTL_MS));
		CHECK(!jResult.isMember(MSG_KEY_CACHE_SCOPE));

		CMCPSession::GetInstance().SetProtocolRegistry(nullptr);
	}

	// Profile order defines the handshake version returned by initialize.
	{
		ProtocolProfile handshakeProfile;
		handshakeProfile.strVersion = "2098-01-01";
		handshakeProfile.eEra = Era_Handshake;
		handshakeProfile.setSupportedMethods.insert(METHOD_INITIALIZE);
		handshakeProfile.setSupportedMethods.insert(METHOD_TOOLS_LIST);

		std::vector<ProtocolProfile> vecProfiles;
		vecProfiles.push_back(handshakeProfile);
		vecProfiles.push_back(legacyProfile);
		CProtocolRegistry registry(vecProfiles);
		CMCPSession::GetInstance().SetProtocolRegistry(&registry);

		Json::Value jResponse;
		CHECK(ExpectResponse(driver, MakeLegacyInitialize(6), jResponse));
		CHECK_EQ(std::string("2098-01-01"),
			jResponse[MSG_KEY_RESULT][MSG_KEY_PROTOCOL_VERSION].asString());

		CHECK_EQ(ERRNO_OK, driver.Send("{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}"));
		CHECK(ExpectResponse(driver, MakeRequest(7, METHOD_TOOLS_LIST, std::string()), jResponse));
		CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_RESULT_TYPE));

		// Methods outside the synthetic revision surface are rejected instead of
		// being silently served by the shared handshake switch.
		CHECK(ExpectError(driver, MakeRequest(8, METHOD_PING, std::string()),
			8, ERRNO_METHOD_NOT_FOUND, jResponse));
		CHECK(ExpectError(driver, MakeRequest(9, METHOD_TOOLS_CALL,
			"{\"name\":\"echo\",\"arguments\":{\"input\":\"x\"}}"),
			9, ERRNO_METHOD_NOT_FOUND, jResponse));

		CMCPSession::GetInstance().SetProtocolRegistry(nullptr);
	}

	driver.Stop();
}

TEST_CASE("integration.modern_core")
{
	CDriver driver;
	if (!StartServer(driver))
		return;

	CMCPSession::GetInstance().SetModernProtocolSupported(true);

	Json::Value jResponse;

	// list results publish resultType first, then the cache hints.
	CHECK(ExpectResponse(driver, MakeRequest(1, METHOD_TOOLS_LIST,
		MakeModernParams("2026-07-28", "{}")), jResponse));
	const Json::Value& jTools = jResponse[MSG_KEY_RESULT];
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE), jTools[MSG_KEY_RESULT_TYPE].asString());
	CHECK(jTools.isMember(MSG_KEY_TTL_MS));
	CHECK_EQ(std::string(CACHE_SCOPE_PRIVATE), jTools[MSG_KEY_CACHE_SCOPE].asString());

	CHECK(ExpectResponse(driver, MakeRequest(2, METHOD_RESOURCES_LIST,
		MakeModernParams("2026-07-28", "{}")), jResponse));
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE),
		jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
	CHECK(jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_TTL_MS));

	CHECK(ExpectResponse(driver, MakeRequest(3, METHOD_RESOURCES_TEMPLATES_LIST,
		MakeModernParams("2026-07-28", "{}")), jResponse));
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE),
		jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
	CHECK(jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_CACHE_SCOPE));

	const Json::Value& jTemplates = jResponse[MSG_KEY_RESULT][MSG_KEY_RESOURCE_TEMPLATES];
	CHECK_EQ(1, static_cast<int>(jTemplates.size()));
	CHECK_EQ(std::string("Echo files"), jTemplates[0][MSG_KEY_NAME].asString());
	CHECK_EQ(std::string("test://echo/{name}"), jTemplates[0][MSG_KEY_URI_TEMPLATE].asString());
	CHECK_EQ(std::string("Legacy test resource template"),
		jTemplates[0][MSG_KEY_DESCRIPTION].asString());
	CHECK_EQ(std::string("text/plain"), jTemplates[0][MSG_KEY_MIMETYPE].asString());

	CHECK(ExpectResponse(driver, MakeRequest(4, METHOD_PROMPTS_LIST,
		MakeModernParams("2026-07-28", "{}")), jResponse));
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE),
		jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
	CHECK(jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_TTL_MS));

	const Json::Value& jPrompts = jResponse[MSG_KEY_RESULT][MSG_KEY_PROMPTS];
	CHECK_EQ(1, static_cast<int>(jPrompts.size()));
	CHECK_EQ(std::string(CTestServer::PROMPT_NAME), jPrompts[0][MSG_KEY_NAME].asString());
	CHECK_EQ(std::string("Echo test prompt"), jPrompts[0][MSG_KEY_DESCRIPTION].asString());
	CHECK_EQ(1, static_cast<int>(jPrompts[0][MSG_KEY_ARGUMENTS].size()));
	CHECK_EQ(std::string("text"), jPrompts[0][MSG_KEY_ARGUMENTS][0][MSG_KEY_NAME].asString());
	CHECK_EQ(std::string("Text to echo"),
		jPrompts[0][MSG_KEY_ARGUMENTS][0][MSG_KEY_DESCRIPTION].asString());
	CHECK(jPrompts[0][MSG_KEY_ARGUMENTS][0][MSG_KEY_REQUIRED].asBool());

	// Every modern content block is serialized behind its feature gate.
	CHECK(ExpectResponse(driver, MakeRequest(5, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"rich\"}")),
		jResponse));

	const Json::Value& jCallResult = jResponse[MSG_KEY_RESULT];
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE), jCallResult[MSG_KEY_RESULT_TYPE].asString());
	CHECK(!jCallResult.isMember(MSG_KEY_TTL_MS));
	CHECK(!jCallResult.isMember(MSG_KEY_CACHE_SCOPE));

	std::set<std::string> setTypes;
	const Json::Value& jContent = jCallResult[MSG_KEY_CONTENT];
	for (Json::ArrayIndex i = 0; i < jContent.size(); ++i)
		setTypes.insert(jContent[i][MSG_KEY_TYPE].asString());
	CHECK_EQ(static_cast<size_t>(6), setTypes.size());
	CHECK(1 == setTypes.count(CONST_TEXT));
	CHECK(1 == setTypes.count(CONST_IMAGE));
	CHECK(1 == setTypes.count(CONST_AUDIO));
	CHECK(1 == setTypes.count(CONST_RESOURCE_LINK));
	CHECK(1 == setTypes.count(CONST_TOOL_USE));
	CHECK(1 == setTypes.count(CONST_TOOL_RESULT));

	// Cacheable reads carry hints, unknown resources keep the parameter error.
	CHECK(ExpectResponse(driver, MakeRequest(6, METHOD_RESOURCES_READ,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"uri\":\"" + std::string(CTestServer::RESOURCE_URI) + "\"")), jResponse));
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE),
		jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
	CHECK(jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_TTL_MS));
	CHECK_EQ(std::string(CACHE_SCOPE_PRIVATE),
		jResponse[MSG_KEY_RESULT][MSG_KEY_CACHE_SCOPE].asString());

	CHECK(ExpectError(driver, MakeRequest(7, METHOD_RESOURCES_READ,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"uri\":\"test://echo/unknown\"")),
		7, ERRNO_INVALID_PARAMS, jResponse));

	// Completions are complete results but never cacheable.
	CHECK(ExpectResponse(driver, MakeRequest(8, METHOD_COMPLETION_COMPLETE,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"ref\":{\"type\":\"ref/prompt\",\"name\":\"echo_prompt\"},"
			"\"argument\":{\"name\":\"text\",\"value\":\"e\"}")),
		jResponse));
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE),
		jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
	CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_TTL_MS));
	CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_CACHE_SCOPE));

	// Legacy-only methods and unknown methods are not part of the profile.
	CHECK(ExpectError(driver, MakeRequest(9, METHOD_LOGGING_SET_LEVEL,
		MakeModernParams("2026-07-28", "{}", std::string(), "\"level\":\"info\"")),
		9, ERRNO_METHOD_NOT_FOUND, jResponse));
	CHECK(ExpectError(driver, MakeRequest(10, METHOD_PING,
		MakeModernParams("2026-07-28", "{}")), 10, ERRNO_METHOD_NOT_FOUND, jResponse));
	CHECK(ExpectError(driver, MakeRequest(11, METHOD_RESOURCES_SUBSCRIBE,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"uri\":\"" + std::string(CTestServer::RESOURCE_URI) + "\"")),
		11, ERRNO_METHOD_NOT_FOUND, jResponse));
	CHECK(ExpectError(driver, MakeRequest(12, "test/unknown",
		MakeModernParams("2026-07-28", "{}")), 12, ERRNO_METHOD_NOT_FOUND, jResponse));

	// A modern initialize still negotiates the handshake revision.
	CHECK(ExpectResponse(driver, MakeRequest(13, METHOD_INITIALIZE,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},"
			"\"clientInfo\":{\"name\":\"client\",\"version\":\"1.0.0\"}")),
		jResponse));
	const ProtocolProfile* pHandshakeProfile = CProtocolRegistry::GetGlobalRegistry().GetDefaultHandshakeProfile();
	CHECK(pHandshakeProfile != nullptr);
	if (pHandshakeProfile)
		CHECK_EQ(pHandshakeProfile->strVersion, jResponse[MSG_KEY_RESULT][MSG_KEY_PROTOCOL_VERSION].asString());
	CHECK(!jResponse[MSG_KEY_RESULT][MSG_KEY_CAPABILITIES].isMember(MSG_KEY_EXTENSIONS));

	// prompts/get resolves the registered prompt and returns its messages.
	CHECK(ExpectResponse(driver, MakeRequest(14, METHOD_PROMPTS_GET,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"name\":\"" + std::string(CTestServer::PROMPT_NAME) + "\","
			"\"arguments\":{\"text\":\"hi\"}")), jResponse));
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE),
		jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
	CHECK_EQ(std::string("Echo test prompt"),
		jResponse[MSG_KEY_RESULT][MSG_KEY_DESCRIPTION].asString());
	CHECK_EQ(1, static_cast<int>(jResponse[MSG_KEY_RESULT][MSG_KEY_MESSAGES].size()));
	CHECK_EQ(std::string(CONST_USER),
		jResponse[MSG_KEY_RESULT][MSG_KEY_MESSAGES][0][MSG_KEY_ROLE].asString());
	CHECK_EQ(std::string(CONST_TEXT),
		jResponse[MSG_KEY_RESULT][MSG_KEY_MESSAGES][0][MSG_KEY_CONTENT][MSG_KEY_TYPE].asString());
	CHECK_EQ(std::string("hello from prompt"),
		jResponse[MSG_KEY_RESULT][MSG_KEY_MESSAGES][0][MSG_KEY_CONTENT][MSG_KEY_TEXT].asString());

	// An unknown prompt is a parameter error, not a method error.
	CHECK(ExpectError(driver, MakeRequest(15, METHOD_PROMPTS_GET,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"name\":\"missing_prompt\",\"arguments\":{}")),
		15, ERRNO_INVALID_PARAMS, jResponse));

	// completion/complete on a resource reference reports a truncated page.
	CHECK(ExpectResponse(driver, MakeRequest(16, METHOD_COMPLETION_COMPLETE,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"ref\":{\"type\":\"ref/resource\",\"uri\":\"" + std::string(CTestServer::RESOURCE_URI) + "\"},"
			"\"argument\":{\"name\":\"text\",\"value\":\"g\"}")), jResponse));
	const Json::Value& jResourceCompletion = jResponse[MSG_KEY_RESULT][MSG_KEY_COMPLETION];
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE),
		jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
	CHECK_EQ(2, static_cast<int>(jResourceCompletion[MSG_KEY_VALUES].size()));
	CHECK_EQ(std::string("gamma"), jResourceCompletion[MSG_KEY_VALUES][0].asString());
	CHECK_EQ(5, jResourceCompletion[MSG_KEY_TOTAL].asInt());
	CHECK(jResourceCompletion[MSG_KEY_HAS_MORE].asBool());

	// Progress: params._meta.progressToken makes the task emit
	// notifications/progress before the tool result.
	size_t nProgressStart = driver.GetOutputCount();
	CHECK_EQ(ERRNO_OK, driver.Send(MakeRequest(17, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", "{}", "\"progressToken\":\"progress-1\"",
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"progress\"}"))));
	CHECK(driver.WaitForOutputCount(nProgressStart + 2, WAIT_TIMEOUT_MS));

	Json::Value jProgress;
	CHECK(ParseJson(driver.GetOutput(nProgressStart), jProgress));
	CHECK_EQ(std::string(METHOD_NOTIFICATION_PROGRESS), jProgress[MSG_KEY_METHOD].asString());
	CHECK(!jProgress.isMember(MSG_KEY_ID));
	CHECK_EQ(std::string("progress-1"),
		jProgress[MSG_KEY_PARAMS][MSG_KEY_PROGRESS_TOKEN].asString());
	CHECK_EQ(5, jProgress[MSG_KEY_PARAMS][MSG_KEY_PROGRESS].asInt());
	CHECK_EQ(10, jProgress[MSG_KEY_PARAMS][MSG_KEY_TOTAL].asInt());

	CHECK(ParseJson(driver.GetOutput(nProgressStart + 1), jResponse));
	CHECK_EQ(17, jResponse[MSG_KEY_ID].asInt());
	CHECK_EQ(std::string("progress-done"),
		jResponse[MSG_KEY_RESULT][MSG_KEY_CONTENT][0][MSG_KEY_TEXT].asString());

	// MRTR can carry a server-to-client sampling request.
	CHECK(ExpectResponse(driver, MakeRequest(18, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", "{\"sampling\":{}}", std::string(),
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"input_sampling\"}")), jResponse));
	CHECK_EQ(std::string(RESULT_TYPE_INPUT_REQUIRED),
		jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
	CHECK_EQ(std::string(METHOD_SAMPLING_CREATE_MESSAGE),
		jResponse[MSG_KEY_RESULT][MSG_KEY_INPUT_REQUESTS]["sample"][MSG_KEY_METHOD].asString());
	CHECK_EQ(std::string("sampling-state"),
		jResponse[MSG_KEY_RESULT][MSG_KEY_REQUEST_STATE].asString());

	CHECK(ExpectResponse(driver, MakeRequest(19, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"input_sampling\"},"
			"\"inputResponses\":{\"sample\":{\"role\":\"assistant\",\"content\":{\"type\":\"text\",\"text\":\"ok\"}}},"
			"\"requestState\":\"sampling-state\"")), jResponse));
	CHECK_EQ(std::string("sampled"),
		jResponse[MSG_KEY_RESULT][MSG_KEY_CONTENT][0][MSG_KEY_TEXT].asString());

	// Without the sampling capability the server reports what is missing.
	CHECK(ExpectError(driver, MakeRequest(20, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"input_sampling\"}")),
		20, ERRNO_MISSING_REQUIRED_CLIENT_CAPABILITY, jResponse));
	CHECK(jResponse[MSG_KEY_ERROR][MSG_KEY_DATA][MSG_KEY_REQUIRED_CAPABILITIES]
		[MSG_KEY_SAMPLING].isObject());

	// MRTR can carry a server-to-client roots/list request.
	CHECK(ExpectResponse(driver, MakeRequest(21, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", "{\"roots\":{}}", std::string(),
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"input_roots\"}")), jResponse));
	CHECK_EQ(std::string(RESULT_TYPE_INPUT_REQUIRED),
		jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
	CHECK_EQ(std::string(METHOD_ROOTS_LIST),
		jResponse[MSG_KEY_RESULT][MSG_KEY_INPUT_REQUESTS]["roots"][MSG_KEY_METHOD].asString());

	CHECK(ExpectResponse(driver, MakeRequest(22, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"input_roots\"},"
			"\"inputResponses\":{\"roots\":{\"roots\":[]}},"
			"\"requestState\":\"roots-state\"")), jResponse));
	CHECK_EQ(std::string("roots-ok"),
		jResponse[MSG_KEY_RESULT][MSG_KEY_CONTENT][0][MSG_KEY_TEXT].asString());

	CHECK(ExpectError(driver, MakeRequest(23, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"input_roots\"}")),
		23, ERRNO_MISSING_REQUIRED_CLIENT_CAPABILITY, jResponse));
	CHECK(jResponse[MSG_KEY_ERROR][MSG_KEY_DATA][MSG_KEY_REQUIRED_CAPABILITIES]
		[MSG_KEY_ROOTS].isObject());

	driver.Stop();
}

TEST_CASE("integration.mrtr")
{
	CTestServer::SetRegisterModernFixtures(true);

	CDriver driver;
	if (!StartServer(driver))
		return;

	CMCPSession::GetInstance().SetModernProtocolSupported(true);

	const std::string strElicitation = "{\"elicitation\":{\"form\":{}}}";
	Json::Value jResponse;

	// Round trip with inputRequests only: the client may retry at will.
	CHECK(ExpectResponse(driver, MakeRequest(1, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", strElicitation, std::string(),
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"confirm_input_only\"}")),
		jResponse));
	const Json::Value& jInputOnly = jResponse[MSG_KEY_RESULT];
	CHECK_EQ(std::string(RESULT_TYPE_INPUT_REQUIRED), jInputOnly[MSG_KEY_RESULT_TYPE].asString());
	CHECK(jInputOnly.isMember(MSG_KEY_INPUT_REQUESTS));
	CHECK(!jInputOnly.isMember(MSG_KEY_REQUEST_STATE));
	CHECK(!jInputOnly.isMember(MSG_KEY_TTL_MS));

	CHECK(ExpectResponse(driver, MakeRequest(2, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", strElicitation, std::string(),
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"confirm_input_only\"},"
			"\"inputResponses\":{\"confirm\":{\"action\":\"accept\",\"content\":{\"confirmed\":true}}}")),
		jResponse));
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE),
		jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
	CHECK_EQ(std::string("input-only-confirmed"),
		jResponse[MSG_KEY_RESULT][MSG_KEY_CONTENT][0][MSG_KEY_TEXT].asString());
	CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_TTL_MS));

	// An MRTR result with neither member is rejected by the SDK.
	CHECK(ExpectError(driver, MakeRequest(3, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", strElicitation, std::string(),
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"confirm_empty\"}")),
		3, ERRNO_INVALID_PARAMS, jResponse));

	// Round trip with requestState only: the client must resend the state.
	CHECK(ExpectResponse(driver, MakeRequest(4, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"confirm_state_only\"}")),
		jResponse));
	const Json::Value& jStateOnly = jResponse[MSG_KEY_RESULT];
	CHECK_EQ(std::string(RESULT_TYPE_INPUT_REQUIRED), jStateOnly[MSG_KEY_RESULT_TYPE].asString());
	CHECK(!jStateOnly.isMember(MSG_KEY_INPUT_REQUESTS));
	CHECK_EQ(std::string("echo-shed-state"), jStateOnly[MSG_KEY_REQUEST_STATE].asString());

	CHECK(ExpectResponse(driver, MakeRequest(5, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"confirm_state_only\"},"
			"\"requestState\":\"echo-shed-state\"")),
		jResponse));
	CHECK_EQ(std::string("state-only-ok"),
		jResponse[MSG_KEY_RESULT][MSG_KEY_CONTENT][0][MSG_KEY_TEXT].asString());

	// The application rejects a tampered state and accepts the original one.
	CHECK(ExpectResponse(driver, MakeRequest(6, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"confirm_tamper\"}")),
		jResponse));
	CHECK_EQ(std::string("echo-tamper-state"),
		jResponse[MSG_KEY_RESULT][MSG_KEY_REQUEST_STATE].asString());

	CHECK(ExpectResponse(driver, MakeRequest(7, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"confirm_tamper\"},"
			"\"requestState\":\"tampered\"")),
		jResponse));
	CHECK(jResponse[MSG_KEY_RESULT][MSG_KEY_IS_ERROR].asBool());
	CHECK_EQ(std::string("tampered state rejected"),
		jResponse[MSG_KEY_RESULT][MSG_KEY_CONTENT][0][MSG_KEY_TEXT].asString());

	CHECK(ExpectResponse(driver, MakeRequest(8, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"confirm_tamper\"},"
			"\"requestState\":\"echo-tamper-state\"")),
		jResponse));
	CHECK_EQ(std::string("state-verified"),
		jResponse[MSG_KEY_RESULT][MSG_KEY_CONTENT][0][MSG_KEY_TEXT].asString());

	// resources/read MRTR retries complete without cache hints.
	CHECK(ExpectResponse(driver, MakeRequest(9, METHOD_RESOURCES_READ,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"uri\":\"" + std::string(CTestServer::MRTR_RESOURCE_URI) + "\"")),
		jResponse));
	CHECK_EQ(std::string(RESULT_TYPE_INPUT_REQUIRED),
		jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
	CHECK_EQ(std::string("echo-read-state"),
		jResponse[MSG_KEY_RESULT][MSG_KEY_REQUEST_STATE].asString());

	CHECK(ExpectResponse(driver, MakeRequest(10, METHOD_RESOURCES_READ,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"uri\":\"" + std::string(CTestServer::MRTR_RESOURCE_URI) + "\","
			"\"requestState\":\"echo-read-state\"")),
		jResponse));
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE),
		jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
	CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_TTL_MS));
	CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_CACHE_SCOPE));

	driver.Stop();
}

TEST_CASE("integration.subscriptions")
{
	CDriver driver;
	CMCPSession::GetInstance().ResetForNewClient();
	CHECK_EQ(ERRNO_OK, CTestServer::s_Instance.Initialize());

	// Withdraw the prompts capability so the listen request must not grant
	// promptsListChanged.
	MCP::Prompts prompts;
	prompts.bExist = false;
	CTestServer::s_Instance.RegisterServerPromptsCapabilities(prompts);

	CHECK_EQ(ERRNO_OK, driver.Start(WAIT_TIMEOUT_MS));
	CMCPSession::GetInstance().SetModernProtocolSupported(true);

	const std::string strListen = MakeRequest(1, METHOD_SUBSCRIPTIONS_LISTEN,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"notifications\":{\"toolsListChanged\":true,\"promptsListChanged\":true,"
			"\"resourcesListChanged\":true,"
			"\"resourceSubscriptions\":[\"" + std::string(CTestServer::RESOURCE_URI) + "\"]}"));

	size_t nBefore = driver.GetOutputCount();
	CHECK_EQ(ERRNO_OK, driver.Send(strListen));
	CHECK(driver.WaitForOutputCount(nBefore + 1, WAIT_TIMEOUT_MS));

	Json::Value jAcknowledged;
	CHECK(ParseJson(driver.GetOutput(nBefore), jAcknowledged));
	CHECK_EQ(std::string(METHOD_NOTIFICATION_SUBSCRIPTIONS_ACKNOWLEDGED),
		jAcknowledged[MSG_KEY_METHOD].asString());

	const Json::Value& jGranted = jAcknowledged[MSG_KEY_PARAMS][MSG_KEY_NOTIFICATIONS];
	CHECK(jGranted[MSG_KEY_TOOLS_LIST_CHANGED].asBool());
	CHECK(!jGranted.isMember(MSG_KEY_PROMPTS_LIST_CHANGED));
	CHECK(jGranted[MSG_KEY_RESOURCES_LIST_CHANGED].asBool());
	CHECK_EQ(1, static_cast<int>(jGranted[MSG_KEY_RESOURCE_SUBSCRIPTIONS].size()));
	CHECK_EQ(std::string(CTestServer::RESOURCE_URI),
		jGranted[MSG_KEY_RESOURCE_SUBSCRIPTIONS][0].asString());

	// A granted event is delivered and carries the subscription id.
	size_t nBeforeEvent = driver.GetOutputCount();
	CHECK_EQ(ERRNO_OK, CMCPSession::GetInstance().SendResourceListChanged());
	CHECK(driver.WaitForOutputCount(nBeforeEvent + 1, WAIT_TIMEOUT_MS));

	Json::Value jEvent;
	CHECK(ParseJson(driver.GetOutput(nBeforeEvent), jEvent));
	CHECK_EQ(std::string(METHOD_NOTIFICATION_RESOURCES_LIST_CHANGED), jEvent[MSG_KEY_METHOD].asString());
	CHECK_EQ(1, jEvent[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asInt());

	// A filter the server cannot publish stays silent.
	size_t nBeforePrompt = driver.GetOutputCount();
	CHECK_EQ(ERRNO_OK, CMCPSession::GetInstance().SendPromptListChanged());
	WaitQuiet();
	CHECK_EQ(nBeforePrompt, driver.GetOutputCount());

	// A resource URI outside the granted set stays silent.
	Json::Value jUpdatedParams(Json::objectValue);
	jUpdatedParams[MSG_KEY_URI] = "test://echo/other";
	size_t nBeforeUri = driver.GetOutputCount();
	CHECK_EQ(ERRNO_OK, CMCPSession::GetInstance().SendSubscriptionNotification(
		METHOD_NOTIFICATION_RESOURCES_UPDATED, jUpdatedParams, "test://echo/other"));
	WaitQuiet();
	CHECK_EQ(nBeforeUri, driver.GetOutputCount());

	// The granted URI is delivered.
	jUpdatedParams[MSG_KEY_URI] = CTestServer::RESOURCE_URI;
	size_t nBeforeGrantedUri = driver.GetOutputCount();
	CHECK_EQ(ERRNO_OK, CMCPSession::GetInstance().SendSubscriptionNotification(
		METHOD_NOTIFICATION_RESOURCES_UPDATED, jUpdatedParams, CTestServer::RESOURCE_URI));
	CHECK(driver.WaitForOutputCount(nBeforeGrantedUri + 1, WAIT_TIMEOUT_MS));

	Json::Value jUpdated;
	CHECK(ParseJson(driver.GetOutput(nBeforeGrantedUri), jUpdated));
	CHECK_EQ(std::string(METHOD_NOTIFICATION_RESOURCES_UPDATED), jUpdated[MSG_KEY_METHOD].asString());

	driver.Stop();
}

TEST_CASE("integration.apps")
{
	CTestServer::SetRegisterModernFixtures(true);

	// Phase 1: apps are opt-in, so neither discover nor initialize advertises them.
	{
		CDriver driver;
		if (!StartServer(driver))
			return;

		CMCPSession::GetInstance().SetModernProtocolSupported(true);

		Json::Value jResponse;
		CHECK(ExpectResponse(driver, MakeRequest(1, METHOD_SERVER_DISCOVER,
			MakeModernParams("2026-07-28", "{}")), jResponse));
		CHECK(!jResponse[MSG_KEY_RESULT][MSG_KEY_CAPABILITIES].isMember(MSG_KEY_EXTENSIONS));

		CHECK(ExpectResponse(driver, MakeLegacyInitialize(2), jResponse));
		CHECK(!jResponse[MSG_KEY_RESULT][MSG_KEY_CAPABILITIES].isMember(MSG_KEY_EXTENSIONS));

		driver.Stop();
	}

	// Phase 2: register the extension and verify the whole surface.
	CDriver driver;
	CMCPSession::GetInstance().ResetForNewClient();
	CHECK_EQ(ERRNO_OK, CTestServer::s_Instance.Initialize());
	CTestServer::s_Instance.RegisterServerAppsCapabilities();
	CHECK_EQ(ERRNO_OK, driver.Start(WAIT_TIMEOUT_MS));

	CMCPSession::GetInstance().SetModernProtocolSupported(true);

	Json::Value jResponse;
	CHECK(ExpectResponse(driver, MakeRequest(10, METHOD_SERVER_DISCOVER,
		MakeModernParams("2026-07-28", "{}")), jResponse));
	const Json::Value& jExtensions = jResponse[MSG_KEY_RESULT][MSG_KEY_CAPABILITIES][MSG_KEY_EXTENSIONS];
	CHECK(jExtensions.isObject());
	CHECK(jExtensions.isMember(EXTENSION_MCP_APPS_UI));

	// tools/list exposes _meta.ui.resourceUri on the app entry point.
	CHECK(ExpectResponse(driver, MakeRequest(11, METHOD_TOOLS_LIST,
		MakeModernParams("2026-07-28", "{}")), jResponse));
	Json::Value jAppTool;
	CHECK(FindTool(jResponse[MSG_KEY_RESULT], CTestServer::APP_TOOL_NAME, jAppTool));
	CHECK_EQ(std::string(CTestServer::UI_RESOURCE_URI),
		jAppTool[MSG_KEY_META][MSG_KEY_UI][MSG_KEY_RESOURCE_URI].asString());

	// resources/list exposes the UI mime type and ui metadata.
	CHECK(ExpectResponse(driver, MakeRequest(12, METHOD_RESOURCES_LIST,
		MakeModernParams("2026-07-28", "{}")), jResponse));
	Json::Value jUiResource;
	CHECK(FindResource(jResponse[MSG_KEY_RESULT], CTestServer::UI_RESOURCE_URI, jUiResource));
	CHECK_EQ(std::string(MIME_TYPE_MCP_APP), jUiResource[MSG_KEY_MIMETYPE].asString());
	CHECK(jUiResource[MSG_KEY_META][MSG_KEY_UI][MSG_KEY_CSP].isObject());
	CHECK(jUiResource[MSG_KEY_META][MSG_KEY_UI][MSG_KEY_PERMISSIONS].isObject());
	CHECK(!jUiResource[MSG_KEY_META][MSG_KEY_UI].isMember("domain"));
	CHECK(!jUiResource[MSG_KEY_META][MSG_KEY_UI].isMember("prefersBorder"));

	// resources/read returns the same metadata with the UI mime type.
	CHECK(ExpectResponse(driver, MakeRequest(13, METHOD_RESOURCES_READ,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"uri\":\"" + std::string(CTestServer::UI_RESOURCE_URI) + "\"")), jResponse));
	const Json::Value& jContents = jResponse[MSG_KEY_RESULT][MSG_KEY_CONTENTS][0];
	CHECK_EQ(std::string(MIME_TYPE_MCP_APP), jContents[MSG_KEY_MIMETYPE].asString());
	CHECK(jContents[MSG_KEY_META][MSG_KEY_UI][MSG_KEY_CSP]["connectDomains"].isArray());
	CHECK(jContents[MSG_KEY_META][MSG_KEY_UI][MSG_KEY_PERMISSIONS]["clipboardWrite"].asBool());
	CHECK(!jContents[MSG_KEY_META][MSG_KEY_UI].isMember("domain"));
	CHECK(!jContents[MSG_KEY_META][MSG_KEY_UI].isMember("prefersBorder"));

	// Client extension capabilities and custom _meta keys are readable.
	CHECK(ExpectResponse(driver, MakeRequest(14, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28",
			"{\"extensions\":{\"" + std::string(EXTENSION_MCP_APPS_UI)
				+ "\":{\"mimeTypes\":[\"" + std::string(MIME_TYPE_MCP_APP) + "\"]}}}",
			"\"example.com/custom\":\"x\"",
			"\"name\":\"" + std::string(CTestServer::APP_TOOL_NAME)
				+ "\",\"arguments\":{\"input\":\"x\",\"action\":\"caps\"}")),
		jResponse));
	CHECK_EQ(std::string("caps:1;" + std::string(MIME_TYPE_MCP_APP) + ";1"),
		jResponse[MSG_KEY_RESULT][MSG_KEY_CONTENT][0][MSG_KEY_TEXT].asString());

	// Custom result _meta keys survive while the SDK keeps its reserved keys.
	CHECK(ExpectResponse(driver, MakeRequest(15, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"name\":\"" + std::string(CTestServer::APP_TOOL_NAME)
				+ "\",\"arguments\":{\"input\":\"x\",\"action\":\"meta\"}")),
		jResponse));
	const Json::Value& jResultMeta = jResponse[MSG_KEY_RESULT][MSG_KEY_META];
	CHECK_EQ(std::string("keep"), jResultMeta["example.com/custom"].asString());
	CHECK_EQ(std::string(CTestServer::SERVER_NAME),
		jResultMeta[META_KEY_SERVER_INFO][MSG_KEY_NAME].asString());
	CHECK(!jResultMeta.isMember(META_KEY_SUBSCRIPTION_ID));

	// Legacy list results never carry the modern _meta field.
	CompleteLegacyHandshake(driver, 16);
	CHECK(ExpectResponse(driver, MakeRequest(17, METHOD_TOOLS_LIST, std::string()), jResponse));
	const Json::Value& jLegacyTools = jResponse[MSG_KEY_RESULT][MSG_KEY_TOOLS];
	for (Json::ArrayIndex i = 0; i < jLegacyTools.size(); ++i)
		CHECK(!jLegacyTools[i].isMember(MSG_KEY_META));

	driver.Stop();
}

TEST_CASE("integration.dual_era")
{
	CDriver driver;
	if (!StartServer(driver))
		return;

	CMCPSession::GetInstance().SetModernProtocolSupported(true);

	CompleteLegacyHandshake(driver, 1);

	size_t nRequests = CMCPSession::GetInstance().GetMessageCount(MessageCategory_Request);
	CHECK(nRequests > 0);

	Json::Value jResponse;
	CHECK(ExpectResponse(driver, MakeRequest(2, METHOD_TOOLS_LIST,
		MakeModernParams("2026-07-28", "{}")), jResponse));
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE),
		jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());

	// Stateless requests never enter the legacy per-connection history.
	CHECK_EQ(nRequests, CMCPSession::GetInstance().GetMessageCount(MessageCategory_Request));

	// A legacy logging threshold does not filter stateless responses. The legacy
	// request itself is recorded, so the baseline moves once here.
	CHECK(ExpectResponse(driver, MakeRequest(3, METHOD_LOGGING_SET_LEVEL,
		"{\"level\":\"error\"}"), jResponse));
	nRequests = CMCPSession::GetInstance().GetMessageCount(MessageCategory_Request);

	// No request level logLevel: the task result is the only output.
	size_t nBeforeNoLog = driver.GetOutputCount();
	CHECK(ExpectResponse(driver, MakeRequest(4, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", "{}", std::string(),
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"log\"}")),
		jResponse));
	WaitQuiet();
	CHECK_EQ(nBeforeNoLog + 1, driver.GetOutputCount());
	CHECK_EQ(std::string("logged"),
		jResponse[MSG_KEY_RESULT][MSG_KEY_CONTENT][0][MSG_KEY_TEXT].asString());

	// With _meta.logLevel the notification is written before the result.
	size_t nBeforeLog = driver.GetOutputCount();
	CHECK_EQ(ERRNO_OK, driver.Send(MakeRequest(5, METHOD_TOOLS_CALL,
		MakeModernParams("2026-07-28", "{}", "\"io.modelcontextprotocol/logLevel\":\"info\"",
			"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"log\"}"))));
	CHECK(driver.WaitForOutputCount(nBeforeLog + 2, WAIT_TIMEOUT_MS));

	Json::Value jNotification;
	CHECK(ParseJson(driver.GetOutput(nBeforeLog), jNotification));
	CHECK_EQ(std::string(METHOD_NOTIFICATION_MESSAGE), jNotification[MSG_KEY_METHOD].asString());
	CHECK_EQ(std::string("info"), jNotification[MSG_KEY_PARAMS][MSG_KEY_LEVEL].asString());

	Json::Value jLogged;
	CHECK(ParseJson(driver.GetOutput(nBeforeLog + 1), jLogged));
	CHECK_EQ(5, jLogged[MSG_KEY_ID].asInt());
	CHECK_EQ(nRequests, CMCPSession::GetInstance().GetMessageCount(MessageCategory_Request));

	// A restarted connection works with stateless requests only.
	CHECK_EQ(ERRNO_OK, CMCPSession::GetInstance().ResetForNewClient());
	CHECK(ExpectResponse(driver, MakeRequest(6, METHOD_TOOLS_LIST,
		MakeModernParams("2026-07-28", "{}")), jResponse));
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE),
		jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());

	driver.Stop();
}
