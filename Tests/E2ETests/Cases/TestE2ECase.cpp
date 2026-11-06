#include "../../TestHarness.h"
#include "../StdioClient.h"
#include "../../IntegrationTests/TestServer.h"

#include <Public/PublicDef.h>

#include <json/json.h>

#include <chrono>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

namespace MCP
{
	namespace Test
	{
		namespace
		{
			const int E2E_WAIT_TIMEOUT_MS = 10000;
			const int E2E_QUIET_WAIT_MS = 300;
			const size_t LEGACY_RESPONSE_COUNT = 13;

			std::string MakeRequestWithId(const std::string& strId, const std::string& strMethod,
				const std::string& strParams)
			{
				std::string strRequest = "{\"jsonrpc\":\"2.0\",\"id\":" + strId + ",\"method\":\"" + strMethod + "\"";
				if (!strParams.empty())
					strRequest += ",\"params\":" + strParams;
				strRequest += "}";

				return strRequest;
			}

			std::string MakeRequest(int iId, const std::string& strMethod, const std::string& strParams = std::string())
			{
				return MakeRequestWithId(std::to_string(iId), strMethod, strParams);
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
					"{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},\"clientInfo\":{\"name\":\"TinyMCPE2E\",\"version\":\"1.0.0\"}}");
			}

			bool ParseJson(const std::string& strText, Json::Value& jValue)
			{
				Json::Reader reader;
				return reader.parse(strText, jValue) && jValue.isObject();
			}

			bool SendRequest(CStdioClient& client, int iId, const std::string& strMethod, const std::string& strParams)
			{
				return ERRNO_OK == client.Send(MakeRequest(iId, strMethod, strParams));
			}

			bool ExpectResponse(CStdioClient& client, int iId, const std::string& strMethod,
				const std::string& strParams, Json::Value& jResponse)
			{
				size_t nStart = client.GetOutputCount();
				if (!SendRequest(client, iId, strMethod, strParams))
					return false;

				size_t nIndex = 0;
				if (!client.WaitForId(iId, nStart, nIndex, E2E_WAIT_TIMEOUT_MS))
					return false;

				return ParseJson(client.GetOutput(nIndex), jResponse);
			}

			bool ExpectError(CStdioClient& client, int iId, const std::string& strMethod,
				const std::string& strParams, int iErrorCode, Json::Value& jResponse)
			{
				if (!ExpectResponse(client, iId, strMethod, strParams, jResponse))
					return false;

				CHECK(jResponse.isMember(MSG_KEY_ERROR));
				if (!jResponse.isMember(MSG_KEY_ERROR))
					return false;

				CHECK_EQ(iErrorCode, jResponse[MSG_KEY_ERROR][MSG_KEY_CODE].asInt());
				return iErrorCode == jResponse[MSG_KEY_ERROR][MSG_KEY_CODE].asInt();
			}

			bool ReadGoldenLines(std::vector<std::string>& vecLines)
			{
				std::ifstream ifs(std::string(TINYMCP_GOLDEN_DIR) + "/legacy_responses.txt", std::ios::binary);
				if (!ifs)
					return false;

				std::string strLine;
				while (std::getline(ifs, strLine))
				{
					if (!strLine.empty() && '\r' == strLine.back())
						strLine.pop_back();
					vecLines.push_back(strLine);
				}

				return true;
			}

			bool ContainsValue(const Json::Value& jArray, const std::string& strValue)
			{
				for (Json::ArrayIndex i = 0; i < jArray.size(); ++i)
				{
					if (jArray[i].isString() && jArray[i].asString() == strValue)
						return true;
				}

				return false;
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

			bool HasStringIdResult(const CStdioClient& client, const std::string& strId)
			{
				for (size_t i = 0; i < client.GetOutputCount(); ++i)
				{
					Json::Value jMessage;
					if (!ParseJson(client.GetOutput(i), jMessage))
						continue;

					if (jMessage.isMember(MSG_KEY_ID) && jMessage[MSG_KEY_ID].isString()
						&& jMessage[MSG_KEY_ID].asString() == strId && jMessage.isMember(MSG_KEY_RESULT))
					{
						return true;
					}
				}

				return false;
			}
		}

