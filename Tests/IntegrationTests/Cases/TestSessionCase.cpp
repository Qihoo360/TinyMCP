#include "../../TestHarness.h"
#include "../Driver.h"
#include "../TestServer.h"

#include <Message/Request.h>
#include <Public/PublicDef.h>
#include <Internal/OriginPolicy.h>
#include <Internal/Protocol.h>
#include <Internal/SessionAccess.h>
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

	// Sends a line that must not be answered, then a ping. The first output
	// after the line must be the ping reply, proving the reader survived and
	// produced nothing for the rejected line.
	bool ExpectSilentThenPing(CDriver& driver, const std::string& strLine, int iPingId)
	{
		size_t nBefore = driver.GetOutputCount();
		if (ERRNO_OK != driver.Send(strLine))
			return false;
		if (ERRNO_OK != driver.Send(MakeRequest(iPingId, METHOD_PING, "")))
			return false;
		if (!driver.WaitForOutputCount(nBefore + 1, WAIT_TIMEOUT_MS))
			return false;

		Json::Value jReply;
		if (!ParseJson(driver.GetOutput(nBefore), jReply))
			return false;

		CHECK_EQ(iPingId, jReply[MSG_KEY_ID].asInt());
		CHECK(jReply.isMember(MSG_KEY_RESULT));

		return iPingId == jReply[MSG_KEY_ID].asInt() && jReply.isMember(MSG_KEY_RESULT);
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
	ProtocolRegistryAccess::Set(CMCPSession::GetInstance(), handshakeOnly);
	CHECK(ExpectError(driver, MakeRequest(3, METHOD_TOOLS_LIST, MakeModernMeta()), 3,
		ERRNO_UNSUPPORTED_PROTOCOL_VERSION, jResponse));
	CHECK_EQ(std::string(PROTOCOL_VER_2026_07_28),
		jResponse[MSG_KEY_ERROR][MSG_KEY_DATA][MSG_KEY_REQUESTED].asString());
	CHECK(jResponse[MSG_KEY_ERROR][MSG_KEY_DATA][MSG_KEY_SUPPORTED].isArray());

	// Restoring the default registry lets the same request through.
	ProtocolRegistryAccess::Set(CMCPSession::GetInstance(), CProtocolRegistry::GetGlobalRegistry());
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

	// Methods outside the stateless profile are not found, including legacy ping.
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

	// A session-based transport refuses stateless requests as invalid requests,
	// independently of which versions the session declares.
	{
		CDriver sessionDriver;
		CMCPSession::GetInstance().ResetForNewClient();
		CHECK_EQ(ERRNO_OK, sessionDriver.Start(WAIT_TIMEOUT_MS, false));
		CHECK(ExpectError(sessionDriver, MakeRequest(12, METHOD_TOOLS_LIST, MakeModernMeta()),
			12, ERRNO_INVALID_REQUEST, jResponse));
		sessionDriver.Stop();
	}
}

// A cancellation only applies to requests that were already queued when it was
// issued, so a client may safely reuse the same jsonrpc id for a fresh request.
TEST_CASE("unit.async_cancel")
{
	// Start from a pristine client state so the scenario is order independent.
	CMCPSession::GetInstance().ResetForNewClient();

	CTestServer& server = CTestServer::s_Instance;
	CHECK_EQ(ERRNO_OK, server.Initialize());

	CDriver driver;
	CHECK_EQ(ERRNO_OK, driver.Start(WAIT_TIMEOUT_MS));

	// Legacy handshake, then occupy the worker with a slow tool call.
	Json::Value jResponse;
	CHECK(ExpectResponse(driver, MakeRequest(1, METHOD_INITIALIZE,
		"{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},\"clientInfo\":{\"name\":\"client\",\"version\":\"1.0.0\"}}"),
		jResponse));
	CHECK_EQ(ERRNO_OK, driver.Send("{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}"));

	CHECK_EQ(ERRNO_OK, driver.Send(MakeRequest(100, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"slow\",\"action\":\"slow\"}}")));
	CHECK_EQ(ERRNO_OK, driver.Send(MakeRequest(101, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"first\"}}")));
	CHECK_EQ(ERRNO_OK, driver.Send(
		"{\"jsonrpc\":\"2.0\",\"method\":\"notifications/cancelled\",\"params\":{\"requestId\":101}}"));
	CHECK_EQ(ERRNO_OK, driver.Send(MakeRequest(101, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"second\"}}")));

	// Exactly one reply for id 101: the cancelled request is dropped while the
	// request queued after the cancellation is served.
	CHECK(driver.WaitForOutputCount(3, WAIT_TIMEOUT_MS));

	int iReplies = 0;
	std::string strText;
	for (size_t i = 1; i < driver.GetOutputCount(); ++i)
	{
		Json::Value jOutput;
		if (!ParseJson(driver.GetOutput(i), jOutput))
			continue;
		if (!jOutput.isMember(MSG_KEY_ID) || 101 != jOutput[MSG_KEY_ID].asInt())
			continue;

		++iReplies;
		if (jOutput[MSG_KEY_RESULT].isMember(MSG_KEY_CONTENT))
			strText = jOutput[MSG_KEY_RESULT][MSG_KEY_CONTENT][0][MSG_KEY_TEXT].asString();
	}

	CHECK_EQ(1, iReplies);
	CHECK_EQ(std::string("second"), strText);

	driver.Stop();
}

