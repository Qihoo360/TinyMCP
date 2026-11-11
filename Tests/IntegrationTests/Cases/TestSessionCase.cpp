#include "../../TestHarness.h"
#include "../Driver.h"
#include "../TestServer.h"

#include <Message/Request.h>
#include <Public/PublicDef.h>
#include <Internal/OriginPolicy.h>
#include <Internal/Protocol.h>
#include <Internal/SessionAccess.h>
#include <Session/Session.h>

#include <chrono>
#include <string>
#include <thread>
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

// The worker takes every queued task as one batch. A cancellation that arrives
// while an earlier task of that batch is still executing must stop the later
// task before it starts.
TEST_CASE("unit.async_cancel_in_batch")
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

	// 200 occupies the worker, so 201 and 202 are queued and taken together.
	CHECK_EQ(ERRNO_OK, driver.Send(MakeRequest(200, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"a\",\"action\":\"slow\"}}")));
	CHECK_EQ(ERRNO_OK, driver.Send(MakeRequest(201, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"b\",\"action\":\"slow\"}}")));
	CHECK_EQ(ERRNO_OK, driver.Send(MakeRequest(202, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"c\"}}")));

	// Once 200 answered, the worker is executing 201 out of the [201, 202] batch.
	CHECK(driver.WaitForOutputCount(2, WAIT_TIMEOUT_MS));
	std::this_thread::sleep_for(std::chrono::milliseconds(300));
	CHECK_EQ(ERRNO_OK, driver.Send(
		"{\"jsonrpc\":\"2.0\",\"method\":\"notifications/cancelled\",\"params\":{\"requestId\":202}}"));

	// 201 still answers; a later request proves the worker went past 202.
	CHECK(driver.WaitForOutputCount(3, WAIT_TIMEOUT_MS));
	CHECK(ExpectResponse(driver, MakeRequest(203, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"d\"}}"), jResponse));
	CHECK_EQ(203, jResponse[MSG_KEY_ID].asInt());

	bool bAnswered201 = false;
	bool bAnswered202 = false;
	for (size_t i = 1; i < driver.GetOutputCount(); ++i)
	{
		Json::Value jOutput;
		if (!ParseJson(driver.GetOutput(i), jOutput) || !jOutput.isMember(MSG_KEY_ID))
			continue;
		if (201 == jOutput[MSG_KEY_ID].asInt())
			bAnswered201 = true;
		if (202 == jOutput[MSG_KEY_ID].asInt())
			bAnswered202 = true;
	}

	CHECK(bAnswered201);
	CHECK(!bAnswered202);

	driver.Stop();
}

// A task whose Cancel() calls back into CancelTask must not deadlock the
// worker: the session cancels tasks only after releasing its task lock.
TEST_CASE("unit.async_cancel_reentrant")
{
	CMCPSession::GetInstance().ResetForNewClient();
	KeepAliveProbe::s_bHangStarted.store(false);
	KeepAliveProbe::s_bCancelRequested.store(false);

	CTestServer& server = CTestServer::s_Instance;
	CHECK_EQ(ERRNO_OK, server.Initialize());

	CDriver driver;
	CHECK_EQ(ERRNO_OK, driver.Start(WAIT_TIMEOUT_MS));

	Json::Value jResponse;
	CHECK(ExpectResponse(driver, MakeRequest(1, METHOD_INITIALIZE,
		"{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},\"clientInfo\":{\"name\":\"client\",\"version\":\"1.0.0\"}}"),
		jResponse));
	CHECK_EQ(ERRNO_OK, driver.Send("{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}"));

	CHECK_EQ(ERRNO_OK, driver.Send(MakeRequest(2, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"reenter\"}}")));
	auto tpDeadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(WAIT_TIMEOUT_MS);
	while (!KeepAliveProbe::s_bHangStarted.load() && std::chrono::steady_clock::now() < tpDeadline)
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	CHECK(KeepAliveProbe::s_bHangStarted.load());

	CHECK_EQ(ERRNO_OK, driver.Send(
		"{\"jsonrpc\":\"2.0\",\"method\":\"notifications/cancelled\",\"params\":{\"requestId\":2}}"));
	tpDeadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(WAIT_TIMEOUT_MS);
	while (!KeepAliveProbe::s_bCancelRequested.load() && std::chrono::steady_clock::now() < tpDeadline)
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	CHECK(KeepAliveProbe::s_bCancelRequested.load());

	// The worker is still alive and serves the next call.
	CHECK(ExpectResponse(driver, MakeRequest(3, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"alive\"}}"), jResponse));
	CHECK_EQ(3, jResponse[MSG_KEY_ID].asInt());

	driver.Stop();
}