		// Legacy era over a real stdio process: replay the same scene as
		// integration.legacy and require byte-for-byte golden parity.
		TEST_CASE("e2e.legacy_stdio")
		{
			CStdioClient client;
			int iErrCode = client.Start(true);
			CHECK_EQ(ERRNO_OK, iErrCode);
			if (ERRNO_OK != iErrCode)
				return;

			std::vector<std::string> vecActual;
			auto Step = [&client, &vecActual](int iId, const std::string& strMethod,
				const std::string& strParams) -> bool
			{
				size_t nStart = client.GetOutputCount();
				if (ERRNO_OK != client.Send(MakeRequest(iId, strMethod, strParams)))
					return false;

				size_t nIndex = 0;
				if (!client.WaitForId(iId, nStart, nIndex, E2E_WAIT_TIMEOUT_MS))
					return false;

				Json::Value jResponse;
				if (!ParseJson(client.GetOutput(nIndex), jResponse))
					return false;

				CHECK_EQ(iId, jResponse[MSG_KEY_ID].asInt());
				CHECK(jResponse.isMember(MSG_KEY_RESULT));
				if (!jResponse.isMember(MSG_KEY_RESULT))
					return false;

				vecActual.push_back(client.GetOutput(nIndex));
				return true;
			};

			CHECK(Step(1, METHOD_INITIALIZE,
				"{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},\"clientInfo\":{\"name\":\"TinyMCPTests\",\"version\":\"1.0.0\"}}"));

			CHECK_EQ(ERRNO_OK, client.Send("{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}"));

			CHECK(Step(2, METHOD_TOOLS_LIST, std::string()));
			CHECK(Step(3, METHOD_TOOLS_CALL, "{\"name\":\"echo\",\"arguments\":{\"input\":\"hello legacy\"}}"));
			CHECK(Step(4, METHOD_RESOURCES_LIST, std::string()));
			CHECK(Step(5, METHOD_RESOURCES_READ, "{\"uri\":\"test://echo/resource\"}"));
			CHECK(Step(6, METHOD_RESOURCES_TEMPLATES_LIST, std::string()));
			CHECK(Step(7, METHOD_RESOURCES_SUBSCRIBE, "{\"uri\":\"test://echo/resource\"}"));
			CHECK(Step(8, METHOD_RESOURCES_UNSUBSCRIBE, "{\"uri\":\"test://echo/resource\"}"));
			CHECK(Step(9, METHOD_PROMPTS_LIST, std::string()));
			CHECK(Step(10, METHOD_PROMPTS_GET, "{\"name\":\"echo_prompt\",\"arguments\":{\"text\":\"hi\"}}"));
			CHECK(Step(11, METHOD_COMPLETION_COMPLETE,
				"{\"ref\":{\"type\":\"ref/prompt\",\"name\":\"echo_prompt\"},\"argument\":{\"name\":\"text\",\"value\":\"a\"}}"));
			CHECK(Step(12, METHOD_LOGGING_SET_LEVEL, "{\"level\":\"info\"}"));
			CHECK(Step(13, METHOD_PING, std::string()));

			CHECK_EQ(vecActual.size(), LEGACY_RESPONSE_COUNT);

			Json::Value jInitialize;
			CHECK(ParseJson(vecActual[0], jInitialize));
			CHECK_EQ(std::string(PROTOCOL_VER_2024_11_05),
				jInitialize[MSG_KEY_RESULT][MSG_KEY_PROTOCOL_VERSION].asString());

			Json::Value jToolsList;
			CHECK(ParseJson(vecActual[1], jToolsList));
			CHECK(!jToolsList[MSG_KEY_RESULT].isMember(MSG_KEY_RESULT_TYPE));
			CHECK(!jToolsList[MSG_KEY_RESULT].isMember(MSG_KEY_TTL_MS));

			std::vector<std::string> vecGolden;
			bool bRead = ReadGoldenLines(vecGolden);
			CHECK(bRead);
			if (bRead)
			{
				CHECK_EQ(vecActual.size(), vecGolden.size());
				size_t nCount = vecActual.size() < vecGolden.size() ? vecActual.size() : vecGolden.size();
				for (size_t i = 0; i < nCount; ++i)
				{
					if (vecActual[i] != vecGolden[i])
					{
						CTestContext::GetInstance().FailEq(__FILE__, __LINE__,
							"e2e.actual[" + std::to_string(i) + "]", "golden[" + std::to_string(i) + "]",
							vecActual[i], vecGolden[i]);
					}
				}
			}

			// A legacy-only server rejects stateless requests and keeps serving.
			Json::Value jResponse;
			CHECK(ExpectError(client, 20, METHOD_SERVER_DISCOVER,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}"), ERRNO_INVALID_REQUEST, jResponse));
			CHECK(ExpectError(client, 21, METHOD_RESOURCES_READ,
				"{\"uri\":\"test://echo/missing\"}", ERRNO_RESOURCE_NOT_FOUND, jResponse));
			CHECK(ExpectResponse(client, 22, METHOD_PING, std::string(), jResponse));

			CHECK_EQ(ERRNO_OK, client.Stop(0));
		}

		// Modern era over a real stdio process: no handshake, stateless metadata,
		// MRTR, Apps, request-scoped logging and listen subscriptions.
		TEST_CASE("e2e.modern_stdio")
		{
			CStdioClient client;
			int iErrCode = client.Start(false);
			CHECK_EQ(ERRNO_OK, iErrCode);
			if (ERRNO_OK != iErrCode)
				return;

			Json::Value jResponse;

			// server/discover reports versions, capabilities, cache hints and serverInfo.
			CHECK(ExpectResponse(client, 1, METHOD_SERVER_DISCOVER,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}"), jResponse));
			const Json::Value& jDiscover = jResponse[MSG_KEY_RESULT];
			CHECK_EQ(std::string(RESULT_TYPE_COMPLETE), jDiscover[MSG_KEY_RESULT_TYPE].asString());
			CHECK(jDiscover.isMember(MSG_KEY_TTL_MS));
			CHECK_EQ(std::string(CACHE_SCOPE_PRIVATE), jDiscover[MSG_KEY_CACHE_SCOPE].asString());
			CHECK_EQ(std::string("TinyMCP E2E server"), jDiscover[MSG_KEY_INSTRUCTIONS].asString());
			CHECK_EQ(std::string(CTestServer::SERVER_NAME),
				jDiscover[MSG_KEY_META][META_KEY_SERVER_INFO][MSG_KEY_NAME].asString());
			CHECK(ContainsValue(jDiscover[MSG_KEY_SUPPORTED_VERSIONS], PROTOCOL_VER_2024_11_05));
			CHECK(ContainsValue(jDiscover[MSG_KEY_SUPPORTED_VERSIONS], PROTOCOL_VER_2026_07_28));

			const Json::Value& jCapabilities = jDiscover[MSG_KEY_CAPABILITIES];
			CHECK(jCapabilities.isMember(MSG_KEY_TOOLS));
			// The stateless era replaces resources/subscribe with listen subscriptions.
			CHECK(!jCapabilities[MSG_KEY_RESOURCES].isMember(MSG_KEY_SUBSCRIBE));
			CHECK(jCapabilities[MSG_KEY_RESOURCES][MSG_KEY_LISTCHANGED].asBool());
			CHECK(jCapabilities.isMember(MSG_KEY_PROMPTS));
			CHECK(jCapabilities.isMember(MSG_KEY_LOGGING));
			CHECK(jCapabilities.isMember(MSG_KEY_COMPLETIONS));
			CHECK(jCapabilities[MSG_KEY_EXTENSIONS][EXTENSION_MCP_APPS_UI].isObject());

			// tools/list exposes the legacy tool and the Apps entry point.
			CHECK(ExpectResponse(client, 2, METHOD_TOOLS_LIST,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}"), jResponse));
			const Json::Value& jToolsResult = jResponse[MSG_KEY_RESULT];
			CHECK_EQ(std::string(RESULT_TYPE_COMPLETE), jToolsResult[MSG_KEY_RESULT_TYPE].asString());
			CHECK(jToolsResult.isMember(MSG_KEY_TTL_MS));
			CHECK_EQ(std::string(CACHE_SCOPE_PRIVATE), jToolsResult[MSG_KEY_CACHE_SCOPE].asString());

			Json::Value jTool;
			CHECK(FindTool(jToolsResult, CTestServer::TOOL_NAME, jTool));
			CHECK(FindTool(jToolsResult, CTestServer::APP_TOOL_NAME, jTool));
			CHECK_EQ(std::string(CTestServer::UI_RESOURCE_URI),
				jTool[MSG_KEY_META][MSG_KEY_UI][MSG_KEY_RESOURCE_URI].asString());

			// resources/list exposes the UI resource and its ui metadata.
			CHECK(ExpectResponse(client, 3, METHOD_RESOURCES_LIST,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}"), jResponse));
			Json::Value jUiResource;
			CHECK(FindResource(jResponse[MSG_KEY_RESULT], CTestServer::UI_RESOURCE_URI, jUiResource));
			CHECK_EQ(std::string(MIME_TYPE_MCP_APP), jUiResource[MSG_KEY_MIMETYPE].asString());
			CHECK(jUiResource[MSG_KEY_META][MSG_KEY_UI][MSG_KEY_CSP].isObject());
			CHECK(jUiResource[MSG_KEY_META][MSG_KEY_UI][MSG_KEY_PERMISSIONS].isObject());

			// resources/read returns the App payload with the same metadata.
			CHECK(ExpectResponse(client, 4, METHOD_RESOURCES_READ,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}", std::string(),
					"\"uri\":\"" + std::string(CTestServer::UI_RESOURCE_URI) + "\""), jResponse));
			const Json::Value& jUiContents = jResponse[MSG_KEY_RESULT][MSG_KEY_CONTENTS][0];
			CHECK_EQ(std::string(MIME_TYPE_MCP_APP), jUiContents[MSG_KEY_MIMETYPE].asString());
			CHECK(jUiContents[MSG_KEY_META][MSG_KEY_UI][MSG_KEY_CSP]["connectDomains"].isArray());
			CHECK_EQ(std::string("<html><body>Echo App</body></html>"), jUiContents[MSG_KEY_TEXT].asString());

			// Rich content blocks are serialized for the modern profile.
			CHECK(ExpectResponse(client, 5, METHOD_TOOLS_CALL,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}", std::string(),
					"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"rich\"}"), jResponse));
			const Json::Value& jRich = jResponse[MSG_KEY_RESULT];
			CHECK_EQ(std::string(RESULT_TYPE_COMPLETE), jRich[MSG_KEY_RESULT_TYPE].asString());
			CHECK(!jRich.isMember(MSG_KEY_TTL_MS));
			CHECK(!jRich.isMember(MSG_KEY_CACHE_SCOPE));
			CHECK_EQ(6, static_cast<int>(jRich[MSG_KEY_CONTENT].size()));
			CHECK_EQ(std::string(CONST_TEXT), jRich[MSG_KEY_CONTENT][0][MSG_KEY_TYPE].asString());
			CHECK_EQ(std::string(CONST_IMAGE), jRich[MSG_KEY_CONTENT][1][MSG_KEY_TYPE].asString());
			CHECK_EQ(std::string(CONST_AUDIO), jRich[MSG_KEY_CONTENT][2][MSG_KEY_TYPE].asString());
			CHECK_EQ(std::string(CONST_RESOURCE_LINK), jRich[MSG_KEY_CONTENT][3][MSG_KEY_TYPE].asString());
			CHECK_EQ(std::string(CONST_TOOL_USE), jRich[MSG_KEY_CONTENT][4][MSG_KEY_TYPE].asString());
			CHECK_EQ(std::string(CONST_TOOL_RESULT), jRich[MSG_KEY_CONTENT][5][MSG_KEY_TYPE].asString());

			// SDK reserved _meta keys win over application provided values.
			CHECK(ExpectResponse(client, 6, METHOD_TOOLS_CALL,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}", std::string(),
					"\"name\":\"" + std::string(CTestServer::APP_TOOL_NAME)
						+ "\",\"arguments\":{\"input\":\"x\",\"action\":\"meta\"}"), jResponse));
			const Json::Value& jResultMeta = jResponse[MSG_KEY_RESULT][MSG_KEY_META];
			CHECK_EQ(std::string("keep"), jResultMeta["example.com/custom"].asString());
			CHECK_EQ(std::string(CTestServer::SERVER_NAME),
				jResultMeta[META_KEY_SERVER_INFO][MSG_KEY_NAME].asString());
			CHECK(!jResultMeta.isMember(META_KEY_SUBSCRIPTION_ID));

