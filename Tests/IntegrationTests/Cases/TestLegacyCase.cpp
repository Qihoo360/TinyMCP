#include "../../TestHarness.h"
#include "../Driver.h"
#include "../Golden.h"
#include "../TestServer.h"

#include <Public/PublicDef.h>
#include <Message/Request.h>

#include <fstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

namespace MCP
{
	namespace Test
	{
		namespace
		{
			const int WAIT_TIMEOUT_MS = 5000;
			const size_t LEGACY_RESPONSE_COUNT = 13;
			const char* GOLDEN_FILE_NAME = "legacy_responses.txt";

			std::string GoldenFilePath()
			{
				return std::string(TINYMCP_GOLDEN_DIR) + "/" + GOLDEN_FILE_NAME;
			}

			std::string MakeRequest(int iId, const std::string& strMethod, const std::string& strParams)
			{
				std::string strRequest = "{\"jsonrpc\":\"2.0\",\"id\":" + std::to_string(iId) + ",\"method\":\"" + strMethod + "\"";
				if (!strParams.empty())
					strRequest += ",\"params\":" + strParams;
				strRequest += "}";

				return strRequest;
			}

			bool ParseJson(const std::string& strText, Json::Value& jValue)
			{
				Json::Reader reader;
				return reader.parse(strText, jValue) && jValue.isObject();
			}

			bool FindKeyPath(const Json::Value& jValue, const std::string& strKey, const std::string& strPath, std::string& strFound)
			{
				if (jValue.isObject())
				{
					auto vecNames = jValue.getMemberNames();
					for (auto& strName : vecNames)
					{
						std::string strChildPath = strPath.empty() ? strName : (strPath + "." + strName);
						if (strName == strKey)
						{
							strFound = strChildPath;
							return true;
						}

						if (FindKeyPath(jValue[strName], strKey, strChildPath, strFound))
							return true;
					}
				}
				else if (jValue.isArray())
				{
					for (Json::ArrayIndex i = 0; i < jValue.size(); ++i)
					{
						if (FindKeyPath(jValue[i], strKey, strPath + "[" + std::to_string(i) + "]", strFound))
							return true;
					}
				}

				return false;
			}

			bool HasServerInfoInMeta(const Json::Value& jValue)
			{
				if (jValue.isObject())
				{
					auto vecNames = jValue.getMemberNames();
					for (auto& strName : vecNames)
					{
						if (strName == MCP::MSG_KEY_META && jValue[strName].isObject() && jValue[strName].isMember(MCP::MSG_KEY_SERVER_INFO))
							return true;

						if (HasServerInfoInMeta(jValue[strName]))
							return true;
					}
				}
				else if (jValue.isArray())
				{
					for (Json::ArrayIndex i = 0; i < jValue.size(); ++i)
					{
						if (HasServerInfoInMeta(jValue[i]))
							return true;
					}
				}

				return false;
			}

			void CheckLegacyShape(const Json::Value& jResponse)
			{
				static const char* FORBIDDEN_KEYS[] = {
					"resultType", "ttlMs", "cacheScope", "extensions", "structuredContent",
					"title", "icons", "outputSchema", "annotations",
					"audio", "resource_link", "tool_use", "tool_result"
				};

				for (auto strKey : FORBIDDEN_KEYS)
				{
					std::string strFound;
					if (FindKeyPath(jResponse, strKey, "", strFound))
						MCP::Test::CTestContext::GetInstance().Fail(__FILE__, __LINE__, "legacy response must not contain key: " + strFound);
				}

				if (HasServerInfoInMeta(jResponse))
					MCP::Test::CTestContext::GetInstance().Fail(__FILE__, __LINE__, "legacy response must not contain _meta.serverInfo");

				std::string strMetaPath;
				if (FindKeyPath(jResponse, MCP::MSG_KEY_META, "", strMetaPath))
					MCP::Test::CTestContext::GetInstance().Fail(__FILE__, __LINE__, "legacy response must not contain _meta: " + strMetaPath);
			}