// A request is answered at most once: a duplicate result is dropped, and so is
// a result the task still reports after the client cancelled the request.
TEST_CASE("unit.reply_once")
{
	CMCPSession::GetInstance().ResetForNewClient();
	KeepAliveProbe::s_bHangStarted.store(false);
	KeepAliveProbe::s_bCancelRequested.store(false);

	CTestServer& server = CTestServer::s_Instance;
	CHECK_EQ(ERRNO_OK, server.Initialize());

	CDriver driver;
	CHECK_EQ(ERRNO_OK, driver.Start(WAIT_TIMEOUT_MS));

	Json::Value jResponse;
	CHECK(ExpectResponse(driver, MakeRequest(1, METHOD_INITIALIZE,
		"{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},\"clientInfo\":{\"name\":\"client\",\"version\":\"1.0.0\"}}"),
		jResponse));
	CHECK_EQ(ERRNO_OK, driver.Send("{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}"));

	CHECK(ExpectResponse(driver, MakeRequest(2, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"double\"}}"), jResponse));
	CHECK_EQ(2, jResponse[MSG_KEY_ID].asInt());
	CHECK(ExpectSilentThenPing(driver, "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/none\"}", 3));

	CHECK_EQ(ERRNO_OK, driver.Send(MakeRequest(4, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"late\"}}")));
	auto tpDeadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(WAIT_TIMEOUT_MS);
	while (!KeepAliveProbe::s_bHangStarted.load() && std::chrono::steady_clock::now() < tpDeadline)
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	CHECK(KeepAliveProbe::s_bHangStarted.load());

	CHECK_EQ(ERRNO_OK, driver.Send(
		"{\"jsonrpc\":\"2.0\",\"method\":\"notifications/cancelled\",\"params\":{\"requestId\":4}}"));
	tpDeadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(WAIT_TIMEOUT_MS);
	while (!KeepAliveProbe::s_bCancelRequested.load() && std::chrono::steady_clock::now() < tpDeadline)
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	CHECK(KeepAliveProbe::s_bCancelRequested.load());

	// Give the background thread time to report its late result.
	std::this_thread::sleep_for(std::chrono::milliseconds(300));
	CHECK(ExpectSilentThenPing(driver, "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/none\"}", 5));

	int iReplies2 = 0;
	int iReplies4 = 0;
	for (size_t i = 1; i < driver.GetOutputCount(); ++i)
	{
		Json::Value jOutput;
		if (!ParseJson(driver.GetOutput(i), jOutput) || !jOutput.isMember(MSG_KEY_ID))
			continue;
		if (2 == jOutput[MSG_KEY_ID].asInt())
			++iReplies2;
		if (4 == jOutput[MSG_KEY_ID].asInt())
			++iReplies4;
	}

	CHECK_EQ(1, iReplies2);
	CHECK_EQ(0, iReplies4);

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

	// Numeric ids within the double safe range are echoed exactly, including
	// values beyond the int range. Integers that cannot be represented exactly
	// are dropped because the reply would no longer carry the original id.
	CHECK(ExpectResponse(driver, "{\"jsonrpc\":\"2.0\",\"id\":4294967296,\"method\":\"ping\"}", jResponse));
	CHECK_EQ(4294967296.0, jResponse[MSG_KEY_ID].asDouble());
	CHECK(jResponse.isMember(MSG_KEY_RESULT));
	CHECK(ExpectResponse(driver, "{\"jsonrpc\":\"2.0\",\"id\":1e10,\"method\":\"ping\"}", jResponse));
	CHECK_EQ(10000000000.0, jResponse[MSG_KEY_ID].asDouble());
	CHECK(jResponse.isMember(MSG_KEY_RESULT));
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

// A task replying from its own thread must stay alive while that thread still
// uses it, even after the worker reaped the finished task from its cache.
TEST_CASE("unit.keepalive")
{
	CMCPSession::GetInstance().ResetForNewClient();
	KeepAliveProbe::s_nDestroyed.store(0);
	KeepAliveProbe::s_bRelease.store(false);
	KeepAliveProbe::s_bTouchedAfterResult.store(false);

	CTestServer& server = CTestServer::s_Instance;
	CHECK_EQ(ERRNO_OK, server.Initialize());

	CDriver driver;
	CHECK_EQ(ERRNO_OK, driver.Start(WAIT_TIMEOUT_MS));

	Json::Value jResponse;
	CHECK(ExpectResponse(driver, MakeRequest(1, METHOD_INITIALIZE,
		"{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},\"clientInfo\":{\"name\":\"client\",\"version\":\"1.0.0\"}}"),
		jResponse));
	CHECK_EQ(ERRNO_OK, driver.Send("{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}"));

	// The dispatched task hands out a reference; an undispatched clone does not.
	CHECK(ExpectResponse(driver, MakeRequest(2, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"keepalive\"}}"), jResponse));
	CHECK_EQ(2, jResponse[MSG_KEY_ID].asInt());
	CHECK_EQ(std::string("keepalive:unbound"), jResponse[MSG_KEY_RESULT][MSG_KEY_CONTENT][0][MSG_KEY_TEXT].asString());

	// Wake the worker so it reaps the finished task. Only the background
	// thread's reference keeps it alive now.
	CHECK(ExpectResponse(driver, MakeRequest(3, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"reap\"}}"), jResponse));
	CHECK_EQ(3, jResponse[MSG_KEY_ID].asInt());
	CHECK_EQ(0, KeepAliveProbe::s_nDestroyed.load());

	// Releasing the thread drops the last reference and destroys the task.
	KeepAliveProbe::s_bRelease.store(true);
	auto tpDeadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(WAIT_TIMEOUT_MS);
	while (0 == KeepAliveProbe::s_nDestroyed.load() && std::chrono::steady_clock::now() < tpDeadline)
		std::this_thread::sleep_for(std::chrono::milliseconds(5));

	CHECK_EQ(1, KeepAliveProbe::s_nDestroyed.load());
	CHECK(KeepAliveProbe::s_bTouchedAfterResult.load());

	driver.Stop();
}

// Stop() must destroy finished tasks itself. Left in the cache they would only
// die with the session singleton during static teardown, after business
// singletons their destructors still use (the NewSearch exit crash).
TEST_CASE("unit.stop_releases_tasks")
{
	CMCPSession::GetInstance().ResetForNewClient();
	KeepAliveProbe::s_nDestroyed.store(0);

	CTestServer& server = CTestServer::s_Instance;
	CHECK_EQ(ERRNO_OK, server.Initialize());

	CDriver driver;
	CHECK_EQ(ERRNO_OK, driver.Start(WAIT_TIMEOUT_MS));

	Json::Value jResponse;
	CHECK(ExpectResponse(driver, MakeRequest(1, METHOD_INITIALIZE,
		"{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},\"clientInfo\":{\"name\":\"client\",\"version\":\"1.0.0\"}}"),
		jResponse));
	CHECK_EQ(ERRNO_OK, driver.Send("{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}"));

	// The last task of a session is never reaped by the worker: nothing wakes it
	// again, so the finished task is still cached when the server stops.
	CHECK(ExpectResponse(driver, MakeRequest(2, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"last\",\"action\":\"track\"}}"), jResponse));
	CHECK_EQ(2, jResponse[MSG_KEY_ID].asInt());
	CHECK_EQ(0, KeepAliveProbe::s_nDestroyed.load());

	driver.Stop();
	CHECK_EQ(1, KeepAliveProbe::s_nDestroyed.load());
}

// A task still running in background work at Stop() is cancelled, and Stop()
// waits for that work to let go so the task is destroyed before it returns.
TEST_CASE("unit.stop_drains_running_tasks")
{
	CMCPSession::GetInstance().ResetForNewClient();
	KeepAliveProbe::s_nDestroyed.store(0);
	KeepAliveProbe::s_bHangStarted.store(false);
	KeepAliveProbe::s_bCancelRequested.store(false);

	CTestServer& server = CTestServer::s_Instance;
	CHECK_EQ(ERRNO_OK, server.Initialize());

	CDriver driver;
	CHECK_EQ(ERRNO_OK, driver.Start(WAIT_TIMEOUT_MS));

	Json::Value jResponse;
	CHECK(ExpectResponse(driver, MakeRequest(1, METHOD_INITIALIZE,
		"{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},\"clientInfo\":{\"name\":\"client\",\"version\":\"1.0.0\"}}"),
		jResponse));
	CHECK_EQ(ERRNO_OK, driver.Send("{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}"));

	CHECK_EQ(ERRNO_OK, driver.Send(MakeRequest(2, METHOD_TOOLS_CALL,
		"{\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"hang\"}}")));
	auto tpDeadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(WAIT_TIMEOUT_MS);
	while (!KeepAliveProbe::s_bHangStarted.load() && std::chrono::steady_clock::now() < tpDeadline)
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	CHECK(KeepAliveProbe::s_bHangStarted.load());

	driver.Stop();
	CHECK(KeepAliveProbe::s_bCancelRequested.load());
	CHECK_EQ(1, KeepAliveProbe::s_nDestroyed.load());
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