// Malformed or hostile input must never take the server down, and every request
// whose id can be recovered must be answered instead of left pending.
TEST_CASE("unit.robustness")
{
	CMCPSession::GetInstance().ResetForNewClient();

	CTestServer& server = CTestServer::s_Instance;
	CHECK_EQ(ERRNO_OK, server.Initialize());

	CDriver driver;
	CHECK_EQ(ERRNO_OK, driver.Start(WAIT_TIMEOUT_MS));

	Json::Value jResponse;
	CHECK(ExpectResponse(driver, MakeRequest(1, METHOD_INITIALIZE,
		"{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},\"clientInfo\":{\"name\":\"client\",\"version\":\"1.0.0\"}}"),
		jResponse));
	CHECK_EQ(ERRNO_OK, driver.Send("{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}"));

	// Integers beyond the int range used to throw out of asInt() and terminate
	// the process. Such an id cannot be echoed, so the request is dropped.
	CHECK(ExpectSilentThenPing(driver, "{\"jsonrpc\":\"2.0\",\"id\":4294967296,\"method\":\"ping\"}", 2));
	CHECK(ExpectSilentThenPing(driver, "{\"jsonrpc\":\"2.0\",\"id\":1e10,\"method\":\"ping\"}", 3));
	CHECK(ExpectSilentThenPing(driver, "{\"jsonrpc\":\"2.0\",\"id\":-9223372036854775808,\"method\":\"ping\"}", 4));

	// A mistyped method has no usable envelope and is dropped without crashing.
	CHECK(ExpectSilentThenPing(driver, "{\"jsonrpc\":\"2.0\",\"id\":5,\"method\":123}", 6));
	CHECK(ExpectSilentThenPing(driver, "{\"jsonrpc\":\"2.0\",\"method\":{\"x\":1}}", 7));

	// Out-of-range progressToken / size and a non-object _meta are tolerated.
	CHECK(ExpectResponse(driver, MakeRequest(8, METHOD_TOOLS_LIST,
		"{\"_meta\":{\"progressToken\":99999999999}}"), jResponse));
	CHECK_EQ(8, jResponse[MSG_KEY_ID].asInt());
	CHECK(jResponse.isMember(MSG_KEY_RESULT));
	CHECK(ExpectResponse(driver, MakeRequest(9, METHOD_TOOLS_LIST, "{\"_meta\":\"not-an-object\"}"), jResponse));
	CHECK_EQ(9, jResponse[MSG_KEY_ID].asInt());
	CHECK(jResponse.isMember(MSG_KEY_RESULT));

	// Known methods with invalid params are answered with -32602 and the id.
	CHECK(ExpectError(driver, MakeRequest(10, METHOD_TOOLS_CALL, "{\"arguments\":{}}"),
		10, ERRNO_INVALID_PARAMS, jResponse));
	CHECK(ExpectError(driver, MakeRequest(11, METHOD_TOOLS_CALL, "{\"name\":42}"),
		11, ERRNO_INVALID_PARAMS, jResponse));
	CHECK(ExpectError(driver, MakeRequest(12, METHOD_RESOURCES_READ, "{\"uri\":{\"nested\":true}}"),
		12, ERRNO_INVALID_PARAMS, jResponse));
	CHECK(ExpectError(driver, MakeRequest(13, METHOD_INITIALIZE, "{\"protocolVersion\":2024}"),
		13, ERRNO_INVALID_PARAMS, jResponse));

	// Methods outside the legacy surface are answered with -32601.
	CHECK(ExpectError(driver, MakeRequest(14, "acme/unknown", ""), 14, ERRNO_METHOD_NOT_FOUND, jResponse));
	CHECK(ExpectError(driver, MakeRequest(15, METHOD_SUBSCRIPTIONS_LISTEN, "{}"),
		15, ERRNO_METHOD_NOT_FOUND, jResponse));

	// A legacy tool that fails without a result, or throws, still gets an
	// answer, and the async worker keeps serving afterwards.
	CHECK(ExpectError(driver, MakeRequest(16, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"fail\"}}"),
		16, ERRNO_INTERNAL_ERROR, jResponse));
	CHECK(ExpectError(driver, MakeRequest(17, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"throw\"}}"),
		17, ERRNO_INTERNAL_ERROR, jResponse));
	CHECK(ExpectResponse(driver, MakeRequest(18, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"alive\"}}"), jResponse));
	CHECK_EQ(18, jResponse[MSG_KEY_ID].asInt());
	CHECK_EQ(std::string("alive"), jResponse[MSG_KEY_RESULT][MSG_KEY_CONTENT][0][MSG_KEY_TEXT].asString());

	// The stateless era answers invalid params and task failures the same way.
	// The default registry is legacy-only, so enable both eras first.
	ProtocolRegistryAccess::Set(CMCPSession::GetInstance(), CProtocolRegistry::GetGlobalRegistry());
	CHECK(ExpectError(driver, MakeRequest(19, METHOD_TOOLS_CALL, MakeModernParams("{\"arguments\":{}}")),
		19, ERRNO_INVALID_PARAMS, jResponse));
	CHECK(ExpectError(driver, MakeRequest(20, METHOD_TOOLS_CALL,
		MakeModernParams("{\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"throw\"}}")),
		20, ERRNO_INTERNAL_ERROR, jResponse));

	driver.Stop();
}