			bool ExpectSuccess(CDriver& driver, int iId, const std::string& strMethod, const std::string& strParams, Json::Value& jResponse, std::vector<std::string>& vecGolden)
			{
				size_t nBefore = driver.GetOutputCount();
				if (MCP::ERRNO_OK != driver.Send(MakeRequest(iId, strMethod, strParams)))
					return false;

				if (!driver.WaitForOutputCount(nBefore + 1, WAIT_TIMEOUT_MS))
					return false;

				std::string strResponse = driver.GetOutput(nBefore);
				if (!ParseJson(strResponse, jResponse))
					return false;

				CHECK_EQ(jResponse["id"].asInt(), iId);
				CHECK(jResponse.isMember(MCP::MSG_KEY_RESULT));
				if (!jResponse.isMember(MCP::MSG_KEY_RESULT))
					return false;

				CheckLegacyShape(jResponse);
				vecGolden.push_back(strResponse);

				return true;
			}

			bool ExpectError(CDriver& driver, int iId, const std::string& strMethod, const std::string& strParams, int iErrorCode, Json::Value& jResponse)
			{
				size_t nBefore = driver.GetOutputCount();
				if (MCP::ERRNO_OK != driver.Send(MakeRequest(iId, strMethod, strParams)))
					return false;

				if (!driver.WaitForOutputCount(nBefore + 1, WAIT_TIMEOUT_MS))
					return false;

				if (!ParseJson(driver.GetOutput(nBefore), jResponse))
					return false;

				CHECK_EQ(jResponse["id"].asInt(), iId);
				CHECK(jResponse.isMember(MCP::MSG_KEY_ERROR));
				CHECK_EQ(jResponse["error"]["code"].asInt(), iErrorCode);

				return true;
			}

			void CheckLegacyMeta()
			{
				// A request with a progress token must carry exactly that key in params._meta.
				MCP::CallToolRequest requestWithToken(true);
				requestWithToken.strMethod = MCP::METHOD_TOOLS_CALL;
				requestWithToken.requestId.eIdDataType = MCP::DataType_Integer;
				requestWithToken.requestId.iId = 100;
				requestWithToken.strName = CTestServer::TOOL_NAME;
				requestWithToken.progressToken.eTokenDataType = MCP::DataType_String;
				requestWithToken.progressToken.strToken = "progress-1";

				std::string strSerialized;
				CHECK_EQ(requestWithToken.Serialize(strSerialized), MCP::ERRNO_OK);

				Json::Value jSerialized;
				CHECK(ParseJson(strSerialized, jSerialized));
				CHECK(jSerialized["params"].isMember(MCP::MSG_KEY_META));
				if (jSerialized["params"].isMember(MCP::MSG_KEY_META))
				{
					CHECK_EQ(jSerialized["params"][MCP::MSG_KEY_META].getMemberNames().size(), static_cast<size_t>(1));
					CHECK(jSerialized["params"][MCP::MSG_KEY_META].isMember(MCP::MSG_KEY_PROGRESS_TOKEN));
				}

				// A request without a progress token must not emit an empty params._meta.
				MCP::CallToolRequest requestWithoutToken(true);
				requestWithoutToken.strMethod = MCP::METHOD_TOOLS_CALL;
				requestWithoutToken.requestId.eIdDataType = MCP::DataType_Integer;
				requestWithoutToken.requestId.iId = 101;
				requestWithoutToken.strName = CTestServer::TOOL_NAME;

				std::string strSerializedNoToken;
				CHECK_EQ(requestWithoutToken.Serialize(strSerializedNoToken), MCP::ERRNO_OK);

				Json::Value jSerializedNoToken;
				CHECK(ParseJson(strSerializedNoToken, jSerializedNoToken));
				CHECK(!jSerializedNoToken["params"].isMember(MCP::MSG_KEY_META));

				// params._meta.toolCallId is still parsed by the legacy CallToolRequest.
				const char* pIncomingRequest = "{\"jsonrpc\":\"2.0\",\"id\":102,\"method\":\"tools/call\",\"params\":{\"name\":\"echo\",\"_meta\":{\"progressToken\":\"progress-2\",\"toolCallId\":\"call-1\"}}}";
				MCP::CallToolRequest parsedRequest(true);
				CHECK_EQ(parsedRequest.Deserialize(pIncomingRequest), MCP::ERRNO_OK);
				CHECK_EQ(parsedRequest.toolCallId.eIdDataType, MCP::DataType_String);
				CHECK_EQ(parsedRequest.toolCallId.strId, std::string("call-1"));
				CHECK_EQ(parsedRequest.progressToken.strToken, std::string("progress-2"));
			}

