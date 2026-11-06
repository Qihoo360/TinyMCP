#include "../../TestHarness.h"
#include "../Driver.h"
#include "../TestServer.h"

#include <Message/Request.h>
#include <Public/PublicDef.h>
#include <Internal/Protocol.h>
#include <Session/Session.h>

#include <string>
#include <vector>

using namespace MCP;
using namespace MCP::Test;

namespace
{
	const int WAIT_TIMEOUT_MS = 5000;

	std::string MakeRequest(int iId, const std::string& strMethod, const std::string& strParams)
	{
		std::string strRequest = "{\"jsonrpc\":\"2.0\",\"id\":" + std::to_string(iId) + ",\"method\":\"" + strMethod + "\"";
		if (!strParams.empty())
			strRequest += ",\"params\":" + strParams;
		strRequest += "}";

		return strRequest;
	}

	std::string MakeModernMeta()
	{
		return "{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\","
			"\"io.modelcontextprotocol/clientCapabilities\":{}}}";
	}

	std::string MakeModernParams(const std::string& strExtra)
	{
		if (strExtra.empty())
			return MakeModernMeta();

		return std::string("{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\","
			"\"io.modelcontextprotocol/clientCapabilities\":{}},") + strExtra.substr(1);
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
}

TEST_CASE("unit.session")
{
	CTestServer& server = CTestServer::s_Instance;
	int iErrCode = server.Initialize();
	CHECK_EQ(ERRNO_OK, iErrCode);
	if (ERRNO_OK != iErrCode)
		return;

	CDriver driver;
	iErrCode = driver.Start(WAIT_TIMEOUT_MS);
	CHECK_EQ(ERRNO_OK, iErrCode);
	if (ERRNO_OK != iErrCode)
		return;

	Json::Value jResponse;

	// initialize is always negotiated through the handshake profile, even when the
	// client decorates it with stateless _meta.
	std::string strInitialize = MakeRequest(1, METHOD_INITIALIZE,
		"{\"protocolVersion\":\"2026-07-28\",\"capabilities\":{},\"clientInfo\":{\"name\":\"client\",\"version\":\"1.0.0\"},"
		"\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\"}}");
	CHECK(ExpectResponse(driver, strInitialize, jResponse));
	CHECK_EQ(std::string(PROTOCOL_VER_2024_11_05), jResponse[MSG_KEY_RESULT][MSG_KEY_PROTOCOL_VERSION].asString());
	CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_RESULT_TYPE));

	// notifications/initialized completes the handshake.
	CHECK_EQ(ERRNO_OK, driver.Send("{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}"));

	// A handshake client probing server/discover receives method-not-found.
	CHECK(ExpectError(driver, MakeRequest(2, METHOD_SERVER_DISCOVER, ""), 2, ERRNO_METHOD_NOT_FOUND, jResponse));

	// A handshake-only registry rejects stateless requests with the
	// unsupported-version error and reports what the server does serve.
	CProtocolRegistry handshakeOnly(std::vector<std::shared_ptr<const ProtocolProfile>>{
		CProtocolRegistry::GetGlobalRegistry().GetDefaultHandshakeProfile() });
	CMCPSession::GetInstance().SetProtocolRegistry(handshakeOnly);
	CHECK(ExpectError(driver, MakeRequest(3, METHOD_TOOLS_LIST, MakeModernMeta()), 3,
		ERRNO_UNSUPPORTED_PROTOCOL_VERSION, jResponse));
	CHECK_EQ(std::string(PROTOCOL_VER_2026_07_28),
		jResponse[MSG_KEY_ERROR][MSG_KEY_DATA][MSG_KEY_REQUESTED].asString());
	CHECK(jResponse[MSG_KEY_ERROR][MSG_KEY_DATA][MSG_KEY_SUPPORTED].isArray());

	// Restoring the default registry lets the same request through.
	CMCPSession::GetInstance().SetProtocolRegistry(CProtocolRegistry::GetGlobalRegistry());
	CHECK(ExpectResponse(driver, MakeRequest(4, METHOD_TOOLS_LIST, MakeModernMeta()), jResponse));
	CHECK_EQ(4, jResponse[MSG_KEY_ID].asInt());
	CHECK_EQ(std::string(RESULT_TYPE_COMPLETE), jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
	CHECK_EQ(0, jResponse[MSG_KEY_RESULT][MSG_KEY_TTL_MS].asInt());
	CHECK_EQ(std::string(CACHE_SCOPE_PRIVATE), jResponse[MSG_KEY_RESULT][MSG_KEY_CACHE_SCOPE].asString());
	CHECK_EQ(std::string(CTestServer::SERVER_NAME),
		jResponse[MSG_KEY_RESULT][MSG_KEY_META][META_KEY_SERVER_INFO][MSG_KEY_NAME].asString());
	CHECK_EQ(static_cast<Json::ArrayIndex>(1), jResponse[MSG_KEY_RESULT][MSG_KEY_TOOLS].size());

	// params._meta carrying only progressToken keeps the handshake flow.
	CHECK(ExpectResponse(driver, MakeRequest(5, METHOD_TOOLS_LIST, "{\"_meta\":{\"progressToken\":\"progress-1\"}}"), jResponse));
	CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_RESULT_TYPE));
	CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_TTL_MS));

	// Methods outside the stateless profile are not found.
	CHECK(ExpectError(driver, MakeRequest(6, "acme/unknown", MakeModernMeta()), 6, ERRNO_METHOD_NOT_FOUND, jResponse));
	CHECK(ExpectError(driver, MakeRequest(7, METHOD_PING, MakeModernMeta()), 7, ERRNO_METHOD_NOT_FOUND, jResponse));
	CHECK(ExpectError(driver, MakeRequest(8, METHOD_RESOURCES_SUBSCRIBE, MakeModernParams("{\"uri\":\"test://echo/resource\"}")),
		8, ERRNO_METHOD_NOT_FOUND, jResponse));

	// An unknown version reports the versions understood by the registry.
	CHECK(ExpectError(driver, MakeRequest(9, METHOD_TOOLS_LIST,
		"{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2099-01-01\","
		"\"io.modelcontextprotocol/clientCapabilities\":{}}}"), 9, ERRNO_UNSUPPORTED_PROTOCOL_VERSION, jResponse));
	const Json::Value& jSupported = jResponse[MSG_KEY_ERROR][MSG_KEY_DATA][MSG_KEY_SUPPORTED];
	CHECK(jSupported.isArray());
	bool bHasLegacyVersion = false;
	bool bHasModernVersion = false;
	for (Json::ArrayIndex i = 0; i < jSupported.size(); ++i)
	{
		if (PROTOCOL_VER_2024_11_05 == jSupported[i].asString())
			bHasLegacyVersion = true;
		if (PROTOCOL_VER_2026_07_28 == jSupported[i].asString())
			bHasModernVersion = true;
	}
	CHECK(bHasLegacyVersion);
	CHECK(bHasModernVersion);

	// Missing protocolVersion must not fall back to a default.
	CHECK(ExpectError(driver, MakeRequest(10, METHOD_TOOLS_LIST,
		"{\"_meta\":{\"io.modelcontextprotocol/clientCapabilities\":{}}}"), 10, ERRNO_INVALID_PARAMS, jResponse));

	// clientCapabilities is mandatory for stateless requests.
	CHECK(ExpectError(driver, MakeRequest(11, METHOD_TOOLS_LIST,
		"{\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"2026-07-28\"}}"), 11, ERRNO_INVALID_PARAMS, jResponse));

	driver.Stop();
}