TEST_CASE("unit.origin_policy")
{
	const std::vector<std::string> vecDefaults = { "http://localhost", "http://127.0.0.1" };

	// Empty Origin is same-origin tooling.
	CHECK(IsOriginAllowedBy(vecDefaults, ""));

	// Exact entries and any port on them are allowed.
	CHECK(IsOriginAllowedBy(vecDefaults, "http://localhost"));
	CHECK(IsOriginAllowedBy(vecDefaults, "http://localhost:5173"));
	CHECK(IsOriginAllowedBy(vecDefaults, "http://127.0.0.1:8080"));

	// A prefix match on the host must not pass (DNS-rebinding bypass).
	CHECK(!IsOriginAllowedBy(vecDefaults, "http://localhost.evil.com"));
	CHECK(!IsOriginAllowedBy(vecDefaults, "http://127.0.0.1.attacker.net"));
	CHECK(!IsOriginAllowedBy(vecDefaults, "http://localhost:5173.evil.com"));
	CHECK(!IsOriginAllowedBy(vecDefaults, "http://localhost@evil.com"));
	CHECK(!IsOriginAllowedBy(vecDefaults, "http://localhost:"));
	CHECK(!IsOriginAllowedBy(vecDefaults, "https://localhost"));
	CHECK(!IsOriginAllowedBy(vecDefaults, "http://evil.com"));

	// An entry pinned to a port only admits that port.
	const std::vector<std::string> vecPinned = { "http://localhost:3000" };
	CHECK(IsOriginAllowedBy(vecPinned, "http://localhost:3000"));
	CHECK(!IsOriginAllowedBy(vecPinned, "http://localhost:30001"));
	CHECK(!IsOriginAllowedBy(vecPinned, "http://localhost:3000:1"));

	// An empty whitelist only admits the empty Origin.
	CHECK(!IsOriginAllowedBy(std::vector<std::string>(), "http://localhost"));
}