			int RunLegacyScene(std::vector<std::string>& vecGolden)
			{
				MCP::Test::CTestServer& server = MCP::Test::CTestServer::s_Instance;
				int iErrCode = server.Initialize();
				CHECK_EQ(iErrCode, MCP::ERRNO_OK);
				if (MCP::ERRNO_OK != iErrCode)
					return iErrCode;

				CDriver driver;
				iErrCode = driver.Start(WAIT_TIMEOUT_MS);
				CHECK_EQ(iErrCode, MCP::ERRNO_OK);
				if (MCP::ERRNO_OK != iErrCode)
					return iErrCode;

				Json::Value jResponse;

				// 1. initialize
				bool bStep = ExpectSuccess(driver, 1, MCP::METHOD_INITIALIZE,
					"{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{},\"clientInfo\":{\"name\":\"TinyMCPTests\",\"version\":\"1.0.0\"}}",
					jResponse, vecGolden);
				CHECK(bStep);
				if (!bStep)
					return MCP::ERRNO_INTERNAL_ERROR;

				CHECK_EQ(jResponse["result"]["protocolVersion"].asString(), std::string(MCP::PROTOCOL_VER));
				CHECK_EQ(jResponse["result"]["serverInfo"]["name"].asString(), std::string(CTestServer::SERVER_NAME));
				CHECK_EQ(jResponse["result"]["capabilities"]["tools"].isObject(), true);
				CHECK_EQ(jResponse["result"]["capabilities"]["resources"]["subscribe"].asBool(), true);
				CHECK_EQ(jResponse["result"]["capabilities"]["resources"]["listChanged"].asBool(), true);
				CHECK_EQ(jResponse["result"]["capabilities"]["prompts"]["listChanged"].asBool(), true);
				CHECK(jResponse["result"]["capabilities"]["experimental"].isMember(MCP::MSG_KEY_COMPLETION));
				CHECK(jResponse["result"]["capabilities"].isMember(MCP::MSG_KEY_LOGGING));

				// 2. notifications/initialized (no response is expected).
				CHECK_EQ(driver.Send("{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}"), MCP::ERRNO_OK);

				// 3. tools/list
				bStep = ExpectSuccess(driver, 2, MCP::METHOD_TOOLS_LIST, "", jResponse, vecGolden);
				CHECK(bStep);
				if (!bStep)
					return MCP::ERRNO_INTERNAL_ERROR;
				CHECK_EQ(jResponse["result"]["tools"].size(), static_cast<Json::ArrayIndex>(1));
				CHECK_EQ(jResponse["result"]["tools"][0]["name"].asString(), std::string(CTestServer::TOOL_NAME));

				// 4. tools/call
				bStep = ExpectSuccess(driver, 3, MCP::METHOD_TOOLS_CALL,
					"{\"name\":\"echo\",\"arguments\":{\"input\":\"hello legacy\"}}", jResponse, vecGolden);
				CHECK(bStep);
				if (!bStep)
					return MCP::ERRNO_INTERNAL_ERROR;
				CHECK_EQ(jResponse["result"]["content"][0]["text"].asString(), std::string("hello legacy"));
				CHECK_EQ(jResponse["result"]["isError"].asBool(), false);

				// 5. resources/list
				bStep = ExpectSuccess(driver, 4, MCP::METHOD_RESOURCES_LIST, "", jResponse, vecGolden);
				CHECK(bStep);
				if (!bStep)
					return MCP::ERRNO_INTERNAL_ERROR;
				CHECK_EQ(jResponse["result"]["resources"].size(), static_cast<Json::ArrayIndex>(1));
				CHECK_EQ(jResponse["result"]["resources"][0]["uri"].asString(), std::string(CTestServer::RESOURCE_URI));

				// 6. resources/read
				bStep = ExpectSuccess(driver, 5, MCP::METHOD_RESOURCES_READ,
					"{\"uri\":\"test://echo/resource\"}", jResponse, vecGolden);
				CHECK(bStep);
				if (!bStep)
					return MCP::ERRNO_INTERNAL_ERROR;
				CHECK_EQ(jResponse["result"]["contents"][0]["uri"].asString(), std::string(CTestServer::RESOURCE_URI));
				CHECK_EQ(jResponse["result"]["contents"][0]["text"].asString(), std::string("legacy resource payload"));

				// 7. resources/templates/list
				bStep = ExpectSuccess(driver, 6, MCP::METHOD_RESOURCES_TEMPLATES_LIST, "", jResponse, vecGolden);
				CHECK(bStep);
				if (!bStep)
					return MCP::ERRNO_INTERNAL_ERROR;
				CHECK_EQ(jResponse["result"]["resourceTemplates"].size(), static_cast<Json::ArrayIndex>(1));
				CHECK_EQ(jResponse["result"]["resourceTemplates"][0]["uriTemplate"].asString(), std::string("test://echo/{name}"));

				// 8. resources/subscribe
				bStep = ExpectSuccess(driver, 7, MCP::METHOD_RESOURCES_SUBSCRIBE,
					"{\"uri\":\"test://echo/resource\"}", jResponse, vecGolden);
				CHECK(bStep);
				if (!bStep)
					return MCP::ERRNO_INTERNAL_ERROR;

				// 9. resources/unsubscribe
				bStep = ExpectSuccess(driver, 8, MCP::METHOD_RESOURCES_UNSUBSCRIBE,
					"{\"uri\":\"test://echo/resource\"}", jResponse, vecGolden);
				CHECK(bStep);
				if (!bStep)
					return MCP::ERRNO_INTERNAL_ERROR;

				// 10. prompts/list
				bStep = ExpectSuccess(driver, 9, MCP::METHOD_PROMPTS_LIST, "", jResponse, vecGolden);
				CHECK(bStep);
				if (!bStep)
					return MCP::ERRNO_INTERNAL_ERROR;
				CHECK_EQ(jResponse["result"]["prompts"].size(), static_cast<Json::ArrayIndex>(1));
				CHECK_EQ(jResponse["result"]["prompts"][0]["name"].asString(), std::string(CTestServer::PROMPT_NAME));

				// 11. prompts/get
				bStep = ExpectSuccess(driver, 10, MCP::METHOD_PROMPTS_GET,
					"{\"name\":\"echo_prompt\",\"arguments\":{\"text\":\"hi\"}}", jResponse, vecGolden);
				CHECK(bStep);
				if (!bStep)
					return MCP::ERRNO_INTERNAL_ERROR;
				CHECK_EQ(jResponse["result"]["messages"][0]["role"].asString(), std::string(MCP::CONST_USER));
				CHECK_EQ(jResponse["result"]["messages"][0]["content"]["text"].asString(), std::string("hello from prompt"));

				// 12. completion/complete
				bStep = ExpectSuccess(driver, 11, MCP::METHOD_COMPLETION_COMPLETE,
					"{\"ref\":{\"type\":\"ref/prompt\",\"name\":\"echo_prompt\"},\"argument\":{\"name\":\"text\",\"value\":\"a\"}}",
					jResponse, vecGolden);
				CHECK(bStep);
				if (!bStep)
					return MCP::ERRNO_INTERNAL_ERROR;
				CHECK_EQ(jResponse["result"]["completion"]["values"].size(), static_cast<Json::ArrayIndex>(2));
				CHECK_EQ(jResponse["result"]["completion"]["values"][0].asString(), std::string("alpha"));

				// 13. logging/setLevel
				bStep = ExpectSuccess(driver, 12, MCP::METHOD_LOGGING_SET_LEVEL, "{\"level\":\"info\"}", jResponse, vecGolden);
				CHECK(bStep);
				if (!bStep)
					return MCP::ERRNO_INTERNAL_ERROR;
				CHECK_EQ(jResponse["result"].size(), static_cast<Json::ArrayIndex>(0));

				// 14. ping
				bStep = ExpectSuccess(driver, 13, MCP::METHOD_PING, "", jResponse, vecGolden);
				CHECK(bStep);
				if (!bStep)
					return MCP::ERRNO_INTERNAL_ERROR;
				CHECK_EQ(jResponse["result"].size(), static_cast<Json::ArrayIndex>(0));

				// 15. Legacy unknown method. Step 4 of the plan adds the -32601
				// answer for server/discover; before that legacy code silently drops
				// unknown methods. A successful response is never allowed here.
				size_t nProbeStart = driver.GetOutputCount();
				CHECK_EQ(driver.Send(MakeRequest(14, "server/discover", "")), MCP::ERRNO_OK);
				CHECK_EQ(driver.Send(MakeRequest(15, MCP::METHOD_PING, "")), MCP::ERRNO_OK);
				bool bProbe = driver.WaitForOutputCount(nProbeStart + 1, WAIT_TIMEOUT_MS);
				CHECK(bProbe);
				if (bProbe)
				{
					Json::Value jFirst;
					CHECK(ParseJson(driver.GetOutput(nProbeStart), jFirst));
					if (jFirst["id"].asInt() == 14)
					{
						CHECK(jFirst.isMember(MCP::MSG_KEY_ERROR));
						CHECK_EQ(jFirst["error"]["code"].asInt(), MCP::ERRNO_METHOD_NOT_FOUND);
						CHECK(driver.WaitForOutputCount(nProbeStart + 2, WAIT_TIMEOUT_MS));
					}
					else
					{
						CHECK_EQ(jFirst["id"].asInt(), 15);
					}
				}

				// 16. Legacy resource-not-found error code is preserved.
				CHECK_EQ(MCP::ERRNO_HEADER_MISMATCH, -32001);
				CHECK_EQ(MCP::ERRNO_RESOURCE_NOT_FOUND, -32002);
				bStep = ExpectError(driver, 16, MCP::METHOD_RESOURCES_READ,
					"{\"uri\":\"test://echo/missing\"}", MCP::ERRNO_RESOURCE_NOT_FOUND, jResponse);
				CHECK(bStep);

				// 17. params._meta compatibility of the legacy request objects.
				CheckLegacyMeta();

				driver.Stop();

				return MCP::ERRNO_OK;
			}