			// Request level logLevel forwards notifications/message.
			size_t nLogStart = client.GetOutputCount();
			CHECK(ExpectResponse(client, 7, METHOD_TOOLS_CALL,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}", "\"io.modelcontextprotocol/logLevel\":\"info\"",
					"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"log\"}"), jResponse));
			size_t nLogIndex = 0;
			CHECK(client.WaitForMethod(METHOD_NOTIFICATION_MESSAGE, nLogStart, nLogIndex, E2E_WAIT_TIMEOUT_MS));
			Json::Value jLog;
			CHECK(ParseJson(client.GetOutput(nLogIndex), jLog));
			CHECK(!jLog.isMember(MSG_KEY_ID));
			CHECK_EQ(std::string("info"), jLog[MSG_KEY_PARAMS][MSG_KEY_LEVEL].asString());
			CHECK_EQ(std::string("echo"), jLog[MSG_KEY_PARAMS][MSG_KEY_LOGGER].asString());
			CHECK_EQ(std::string("echo log"),
				jLog[MSG_KEY_PARAMS][MSG_KEY_DATA][MSG_KEY_MESSAGE].asString());

			// Without logLevel the same task stays silent.
			size_t nNoLogStart = client.GetOutputCount();
			CHECK(ExpectResponse(client, 8, METHOD_TOOLS_CALL,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}", std::string(),
					"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"log\"}"), jResponse));
			CHECK(client.WaitForSilence(nNoLogStart + 1, E2E_QUIET_WAIT_MS));

			// MRTR round one: input_required with inputRequests and requestState.
			CHECK(ExpectResponse(client, 9, METHOD_TOOLS_CALL,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{\"elicitation\":{\"form\":{}}}", std::string(),
					"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"confirm\"}"), jResponse));
			const Json::Value& jInputRequired = jResponse[MSG_KEY_RESULT];
			CHECK_EQ(std::string(RESULT_TYPE_INPUT_REQUIRED), jInputRequired[MSG_KEY_RESULT_TYPE].asString());
			CHECK_EQ(std::string(METHOD_ELICITATION_CREATE),
				jInputRequired[MSG_KEY_INPUT_REQUESTS]["confirm"][MSG_KEY_METHOD].asString());
			CHECK_EQ(std::string("echo-confirm-state"),
				jInputRequired[MSG_KEY_REQUEST_STATE].asString());
			CHECK(!jInputRequired.isMember(MSG_KEY_TTL_MS));
			CHECK(!jInputRequired.isMember(MSG_KEY_CACHE_SCOPE));

			// MRTR round two: inputResponses plus the opaque requestState complete.
			CHECK(ExpectResponse(client, 10, METHOD_TOOLS_CALL,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}", std::string(),
					"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"confirm\"},"
					"\"inputResponses\":{\"confirm\":{\"action\":\"accept\",\"content\":{\"answer\":\"yes\"}}},"
					"\"requestState\":\"echo-confirm-state\""), jResponse));
			CHECK_EQ(std::string(RESULT_TYPE_COMPLETE),
				jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
			CHECK_EQ(std::string("confirmed"),
				jResponse[MSG_KEY_RESULT][MSG_KEY_CONTENT][0][MSG_KEY_TEXT].asString());
			CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_TTL_MS));

			// A mismatched requestState is rejected as a tool level error.
			CHECK(ExpectResponse(client, 11, METHOD_TOOLS_CALL,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}", std::string(),
					"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"confirm_tamper\"},"
					"\"requestState\":\"tampered\""), jResponse));
			CHECK(jResponse[MSG_KEY_RESULT][MSG_KEY_IS_ERROR].asBool());
			CHECK_EQ(std::string("tampered state rejected"),
				jResponse[MSG_KEY_RESULT][MSG_KEY_CONTENT][0][MSG_KEY_TEXT].asString());

			// Without the elicitation capability the server answers -32021.
			CHECK(ExpectError(client, 12, METHOD_TOOLS_CALL,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}", std::string(),
					"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"confirm\"}"),
				ERRNO_MISSING_REQUIRED_CLIENT_CAPABILITY, jResponse));
			CHECK(jResponse[MSG_KEY_ERROR][MSG_KEY_DATA][MSG_KEY_REQUIRED_CAPABILITIES]
				[MSG_KEY_ELICITATION][MSG_KEY_FORM].isObject());

			// Missing metadata and unknown versions map to distinct errors.
			CHECK(ExpectError(client, 13, METHOD_TOOLS_LIST,
				"{\"_meta\":{\"io.modelcontextprotocol/clientCapabilities\":{}}}",
				ERRNO_INVALID_PARAMS, jResponse));
			CHECK(ExpectError(client, 22, METHOD_TOOLS_LIST, std::string(),
				ERRNO_INVALID_REQUEST, jResponse));
			CHECK(ExpectError(client, 14, METHOD_TOOLS_LIST,
				MakeModernParams("2025-06-18", "{}"), ERRNO_UNSUPPORTED_PROTOCOL_VERSION, jResponse));
			const Json::Value& jVersionError = jResponse[MSG_KEY_ERROR][MSG_KEY_DATA];
			CHECK_EQ(std::string("2025-06-18"), jVersionError[MSG_KEY_REQUESTED].asString());
			CHECK(ContainsValue(jVersionError[MSG_KEY_SUPPORTED], PROTOCOL_VER_2024_11_05));
			CHECK(ContainsValue(jVersionError[MSG_KEY_SUPPORTED], PROTOCOL_VER_2026_07_28));

			// Unknown methods and legacy-only methods are -32601 in the modern era.
			CHECK(ExpectError(client, 15, "e2e/unknown",
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}"), ERRNO_METHOD_NOT_FOUND, jResponse));
			CHECK(ExpectError(client, 16, METHOD_LOGGING_SET_LEVEL,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}", std::string(), "\"level\":\"info\""),
				ERRNO_METHOD_NOT_FOUND, jResponse));

			// Listen subscription: acknowledgment carries the integer id.
			size_t nListenStart = client.GetOutputCount();
			CHECK_EQ(ERRNO_OK, client.Send(MakeRequest(17, METHOD_SUBSCRIPTIONS_LISTEN,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}", std::string(),
					"\"notifications\":{\"toolsListChanged\":true,\"resourcesListChanged\":true,"
					"\"resourceSubscriptions\":[\"" + std::string(CTestServer::RESOURCE_URI) + "\"]}"))));

			size_t nAckIndex = 0;
			CHECK(client.WaitForMethod(METHOD_NOTIFICATION_SUBSCRIPTIONS_ACKNOWLEDGED,
				nListenStart, nAckIndex, E2E_WAIT_TIMEOUT_MS));
			Json::Value jAck;
			CHECK(ParseJson(client.GetOutput(nAckIndex), jAck));
			CHECK_EQ(17, jAck[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asInt());
			CHECK(jAck[MSG_KEY_PARAMS][MSG_KEY_NOTIFICATIONS][MSG_KEY_RESOURCES_LIST_CHANGED].asBool());
			CHECK_EQ(1, static_cast<int>(jAck[MSG_KEY_PARAMS][MSG_KEY_NOTIFICATIONS]
				[MSG_KEY_RESOURCE_SUBSCRIPTIONS].size()));

			// Granted list_changed event carries the subscription id.
			size_t nListEventStart = client.GetOutputCount();
			CHECK(ExpectResponse(client, 18, METHOD_TOOLS_CALL,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}", std::string(),
					"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"notify_list_changed\"}"),
				jResponse));
			size_t nListEventIndex = 0;
			CHECK(client.WaitForMethod(METHOD_NOTIFICATION_RESOURCES_LIST_CHANGED,
				nListEventStart, nListEventIndex, E2E_WAIT_TIMEOUT_MS));
			Json::Value jListEvent;
			CHECK(ParseJson(client.GetOutput(nListEventIndex), jListEvent));
			CHECK_EQ(17, jListEvent[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asInt());

			// Granted resource subscription delivers resources/updated.
			size_t nUpdatedStart = client.GetOutputCount();
			CHECK(ExpectResponse(client, 19, METHOD_TOOLS_CALL,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}", std::string(),
					"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"notify_resource\"}"),
				jResponse));
			size_t nUpdatedIndex = 0;
			CHECK(client.WaitForMethod(METHOD_NOTIFICATION_RESOURCES_UPDATED,
				nUpdatedStart, nUpdatedIndex, E2E_WAIT_TIMEOUT_MS));
			Json::Value jUpdated;
			CHECK(ParseJson(client.GetOutput(nUpdatedIndex), jUpdated));
			CHECK_EQ(std::string(CTestServer::RESOURCE_URI), jUpdated[MSG_KEY_PARAMS][MSG_KEY_URI].asString());
			CHECK_EQ(17, jUpdated[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asInt());

			// Cancelling the listen request stops the delivery. Stateless
			// notifications must carry their own _meta to reach the handler.
			CHECK_EQ(ERRNO_OK, client.Send(
				"{\"jsonrpc\":\"2.0\",\"method\":\"notifications/cancelled\",\"params\":{\"requestId\":17,"
				"\"_meta\":{\"io.modelcontextprotocol/protocolVersion\":\"" + std::string(PROTOCOL_VER_2026_07_28)
				+ "\",\"io.modelcontextprotocol/clientCapabilities\":{}}}}"));
			std::this_thread::sleep_for(std::chrono::milliseconds(E2E_QUIET_WAIT_MS));

			size_t nAfterCancelStart = client.GetOutputCount();
			CHECK(ExpectResponse(client, 20, METHOD_TOOLS_CALL,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}", std::string(),
					"\"name\":\"echo\",\"arguments\":{\"input\":\"x\",\"action\":\"notify_resource\"}"),
				jResponse));
			CHECK(client.WaitForSilence(nAfterCancelStart + 1, E2E_QUIET_WAIT_MS));

			// A string listen id stays a string in the acknowledgment.
			size_t nStringListenStart = client.GetOutputCount();
			CHECK_EQ(ERRNO_OK, client.Send(MakeRequestWithId("\"e2e-sub\"", METHOD_SUBSCRIPTIONS_LISTEN,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}", std::string(),
					"\"notifications\":{\"resourcesListChanged\":true}"))));
			size_t nStringAckIndex = 0;
			CHECK(client.WaitForMethod(METHOD_NOTIFICATION_SUBSCRIPTIONS_ACKNOWLEDGED,
				nStringListenStart, nStringAckIndex, E2E_WAIT_TIMEOUT_MS));
			Json::Value jStringAck;
			CHECK(ParseJson(client.GetOutput(nStringAckIndex), jStringAck));
			CHECK(jStringAck[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].isString());
			CHECK_EQ(std::string("e2e-sub"),
				jStringAck[MSG_KEY_PARAMS][MSG_KEY_META][META_KEY_SUBSCRIPTION_ID].asString());

			// Closing stdin shuts the server down and completes the live listen.
			CHECK_EQ(ERRNO_OK, client.Stop(0));
			CHECK(HasStringIdResult(client, "e2e-sub"));
		}

		// Both eras on one connection: modern first, then the legacy handshake.
		TEST_CASE("e2e.dual_era_stdio")
		{
			CStdioClient client;
			int iErrCode = client.Start(false);
			CHECK_EQ(ERRNO_OK, iErrCode);
			if (ERRNO_OK != iErrCode)
				return;

			Json::Value jResponse;

			CHECK(ExpectResponse(client, 1, METHOD_SERVER_DISCOVER,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}"), jResponse));

			CHECK(ExpectResponse(client, 2, METHOD_INITIALIZE,
				"{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},\"clientInfo\":{\"name\":\"TinyMCPE2E\",\"version\":\"1.0.0\"}}",
				jResponse));
			CHECK_EQ(ERRNO_OK, client.Send("{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}"));

			// Legacy view shares the registered surface but strips modern metadata.
			CHECK(ExpectResponse(client, 3, METHOD_TOOLS_LIST, std::string(), jResponse));
			CHECK(!jResponse[MSG_KEY_RESULT].isMember(MSG_KEY_RESULT_TYPE));
			CHECK_EQ(2, static_cast<int>(jResponse[MSG_KEY_RESULT][MSG_KEY_TOOLS].size()));
			for (Json::ArrayIndex i = 0; i < jResponse[MSG_KEY_RESULT][MSG_KEY_TOOLS].size(); ++i)
				CHECK(!jResponse[MSG_KEY_RESULT][MSG_KEY_TOOLS][i].isMember(MSG_KEY_META));

			// The stateless view still carries the modern fields and the Apps tool.
			CHECK(ExpectResponse(client, 4, METHOD_TOOLS_LIST,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}"), jResponse));
			CHECK_EQ(std::string(RESULT_TYPE_COMPLETE),
				jResponse[MSG_KEY_RESULT][MSG_KEY_RESULT_TYPE].asString());
			CHECK_EQ(2, static_cast<int>(jResponse[MSG_KEY_RESULT][MSG_KEY_TOOLS].size()));

			// The legacy resource view never carries _meta either.
			CHECK(ExpectResponse(client, 5, METHOD_RESOURCES_LIST, std::string(), jResponse));
			CHECK_EQ(2, static_cast<int>(jResponse[MSG_KEY_RESULT][MSG_KEY_RESOURCES].size()));
			for (Json::ArrayIndex i = 0; i < jResponse[MSG_KEY_RESULT][MSG_KEY_RESOURCES].size(); ++i)
				CHECK(!jResponse[MSG_KEY_RESULT][MSG_KEY_RESOURCES][i].isMember(MSG_KEY_META));

			CHECK(ExpectResponse(client, 6, METHOD_RESOURCES_LIST,
				MakeModernParams(PROTOCOL_VER_2026_07_28, "{}"), jResponse));
			CHECK_EQ(2, static_cast<int>(jResponse[MSG_KEY_RESULT][MSG_KEY_RESOURCES].size()));

			CHECK_EQ(ERRNO_OK, client.Stop(0));
		}
	}
}
