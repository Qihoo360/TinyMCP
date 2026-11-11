#include "Response.h"
#include "../Internal/Protocol.h"
#include "../Internal/VersionExtras.h"
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

	void Response::WriteVersionResultFields(Json::Value& jResult, const ProtocolContext& ctxProtocol, const Json::Value& jServerInfo, bool bForceCacheHints) const
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
		if ((ctxProtocol.Supports(Feature_Caching) || bForceCacheHints)
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
		if (ctxProtocol.Supports(Feature_ResultMeta) && jResultMeta.isObject())
		{
			// Reserved keys are SDK owned: drop application supplied values so the
			// existing result keys and the SDK serverInfo always win.
			for (auto it = jResultMeta.begin(); it != jResultMeta.end(); ++it)
			{
				if (it.name() == META_KEY_SERVER_INFO || it.name() == META_KEY_SUBSCRIPTION_ID)
					continue;

				jMeta[it.name()] = *it;
			}
		}

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
		const ProtocolContext& ctxProtocol = GetProtocolContext();

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

		VersionExtrasAccess::Serialize(capabilities, jResult[MSG_KEY_CAPABILITIES], ctxProtocol);
		VersionExtrasAccess::Serialize(implServerInfo, jResult[MSG_KEY_SERVER_INFO], ctxProtocol);

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
		const ProtocolContext& ctxProtocol = GetProtocolContext();

		Json::Value jResult(Json::objectValue);

		Json::Value jTools(Json::arrayValue);
		for (auto& tool : vecTools)
		{
			Json::Value jTool(Json::objectValue);
			if (ERRNO_OK == tool.DoSerialize(jTool))
			{
				VersionExtrasAccess::Serialize(tool, jTool, ctxProtocol);
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
		const ProtocolContext& ctxProtocol = GetProtocolContext();

		Json::Value jResult(Json::objectValue);

		Json::Value jIsError(bIsError);
		jResult[MSG_KEY_IS_ERROR] = jIsError;

		Json::Value jContent(Json::arrayValue);
		for (auto& text : vecTextContent)
		{
			Json::Value jTextContent(Json::objectValue);
			if (ERRNO_OK == text.DoSerialize(jTextContent))
			{
				VersionExtrasAccess::Serialize(text, jTextContent, ctxProtocol);
				jContent.append(jTextContent);
			}
		}
		for (auto& image : vecImageContent)
		{
			Json::Value jImageContent(Json::objectValue);
			if (ERRNO_OK == image.DoSerialize(jImageContent))
			{
				VersionExtrasAccess::Serialize(image, jImageContent, ctxProtocol);
				jContent.append(jImageContent);
			}
		}
		if (ctxProtocol.Supports(Feature_ContentAudio))
		{
			for (auto& audio : vecAudioContent)
			{
				Json::Value jAudioContent(Json::objectValue);
				if (ERRNO_OK == audio.DoSerialize(jAudioContent))
				{
					VersionExtrasAccess::Serialize(audio, jAudioContent, ctxProtocol);
					jContent.append(jAudioContent);
				}
			}
		}
		if (ctxProtocol.Supports(Feature_ContentResourceLink))
		{
			for (auto& link : vecResourceLinkContent)
			{
				Json::Value jResourceLink(Json::objectValue);
				if (ERRNO_OK == link.DoSerialize(jResourceLink))
				{
					VersionExtrasAccess::Serialize(link, jResourceLink, ctxProtocol);
					jContent.append(jResourceLink);
				}
			}
		}
		for (auto& embedded : vecEmbeddedResource)
		{
			Json::Value jEmbedded(Json::objectValue);
			if (ERRNO_OK == embedded.DoSerialize(jEmbedded))
			{
				VersionExtrasAccess::Serialize(embedded, jEmbedded, ctxProtocol);
				jContent.append(jEmbedded);
			}
		}
		if (ctxProtocol.Supports(Feature_ContentToolUse))
		{
			for (auto& toolUse : vecToolUseContent)
			{
				Json::Value jToolUse(Json::objectValue);
				if (ERRNO_OK == toolUse.DoSerialize(jToolUse))
				{
					VersionExtrasAccess::Serialize(toolUse, jToolUse, ctxProtocol);
					jContent.append(jToolUse);
				}
			}
			for (auto& toolResult : vecToolResultContent)
			{
				Json::Value jToolResult(Json::objectValue);
				if (ERRNO_OK == toolResult.DoSerialize(jToolResult))
				{
					VersionExtrasAccess::Serialize(toolResult, jToolResult, ctxProtocol);
					jContent.append(jToolResult);
				}
			}
		}
		jResult[MSG_KEY_CONTENT] = jContent;

		if (ctxProtocol.Supports(Feature_StructuredContent) && structuredContent.HasValue())
		{
			Json::Value jStructuredContent;
			if (ERRNO_OK == structuredContent.DoSerialize(jStructuredContent))
				jResult[MSG_KEY_STRUCTURED_CONTENT] = jStructuredContent;
		}

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
		// An MRTR round trip carries no content until the input is supplied.
		if (RESULT_TYPE_INPUT_REQUIRED == strResultType)
			return requestId.IsValid();

		if (vecTextContent.empty()
			&& vecImageContent.empty()
			&& vecAudioContent.empty()
			&& vecResourceLinkContent.empty()
			&& vecEmbeddedResource.empty()
			&& vecToolUseContent.empty()
			&& vecToolResultContent.empty())
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ListResourcesResult
	int ListResourcesResult::DoSerialize(Json::Value& jMsg) const
	{
		const ProtocolContext& ctxProtocol = GetProtocolContext();

		Json::Value jResult(Json::objectValue);

		Json::Value jResources(Json::arrayValue);
		for (auto& resource : vecResources)
		{
			Json::Value jResource(Json::objectValue);
			if (ERRNO_OK == resource.DoSerialize(jResource))
			{
				VersionExtrasAccess::Serialize(resource, jResource, ctxProtocol);
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
		const ProtocolContext& ctxProtocol = GetProtocolContext();

		Json::Value jResult(Json::objectValue);

		Json::Value jContents(Json::arrayValue);
		for (auto& text : vecTextResourceContents)
		{
			Json::Value jTextContent(Json::objectValue);
			if (ERRNO_OK == text.DoSerialize(jTextContent))
			{
				VersionExtrasAccess::Serialize(text, jTextContent, ctxProtocol);
				jContents.append(jTextContent);
			}
		}
		for (auto& blob : vecBlobResourceContents)
		{
			Json::Value jBlobContent(Json::objectValue);
			if (ERRNO_OK == blob.DoSerialize(jBlobContent))
			{
				VersionExtrasAccess::Serialize(blob, jBlobContent, ctxProtocol);
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
		const ProtocolContext& ctxProtocol = GetProtocolContext();

		Json::Value jResult(Json::objectValue);

		Json::Value jResourceTemplates(Json::arrayValue);
		for (auto& template_ : vecResourceTemplates)
		{
			Json::Value jTemplate(Json::objectValue);
			if (ERRNO_OK == template_.DoSerialize(jTemplate))
			{
				VersionExtrasAccess::Serialize(template_, jTemplate, ctxProtocol);
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
		const ProtocolContext& ctxProtocol = GetProtocolContext();

		Json::Value jResult(Json::objectValue);

		Json::Value jPrompts(Json::arrayValue);
		for (const auto& prompt : vecPrompts)
		{
			Json::Value jPrompt(Json::objectValue);
			if (ERRNO_OK == prompt.DoSerialize(jPrompt))
			{
				VersionExtrasAccess::Serialize(prompt, jPrompt, ctxProtocol);
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
		const ProtocolContext& ctxProtocol = GetProtocolContext();

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
				VersionExtrasAccess::Serialize(msg, jMessage, ctxProtocol);
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
		// An MRTR round trip carries no messages until the input is supplied.
		if (RESULT_TYPE_INPUT_REQUIRED == strResultType)
			return requestId.IsValid();

		return !vecMessages.empty();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// CompleteResult
	int CompleteResult::DoSerialize(Json::Value& jMsg) const
	{
		const ProtocolContext& ctxProtocol = GetProtocolContext();

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
		const ProtocolContext& ctxProtocol = GetProtocolContext();

		Json::Value jResult(Json::objectValue);

		Json::Value jVersions(Json::arrayValue);
		for (const auto& strVersion : vecSupportedVersions)
			jVersions.append(strVersion);
		jResult[MSG_KEY_SUPPORTED_VERSIONS] = jVersions;

		Json::Value jCapabilities(Json::objectValue);
		if (ERRNO_OK == capabilities.DoSerialize(jCapabilities))
		{
			VersionExtrasAccess::Serialize(capabilities, jCapabilities, ctxProtocol);

			// The stateless era replaces legacy-only capability slots: completions
			// are advertised on their own key and resource subscriptions move to
			// subscriptions/listen, so discover must not publish either legacy slot.
			if (ctxProtocol.Supports(Feature_Discover))
				jCapabilities.removeMember(MSG_KEY_EXPERIMENTAL);

			if (ctxProtocol.Supports(Feature_ListenSubscriptions)
				&& jCapabilities.isMember(MSG_KEY_RESOURCES)
				&& jCapabilities[MSG_KEY_RESOURCES].isObject())
			{
				jCapabilities[MSG_KEY_RESOURCES].removeMember(MSG_KEY_SUBSCRIBE);
			}

			jResult[MSG_KEY_CAPABILITIES] = jCapabilities;
		}

		if (!strInstructions.empty())
		{
			Json::Value jInstructions(strInstructions);
			jResult[MSG_KEY_INSTRUCTIONS] = jInstructions;
		}

		// Discover itself requires cache hints, independently of the general
		// caching feature: the schema makes ttlMs/cacheScope mandatory here.
		WriteVersionResultFields(jResult, ctxProtocol, jServerInfo, ctxProtocol.Supports(Feature_Discover));

		jMsg[MSG_KEY_RESULT] = jResult;

		return Response::DoSerialize(jMsg);
	}

	int DiscoverResult::DoDeserialize(const Json::Value& jMsg)
	{
		return Response::DoDeserialize(jMsg);
	}

	bool DiscoverResult::IsCacheableResult() const
	{
		return true;
	}

	bool DiscoverResult::IsValid() const
	{
		return !vecSupportedVersions.empty();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// SubscriptionsListenResult
	int SubscriptionsListenResult::DoSerialize(Json::Value& jMsg) const
	{
		const ProtocolContext& ctxProtocol = GetProtocolContext();

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