			bool ReadGoldenLines(std::vector<std::string>& vecLines)
			{
				std::ifstream ifs(GoldenFilePath().c_str(), std::ios::binary);
				if (!ifs)
					return false;

				std::string strLine;
				while (std::getline(ifs, strLine))
				{
					if (!strLine.empty() && strLine.back() == '\r')
						strLine.pop_back();
					vecLines.push_back(strLine);
				}

				return true;
			}

			bool WriteGoldenLines(const std::vector<std::string>& vecLines)
			{
#ifdef _WIN32
				_mkdir(TINYMCP_GOLDEN_DIR);
#else
				mkdir(TINYMCP_GOLDEN_DIR, 0755);
#endif
				std::ofstream ofs(GoldenFilePath().c_str(), std::ios::binary | std::ios::trunc);
				if (!ofs)
					return false;

				for (auto& strLine : vecLines)
					ofs << strLine << "\n";

				return ofs.good();
			}
		}

		int GenerateLegacyGolden()
		{
			MCP::Test::CTestContext::GetInstance().Begin("generate.legacy");

			std::vector<std::string> vecGolden;
			int iErrCode = RunLegacyScene(vecGolden);
			if (MCP::ERRNO_OK != iErrCode)
			{
				std::cout << "Legacy scene failed while generating the golden baseline." << std::endl;
				return 1;
			}

			if (!WriteGoldenLines(vecGolden))
			{
				std::cout << "Failed to write golden file: " << GoldenFilePath() << std::endl;
				return 1;
			}

			std::cout << "Wrote " << vecGolden.size() << " line(s) to " << GoldenFilePath() << std::endl;

			return 0;
		}

		TEST_CASE("integration.legacy")
		{
			std::vector<std::string> vecActual;
			int iErrCode = RunLegacyScene(vecActual);
			CHECK_EQ(iErrCode, MCP::ERRNO_OK);
			CHECK_EQ(vecActual.size(), LEGACY_RESPONSE_COUNT);

			std::vector<std::string> vecGolden;
			bool bRead = ReadGoldenLines(vecGolden);
			CHECK(bRead);
			if (!bRead)
				return;

			CHECK_EQ(vecActual.size(), vecGolden.size());
			size_t nCount = vecActual.size() < vecGolden.size() ? vecActual.size() : vecGolden.size();
			for (size_t i = 0; i < nCount; ++i)
			{
				if (vecActual[i] != vecGolden[i])
				{
					MCP::Test::CTestContext::GetInstance().FailEq(__FILE__, __LINE__,
						"actual[" + std::to_string(i) + "]", "golden[" + std::to_string(i) + "]",
						vecActual[i], vecGolden[i]);
				}
			}
		}
	}
}
