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

	bool Response::IsCacheableResult() const
	{
		return false;
	}

	void Response::WriteVersionResultFields(Json::Value& jResult, const ProtocolContext& ctxProtocol, const Json::Value& jServerInfo) const
	{
		if (ctxProtocol.Supports(Feature_ResultType))
		{
			std::string strType = strResultType.empty() ? std::string(RESULT_TYPE_COMPLETE) : strResultType;
			jResult[MSG_KEY_RESULT_TYPE] = strType;
		}

		if (ctxProtocol.Supports(Feature_Mrtr))
		{
			if (jInputRequests.isObject() && !jInputRequests.empty())
				jResult[MSG_KEY_INPUT_REQUESTS] = jInputRequests;

			if (!strRequestState.empty())
				jResult[MSG_KEY_REQUEST_STATE] = strRequestState;
		}

		// Cache hints are only defined for complete results of read methods and
		// must not be reused by an MRTR retry round trip.
		if (ctxProtocol.Supports(Feature_Caching)
			&& IsCacheableResult()
			&& (strResultType.empty() || strResultType == RESULT_TYPE_COMPLETE)
			&& !bMrtrRetry)
		{
			if (iTtlMs >= 0)
			{
				Json::Value jTtlMs(iTtlMs);
				jResult[MSG_KEY_TTL_MS] = jTtlMs;
			}

			if (!strCacheScope.empty())
				jResult[MSG_KEY_CACHE_SCOPE] = strCacheScope;
		}

		Json::Value jMeta(Json::objectValue);
		if (jResultMeta.isObject())
			jMeta = jResultMeta;

		// Merge with whatever reserved keys the result already produced and let
		// the SDK owned serverInfo win over application supplied values.
		if (jResult.isMember(MSG_KEY_META) && jResult[MSG_KEY_META].isObject())
		{
			const Json::Value& jExistingMeta = jResult[MSG_KEY_META];
			for (auto it = jExistingMeta.begin(); it != jExistingMeta.end(); ++it)
			{
				bool bReserved = it.name() == META_KEY_SERVER_INFO || it.name() == META_KEY_SUBSCRIPTION_ID;
				if (bReserved || !jMeta.isMember(it.name()))
					jMeta[it.name()] = *it;
			}
		}

		if (ctxProtocol.Supports(Feature_ResultMeta) && jServerInfo.isObject() && !jServerInfo.empty())
			jMeta[META_KEY_SERVER_INFO] = jServerInfo;

		if (!jMeta.empty())
			jResult[MSG_KEY_META] = jMeta;
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
		if (!jErrorData.isNull())
		{
			jError[MSG_KEY_DATA] = jErrorData;
		}

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

		capabilities.DoSerializeVersionExtras(jResult[MSG_KEY_CAPABILITIES], ctxProtocol);
		implServerInfo.DoSerializeVersionExtras(jResult[MSG_KEY_SERVER_INFO], ctxProtocol);

		WriteVersionResultFields(jResult, ctxProtocol, jServerInfo);

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
				tool.DoSerializeVersionExtras(jTool, ctxProtocol);
				jTools.append(jTool);
			}
		}
		jResult[MSG_KEY_TOOLS] = jTools;

		if (!strNextCursor.empty())
		{
			Json::Value jNextCursor(strNextCursor);
			jResult[MSG_KEY_NEXT_CURSOR] = jNextCursor;
		}

		WriteVersionResultFields(jResult, ctxProtocol, jServerInfo);

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

	bool ListToolsResult::IsCacheableResult() const
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
			{
				text.DoSerializeVersionExtras(jTextContent, ctxProtocol);
				jContent.append(jTextContent);
			}
		}
		for (auto& image : vecImageContent)
		{
			Json::Value jImageContent(Json::objectValue);
			if (ERRNO_OK == image.DoSerialize(jImageContent))
			{
				image.DoSerializeVersionExtras(jImageContent, ctxProtocol);
				jContent.append(jImageContent);
			}
		}
		for (auto& embedded : vecEmbeddedResource)
		{
			Json::Value jEmbedded(Json::objectValue);
			if (ERRNO_OK == embedded.DoSerialize(jEmbedded))
			{
				embedded.DoSerializeVersionExtras(jEmbedded, ctxProtocol);
				jContent.append(jEmbedded);
			}
		}
		jResult[MSG_KEY_CONTENT] = jContent;

		if (ctxProtocol.Supports(Feature_StructuredContent) && !jStructuredContent.isNull())
			jResult[MSG_KEY_STRUCTURED_CONTENT] = jStructuredContent;

		WriteVersionResultFields(jResult, ctxProtocol, jServerInfo);

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

	////////////////////////////////////////////////////////////////////////////////////////
	// ListResourcesResult
	int ListResourcesResult::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jResult(Json::objectValue);

		Json::Value jResources(Json::arrayValue);
		for (auto& resource : vecResources)
		{
			Json::Value jResource(Json::objectValue);
			if (ERRNO_OK == resource.DoSerialize(jResource))
			{
				resource.DoSerializeVersionExtras(jResource, ctxProtocol);
				jResources.append(jResource);
			}
		}
		jResult[MSG_KEY_RESOURCES] = jResources;

		if (!strNextCursor.empty())
		{
			Json::Value jNextCursor(strNextCursor);
			jResult[MSG_KEY_NEXT_CURSOR] = jNextCursor;
		}

		WriteVersionResultFields(jResult, ctxProtocol, jServerInfo);

		jMsg[MSG_KEY_RESULT] = jResult;

		return Response::DoSerialize(jMsg);
	}

	int ListResourcesResult::DoDeserialize(const Json::Value& jMsg)
	{
		return Response::DoDeserialize(jMsg);
	}

	bool ListResourcesResult::IsValid() const
	{
		return true;
	}

	bool ListResourcesResult::IsCacheableResult() const
	{
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ReadResourceResult
	int ReadResourceResult::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jResult(Json::objectValue);

		Json::Value jContents(Json::arrayValue);
		for (auto& text : vecTextResourceContents)
		{
			Json::Value jTextContent(Json::objectValue);
			if (ERRNO_OK == text.DoSerialize(jTextContent))
			{
				text.DoSerializeVersionExtras(jTextContent, ctxProtocol);
				jContents.append(jTextContent);
			}
		}
		for (auto& blob : vecBlobResourceContents)
		{
			Json::Value jBlobContent(Json::objectValue);
			if (ERRNO_OK == blob.DoSerialize(jBlobContent))
			{
				blob.DoSerializeVersionExtras(jBlobContent, ctxProtocol);
				jContents.append(jBlobContent);
			}
		}
		jResult[MSG_KEY_CONTENTS] = jContents;

		WriteVersionResultFields(jResult, ctxProtocol, jServerInfo);

		jMsg[MSG_KEY_RESULT] = jResult;

		return Response::DoSerialize(jMsg);
	}

	int ReadResourceResult::DoDeserialize(const Json::Value& jMsg)
	{
		return Response::DoDeserialize(jMsg);
	}

	bool ReadResourceResult::IsValid() const
	{
		return true;
	}

	bool ReadResourceResult::IsCacheableResult() const
	{
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ListResourceTemplatesResult
	int ListResourceTemplatesResult::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jResult(Json::objectValue);

		Json::Value jResourceTemplates(Json::arrayValue);
		for (auto& template_ : vecResourceTemplates)
		{
			Json::Value jTemplate(Json::objectValue);
			if (ERRNO_OK == template_.DoSerialize(jTemplate))
			{
				template_.DoSerializeVersionExtras(jTemplate, ctxProtocol);
				jResourceTemplates.append(jTemplate);
			}
		}
		jResult[MSG_KEY_RESOURCE_TEMPLATES] = jResourceTemplates;

		if (!strNextCursor.empty())
		{
			Json::Value jNextCursor(strNextCursor);
			jResult[MSG_KEY_NEXT_CURSOR] = jNextCursor;
		}

		WriteVersionResultFields(jResult, ctxProtocol, jServerInfo);

		jMsg[MSG_KEY_RESULT] = jResult;

		return Response::DoSerialize(jMsg);
	}

	int ListResourceTemplatesResult::DoDeserialize(const Json::Value& jMsg)
	{
		return Response::DoDeserialize(jMsg);
	}

	bool ListResourceTemplatesResult::IsValid() const
	{
		return true;
	}

	bool ListResourceTemplatesResult::IsCacheableResult() const
	{
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ListPromptsResult
	int ListPromptsResult::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jResult(Json::objectValue);

		Json::Value jPrompts(Json::arrayValue);
		for (const auto& prompt : vecPrompts)
		{
			Json::Value jPrompt(Json::objectValue);
			if (ERRNO_OK == prompt.DoSerialize(jPrompt))
			{
				prompt.DoSerializeVersionExtras(jPrompt, ctxProtocol);
				jPrompts.append(jPrompt);
			}
		}
		jResult[MSG_KEY_PROMPTS] = jPrompts;

		if (!strNextCursor.empty())
		{
			Json::Value jNextCursor(strNextCursor);
			jResult[MSG_KEY_NEXT_CURSOR] = jNextCursor;
		}

		WriteVersionResultFields(jResult, ctxProtocol, jServerInfo);

		jMsg[MSG_KEY_RESULT] = jResult;

		return Response::DoSerialize(jMsg);
	}

	int ListPromptsResult::DoDeserialize(const Json::Value& jMsg)
	{
		return Response::DoDeserialize(jMsg);
	}

	bool ListPromptsResult::IsValid() const
	{
		return true;
	}

	bool ListPromptsResult::IsCacheableResult() const
	{
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// GetPromptResult
	int GetPromptResult::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jResult(Json::objectValue);

		if (!strDescription.empty())
		{
			Json::Value jDesc(strDescription);
			jResult[MSG_KEY_DESCRIPTION] = jDesc;
		}

		Json::Value jMessages(Json::arrayValue);
		for (const auto& msg : vecMessages)
		{
			Json::Value jMessage(Json::objectValue);
			if (ERRNO_OK == msg.DoSerialize(jMessage))
			{
				msg.DoSerializeVersionExtras(jMessage, ctxProtocol);
				jMessages.append(jMessage);
			}
		}
		jResult[MSG_KEY_MESSAGES] = jMessages;

		WriteVersionResultFields(jResult, ctxProtocol, jServerInfo);

		jMsg[MSG_KEY_RESULT] = jResult;

		return Response::DoSerialize(jMsg);
	}

	int GetPromptResult::DoDeserialize(const Json::Value& jMsg)
	{
		return Response::DoDeserialize(jMsg);
	}

	bool GetPromptResult::IsValid() const
	{
		return !vecMessages.empty();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// CompleteResult
	int CompleteResult::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jResult(Json::objectValue);

		Json::Value jCompletion(Json::objectValue);

		Json::Value jValues(Json::arrayValue);
		for (const auto& strValue : vecValues)
		{
			Json::Value jVal(strValue);
			jValues.append(jVal);
		}
		jCompletion[MSG_KEY_VALUES] = jValues;

		if (iTotal >= 0)
		{
			Json::Value jTotal(iTotal);
			jCompletion[MSG_KEY_TOTAL] = jTotal;
		}

		Json::Value jHasMore(bHasMore);
		jCompletion[MSG_KEY_HAS_MORE] = jHasMore;

		jResult[MSG_KEY_COMPLETION] = jCompletion;

		WriteVersionResultFields(jResult, ctxProtocol, jServerInfo);

		jMsg[MSG_KEY_RESULT] = jResult;

		return Response::DoSerialize(jMsg);
	}

	int CompleteResult::DoDeserialize(const Json::Value& jMsg)
	{
		return Response::DoDeserialize(jMsg);
	}

	bool CompleteResult::IsValid() const
	{
		return !vecValues.empty();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// DiscoverResult
	int DiscoverResult::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jResult(Json::objectValue);

		Json::Value jVersions(Json::arrayValue);
		for (const auto& strVersion : vecSupportedVersions)
			jVersions.append(strVersion);
		jResult[MSG_KEY_SUPPORTED_VERSIONS] = jVersions;

		Json::Value jCapabilities(Json::objectValue);
		if (ERRNO_OK == capabilities.DoSerialize(jCapabilities))
		{
			capabilities.DoSerializeVersionExtras(jCapabilities, ctxProtocol);
			jResult[MSG_KEY_CAPABILITIES] = jCapabilities;
		}

		if (!strInstructions.empty())
		{
			Json::Value jInstructions(strInstructions);
			jResult[MSG_KEY_INSTRUCTIONS] = jInstructions;
		}

		// discover always publishes its cache hint once the feature is enabled.
		if (ctxProtocol.Supports(Feature_Discover))
		{
			if (iTtlMs >= 0)
			{
				Json::Value jTtlMs(iTtlMs);
				jResult[MSG_KEY_TTL_MS] = jTtlMs;
			}

			if (!strCacheScope.empty())
				jResult[MSG_KEY_CACHE_SCOPE] = strCacheScope;
		}

		WriteVersionResultFields(jResult, ctxProtocol, jServerInfo);

		jMsg[MSG_KEY_RESULT] = jResult;

		return Response::DoSerialize(jMsg);
	}

	int DiscoverResult::DoDeserialize(const Json::Value& jMsg)
	{
		return Response::DoDeserialize(jMsg);
	}

	bool DiscoverResult::IsValid() const
	{
		return !vecSupportedVersions.empty();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// SubscriptionsListenResult
	int SubscriptionsListenResult::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jResult(Json::objectValue);

		if (ctxProtocol.Supports(Feature_ListenSubscriptions) && !jSubscriptionId.isNull())
		{
			Json::Value jMeta(Json::objectValue);
			jMeta[META_KEY_SUBSCRIPTION_ID] = jSubscriptionId;
			jResult[MSG_KEY_META] = jMeta;
		}

		WriteVersionResultFields(jResult, ctxProtocol, jServerInfo);

		jMsg[MSG_KEY_RESULT] = jResult;

		return Response::DoSerialize(jMsg);
	}

	int SubscriptionsListenResult::DoDeserialize(const Json::Value& jMsg)
	{
		return Response::DoDeserialize(jMsg);
	}

	bool SubscriptionsListenResult::IsValid() const
	{
		if (!Response::IsValid())
			return false;

		return true;
	}
}