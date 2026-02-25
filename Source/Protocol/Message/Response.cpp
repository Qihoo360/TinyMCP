#include "Response.h"
#include <json/writer.h>

namespace MCP
{
	////////////////////////////////////////////////////////////////////////////////////////
	// Response
	int Response::DoSerialize(Json::Value& jMsg) const
	{
		if (!IsValid())
			return ERRNO_INVALID_RESPONSE;

		Json::Value jRPC(JSON_RPC_VER);
		jMsg[MSG_KEY_JSONRPC] = jRPC;

		return requestId.DoSerialize(jMsg);
	}

	int Response::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_JSONRPC) || !jMsg[MSG_KEY_JSONRPC].isString())
			return ERRNO_INVALID_RESPONSE;
		auto strRPCVer = jMsg[MSG_KEY_JSONRPC].asString();
		if (strRPCVer.compare(JSON_RPC_VER) != 0)
			return ERRNO_INVALID_RESPONSE;

		return requestId.DoDeserialize(jMsg);
	}

	bool Response::IsValid() const
	{
		return requestId.IsValid();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// EmptyResponse
	int EmptyResponse::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jResult(Json::objectValue);
		jMsg[MSG_KEY_RESULT] = jResult;

		return Response::DoSerialize(jMsg);
	}

	int EmptyResponse::DoDeserialize(const Json::Value& jMsg)
	{
		return Response::DoDeserialize(jMsg);
	}

	bool EmptyResponse::IsValid() const
	{
		return Response::IsValid();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ErrorResponse
	int ErrorResponse::DoSerialize(Json::Value& jMsg) const
	{
		if (!IsValid())
			return ERRNO_INVALID_RESPONSE;

		int iErrCode = Response::DoSerialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		Json::Value jError(Json::objectValue);
		Json::Value jCode(iCode);
		jError[MSG_KEY_CODE] = jCode;
		Json::Value jMessage(strMesage);
		jError[MSG_KEY_MESSAGE] = jMessage;

		jMsg[MSG_KEY_ERROR] = jError;

		return ERRNO_OK;
	}

	int ErrorResponse::DoDeserialize(const Json::Value& jMsg)
	{
		return Response::DoDeserialize(jMsg);
	}

	bool ErrorResponse::IsValid() const
	{
		return Response::IsValid();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// InitializeResult
	int InitializeResult::DoSerialize(Json::Value& jMsg) const
	{
		if (!IsValid())
			return ERRNO_INVALID_RESPONSE;

		Json::Value jResult(Json::objectValue);
		
		Json::Value jProtocolVersion(strProtocolVersion);
		jResult[MSG_KEY_PROTOCOL_VERSION] = jProtocolVersion;

		auto fnSerializeMember = [&jResult](const auto& objMember, const char* lpcszKey) -> int
		{
			std::string strMember;
			Json::Value jMember(Json::objectValue);
			int iErrCode = objMember.DoSerialize(jMember);
			if (ERRNO_OK != iErrCode)
				return iErrCode;
			jResult[lpcszKey] = jMember;
			
			return ERRNO_OK;
		};
		int iErrCode = fnSerializeMember(capabilities, MSG_KEY_CAPABILITIES);
		if (ERRNO_OK != iErrCode)
			return iErrCode;
		iErrCode = fnSerializeMember(implServerInfo, MSG_KEY_SERVER_INFO);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		iErrCode = Response::DoSerialize(jMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;
		jMsg[MSG_KEY_RESULT] = jResult;

		return ERRNO_OK;
	}

	int InitializeResult::DoDeserialize(const Json::Value& jMsg)
	{
		return Response::DoDeserialize(jMsg);
	}

	bool InitializeResult::IsValid() const
	{
		if (!Response::IsValid())
			return false;

		if (strProtocolVersion.empty() || !implServerInfo.IsValid() || !capabilities.IsValid())
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ListToolsResult
	int ListToolsResult::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jResult(Json::objectValue);

		Json::Value jTools(Json::arrayValue);
		for (auto& tool : vecTools)
		{
			Json::Value jTool(Json::objectValue);
			if (ERRNO_OK == tool.DoSerialize(jTool))
			{
				jTools.append(jTool);
			}
		}
		jResult[MSG_KEY_TOOLS] = jTools;

		if (!strNextCursor.empty())
		{
			Json::Value jNextCursor(strNextCursor);
			jResult[MSG_KEY_NEXT_CURSOR] = jNextCursor;
		}

		jMsg[MSG_KEY_RESULT] = jResult;

		return Response::DoSerialize(jMsg);
	}

	int ListToolsResult::DoDeserialize(const Json::Value& jMsg)
	{
		return Response::DoDeserialize(jMsg);
	}

	bool ListToolsResult::IsValid() const
	{
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// CallToolResult
	int CallToolResult::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jResult(Json::objectValue);

		Json::Value jIsError(bIsError);
		jResult[MSG_KEY_IS_ERROR] = jIsError;

		Json::Value jContent(Json::arrayValue);
		for (auto& text : vecTextContent)
		{
			Json::Value jTextContent(Json::objectValue);
			if (ERRNO_OK == text.DoSerialize(jTextContent))
				jContent.append(jTextContent);
		}
		for (auto& image : vecImageContent)
		{
			Json::Value jImageContent(Json::objectValue);
			if (ERRNO_OK == image.DoSerialize(jImageContent))
				jContent.append(jImageContent);
		}
		for (auto& embedded : vecEmbeddedResource)
		{
			Json::Value jEmbedded(Json::objectValue);
			if (ERRNO_OK == embedded.DoSerialize(jEmbedded))
				jContent.append(jEmbedded);
		}
		jResult[MSG_KEY_CONTENT] = jContent;

		jMsg[MSG_KEY_RESULT] = jResult;

		return Response::DoSerialize(jMsg);
	}

	int CallToolResult::DoDeserialize(const Json::Value& jMsg)
	{
		return Response::DoDeserialize(jMsg);
	}

	bool CallToolResult::IsValid() const
	{
		if (vecTextContent.empty() && vecImageContent.empty() && vecEmbeddedResource.empty())
			return false;

		return true;
	}
}