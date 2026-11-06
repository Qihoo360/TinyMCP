#include "../TestHarness.h"

#include <Message/Request.h>
#include <Message/Response.h>
#include <Public/PublicDef.h>
#include <Internal/Protocol.h>
#include <Internal/MessageAccess.h>

#include <json/writer.h>
#include <string>

using namespace MCP;

namespace
{
	void SetLegacyResponseId(MCP::Response& response, const std::string& strId)
	{
		response.requestId.eIdDataType = DataType_String;
		response.requestId.strId = strId;
	}

	std::string SerializeMessage(const MCP::Message& message)
	{
		Json::Value jMsg;
		int iErrCode = message.DoSerialize(jMsg);
		CHECK_EQ(ERRNO_OK, iErrCode);

		Json::StreamWriterBuilder builder;
		builder["indentation"] = "";

		return Json::writeString(builder, jMsg);
	}

	bool ParseJson(const std::string& strText, Json::Value& jValue)
	{
		Json::Reader reader;
		return reader.parse(strText, jValue) && jValue.isObject();
	}
}

TEST_CASE("unit.legacy_compat")
{
	// The global registry keeps 2024-11-05 as the handshake revision.
	const std::shared_ptr<const ProtocolProfile> pHandshakeProfile = CProtocolRegistry::GetGlobalRegistry().GetDefaultHandshakeProfile();
	CHECK(pHandshakeProfile != nullptr);
	if (pHandshakeProfile)
		CHECK_EQ(std::string("2024-11-05"), pHandshakeProfile->strVersion);

	// A default (empty) protocol context must reproduce the legacy result shape.
	CallToolResult callResult(false);
	SetLegacyResponseId(callResult, "legacy-1");

	TextContent textContent;
	textContent.strType = CONST_TEXT;
	textContent.strText = "hello";
	callResult.vecTextContent.push_back(textContent);

	std::string strSerialized = SerializeMessage(callResult);
	CHECK(strSerialized.find(MSG_KEY_RESULT_TYPE) == std::string::npos);
	CHECK(strSerialized.find(MSG_KEY_TTL_MS) == std::string::npos);
	CHECK(strSerialized.find(MSG_KEY_CACHE_SCOPE) == std::string::npos);
	CHECK(strSerialized.find(MSG_KEY_META) == std::string::npos);

	Json::Value jLegacyResult;
	CHECK(ParseJson(strSerialized, jLegacyResult));
	CHECK_EQ(std::string("hello"),
		jLegacyResult[MSG_KEY_RESULT][MSG_KEY_CONTENT][0][MSG_KEY_TEXT].asString());

	// Content blocks introduced by the stateless era are gated out.
	AudioContent audioContent;
	audioContent.strType = CONST_AUDIO;
	audioContent.strMimeType = "audio/wav";
	audioContent.strData = "YQ==";
	callResult.vecAudioContent.push_back(audioContent);

	ResourceLink resourceLink;
	resourceLink.strType = CONST_RESOURCE_LINK;
	resourceLink.strName = "echo.txt";
	resourceLink.strUri = "test://echo/resource";
	callResult.vecResourceLinkContent.push_back(resourceLink);

	strSerialized = SerializeMessage(callResult);
	CHECK(strSerialized.find(CONST_AUDIO) == std::string::npos);
	CHECK(strSerialized.find(CONST_RESOURCE_LINK) == std::string::npos);

	// Entity _meta only appears with a modern protocol context.
	ListToolsResult listToolsResult(false);
	SetLegacyResponseId(listToolsResult, "legacy-2");

	Tool tool;
	tool.strName = "echo";
	tool.strDescription = "Echo tool";
	tool.SetUiResourceUri("ui://echo/app.html");
	listToolsResult.vecTools.push_back(tool);

	strSerialized = SerializeMessage(listToolsResult);
	CHECK(strSerialized.find(MSG_KEY_META) == std::string::npos);
	CHECK(strSerialized.find(MSG_KEY_RESOURCE_URI) == std::string::npos);
	CHECK(strSerialized.find(MSG_KEY_RESULT_TYPE) == std::string::npos);

	// Error responses keep the legacy code/message/data layout.
	ErrorResponse errorResponse(false);
	SetLegacyResponseId(errorResponse, "legacy-3");
	errorResponse.iCode = ERRNO_INVALID_PARAMS;
	errorResponse.strMesage = "bad params";
	errorResponse.jErrorData[MSG_KEY_URI] = "test://echo/resource";

	Json::Value jErrorResponse;
	CHECK_EQ(ERRNO_OK, errorResponse.DoSerialize(jErrorResponse));
	CHECK_EQ(ERRNO_INVALID_PARAMS, jErrorResponse[MSG_KEY_ERROR][MSG_KEY_CODE].asInt());
	CHECK_EQ(std::string("bad params"),
		jErrorResponse[MSG_KEY_ERROR][MSG_KEY_MESSAGE].asString());
	CHECK_EQ(std::string("test://echo/resource"),
		jErrorResponse[MSG_KEY_ERROR][MSG_KEY_DATA][MSG_KEY_URI].asString());

	// Legacy request parsing stays version agnostic: progressToken is not a
	// modern opt in, so the request resolves to the default handshake profile.
	Json::Value jRequest;
	CHECK(ParseJson("{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/call\","
		"\"params\":{\"name\":\"echo\",\"arguments\":{\"input\":\"x\"},"
		"\"_meta\":{\"progressToken\":\"p-1\"}}}", jRequest));

	CallToolRequest callToolRequest(false);
	CHECK_EQ(ERRNO_OK, callToolRequest.DoDeserialize(jRequest));
	CHECK_EQ(std::string("echo"), callToolRequest.strName);
	CHECK(MessageContextAccess::Get(callToolRequest).IsValid());
	CHECK(!MessageContextAccess::Get(callToolRequest).IsStateless());
	CHECK_EQ(std::string("2024-11-05"), MessageContextAccess::Get(callToolRequest).strVersion);
}
