#include "BasicMessage.h"
#include "../Internal/Protocol.h"
#include "../Internal/VersionExtras.h"
#include <json/json.h>

namespace MCP
{
	const ProtocolContext& Message::GetProtocolContext() const
	{
		static const ProtocolContext s_DefaultContext;
		return m_spProtocolContext ? *m_spProtocolContext : s_DefaultContext;
	}

	void Message::SetProtocolContext(const ProtocolContext& ctxProtocol)
	{
		m_spProtocolContext = std::make_shared<const ProtocolContext>(ctxProtocol);
	}

	namespace
	{
		void WriteStringIfNotEmpty(Json::Value& jObj, const char* lpcszKey, const std::string& strValue)
		{
			if (strValue.empty())
				return;
			jObj[lpcszKey] = strValue;
		}

		void WriteIconsIfNotEmpty(Json::Value& jObj, const std::vector<Icon>& vecIcons)
		{
			if (vecIcons.empty())
				return;

			Json::Value jIcons(Json::arrayValue);
			for (const auto& icon : vecIcons)
			{
				Json::Value jIcon(Json::objectValue);
				if (ERRNO_OK == icon.DoSerialize(jIcon))
					jIcons.append(jIcon);
			}

			if (!jIcons.empty())
				jObj[MSG_KEY_ICONS] = jIcons;
		}

		void ReadIconsIfPresent(const Json::Value& jObj, std::vector<Icon>& vecIcons)
		{
			if (!jObj.isMember(MSG_KEY_ICONS) || !jObj[MSG_KEY_ICONS].isArray())
				return;

			vecIcons.clear();
			for (const auto& jIcon : jObj[MSG_KEY_ICONS])
			{
				Icon icon;
				if (ERRNO_OK == icon.DoDeserialize(jIcon))
					vecIcons.push_back(icon);
			}
		}

		void WriteAnnotationsIfPresent(Json::Value& jObj, const Annotations& annotations)
		{
			if (!annotations.HasValue())
				return;

			Json::Value jAnnotations(Json::objectValue);
			if (ERRNO_OK == annotations.DoSerialize(jAnnotations))
				jObj[MSG_KEY_ANNOTATIONS] = jAnnotations;
		}

		void WriteMetaIfPresent(Json::Value& jObj, const Json::Value& jMeta)
		{
			if (jMeta.isObject() && !jMeta.empty())
				jObj[MSG_KEY_META] = jMeta;
		}

		// Shared gate order for entity level extras (title/icons/annotations/_meta).
		void WriteEntityVersionExtras(Json::Value& jObj, const ProtocolContext& ctxProtocol,
			const std::string& strTitle, const std::vector<Icon>& vecIcons,
			const Json::Value& jMeta, const Annotations* pAnnotations,
			ProtocolFeature eAnnotationFeature)
		{
			if (ctxProtocol.Supports(Feature_ToolTitle))
			{
				WriteStringIfNotEmpty(jObj, MSG_KEY_TITLE, strTitle);
				WriteIconsIfNotEmpty(jObj, vecIcons);
			}

			if (nullptr != pAnnotations && ctxProtocol.Supports(eAnnotationFeature))
				WriteAnnotationsIfPresent(jObj, *pAnnotations);

			if (ctxProtocol.Supports(Feature_EntityMeta))
				WriteMetaIfPresent(jObj, jMeta);
		}

		void ReadEntityVersionExtras(const Json::Value& jObj, const ProtocolContext& ctxProtocol,
			std::string& strTitle, std::vector<Icon>& vecIcons,
			Json::Value& jMeta, Annotations* pAnnotations,
			ProtocolFeature eAnnotationFeature)
		{
			if (ctxProtocol.Supports(Feature_ToolTitle))
			{
				if (jObj.isMember(MSG_KEY_TITLE) && jObj[MSG_KEY_TITLE].isString())
					strTitle = jObj[MSG_KEY_TITLE].asString();
				ReadIconsIfPresent(jObj, vecIcons);
			}

			if (nullptr != pAnnotations && ctxProtocol.Supports(eAnnotationFeature))
			{
				if (jObj.isMember(MSG_KEY_ANNOTATIONS) && jObj[MSG_KEY_ANNOTATIONS].isObject())
					pAnnotations->DoDeserialize(jObj[MSG_KEY_ANNOTATIONS]);
			}

			if (ctxProtocol.Supports(Feature_EntityMeta))
			{
				if (jObj.isMember(MSG_KEY_META) && jObj[MSG_KEY_META].isObject())
					jMeta = jObj[MSG_KEY_META];
			}
		}

		void WriteContentVersionExtras(Json::Value& jObj, const ProtocolContext& ctxProtocol,
			const Json::Value& jMeta, const Annotations& annotations)
		{
			if (ctxProtocol.Supports(Feature_ContentAnnotations))
				WriteAnnotationsIfPresent(jObj, annotations);

			if (ctxProtocol.Supports(Feature_EntityMeta))
				WriteMetaIfPresent(jObj, jMeta);
		}

		void ReadContentVersionExtras(const Json::Value& jObj, const ProtocolContext& ctxProtocol,
			Json::Value& jMeta, Annotations& annotations)
		{
			if (ctxProtocol.Supports(Feature_ContentAnnotations)
				&& jObj.isMember(MSG_KEY_ANNOTATIONS) && jObj[MSG_KEY_ANNOTATIONS].isObject())
			{
				annotations.DoDeserialize(jObj[MSG_KEY_ANNOTATIONS]);
			}

			if (ctxProtocol.Supports(Feature_EntityMeta)
				&& jObj.isMember(MSG_KEY_META) && jObj[MSG_KEY_META].isObject())
			{
				jMeta = jObj[MSG_KEY_META];
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Icon
	int Icon::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jSrc(strSrc);
		jMsg[MSG_KEY_SRC] = jSrc;

		WriteStringIfNotEmpty(jMsg, MSG_KEY_MIMETYPE, strMimeType);

		if (!vecSizes.empty())
		{
			Json::Value jSizes(Json::arrayValue);
			for (const auto& strSize : vecSizes)
				jSizes.append(strSize);
			jMsg[MSG_KEY_SIZES] = jSizes;
		}

		WriteStringIfNotEmpty(jMsg, MSG_KEY_THEME, strTheme);

		return ERRNO_OK;
	}

	int Icon::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isObject() || !jMsg.isMember(MSG_KEY_SRC) || !jMsg[MSG_KEY_SRC].isString())
			return ERRNO_PARSE_ERROR;
		strSrc = jMsg[MSG_KEY_SRC].asString();

		if (jMsg.isMember(MSG_KEY_MIMETYPE) && jMsg[MSG_KEY_MIMETYPE].isString())
			strMimeType = jMsg[MSG_KEY_MIMETYPE].asString();

		if (jMsg.isMember(MSG_KEY_SIZES) && jMsg[MSG_KEY_SIZES].isArray())
		{
			vecSizes.clear();
			for (const auto& jSize : jMsg[MSG_KEY_SIZES])
			{
				if (jSize.isString())
					vecSizes.push_back(jSize.asString());
			}
		}

		if (jMsg.isMember(MSG_KEY_THEME) && jMsg[MSG_KEY_THEME].isString())
			strTheme = jMsg[MSG_KEY_THEME].asString();

		return ERRNO_OK;
	}

	bool Icon::IsValid() const
	{
		return !strSrc.empty();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Annotations
	bool Annotations::HasValue() const
	{
		return !vecAudience.empty() || bHasPriority || !strLastModified.empty();
	}

	int Annotations::DoSerialize(Json::Value& jMsg) const
	{
		if (!vecAudience.empty())
		{
			Json::Value jAudience(Json::arrayValue);
			for (const auto& strAudience : vecAudience)
				jAudience.append(strAudience);
			jMsg[MSG_KEY_AUDIENCE] = jAudience;
		}

		if (bHasPriority)
		{
			Json::Value jPriority(dPriority);
			jMsg[MSG_KEY_PRIORITY] = jPriority;
		}

		WriteStringIfNotEmpty(jMsg, MSG_KEY_LAST_MODIFIED, strLastModified);

		return ERRNO_OK;
	}

	int Annotations::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isObject())
			return ERRNO_PARSE_ERROR;

		if (jMsg.isMember(MSG_KEY_AUDIENCE) && jMsg[MSG_KEY_AUDIENCE].isArray())
		{
			vecAudience.clear();
			for (const auto& jAudience : jMsg[MSG_KEY_AUDIENCE])
			{
				if (jAudience.isString())
					vecAudience.push_back(jAudience.asString());
			}
		}

		if (jMsg.isMember(MSG_KEY_PRIORITY) && jMsg[MSG_KEY_PRIORITY].isNumeric())
		{
			bHasPriority = true;
			dPriority = jMsg[MSG_KEY_PRIORITY].asDouble();
		}

		if (jMsg.isMember(MSG_KEY_LAST_MODIFIED) && jMsg[MSG_KEY_LAST_MODIFIED].isString())
			strLastModified = jMsg[MSG_KEY_LAST_MODIFIED].asString();

		return ERRNO_OK;
	}

	bool Annotations::IsValid() const
	{
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ToolAnnotations
	bool ToolAnnotations::HasValue() const
	{
		return !strTitle.empty()
			|| bHasReadOnlyHint
			|| bHasDestructiveHint
			|| bHasIdempotentHint
			|| bHasOpenWorldHint;
	}

	int ToolAnnotations::DoSerialize(Json::Value& jMsg) const
	{
		WriteStringIfNotEmpty(jMsg, MSG_KEY_TITLE, strTitle);

		if (bHasReadOnlyHint)
		{
			Json::Value jHint(bReadOnlyHint);
			jMsg[MSG_KEY_READ_ONLY_HINT] = jHint;
		}

		if (bHasDestructiveHint)
		{
			Json::Value jHint(bDestructiveHint);
			jMsg[MSG_KEY_DESTRUCTIVE_HINT] = jHint;
		}

		if (bHasIdempotentHint)
		{
			Json::Value jHint(bIdempotentHint);
			jMsg[MSG_KEY_IDEMPOTENT_HINT] = jHint;
		}

		if (bHasOpenWorldHint)
		{
			Json::Value jHint(bOpenWorldHint);
			jMsg[MSG_KEY_OPEN_WORLD_HINT] = jHint;
		}

		return ERRNO_OK;
	}

	int ToolAnnotations::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isObject())
			return ERRNO_PARSE_ERROR;

		if (jMsg.isMember(MSG_KEY_TITLE) && jMsg[MSG_KEY_TITLE].isString())
			strTitle = jMsg[MSG_KEY_TITLE].asString();

		if (jMsg.isMember(MSG_KEY_READ_ONLY_HINT) && jMsg[MSG_KEY_READ_ONLY_HINT].isBool())
		{
			bHasReadOnlyHint = true;
			bReadOnlyHint = jMsg[MSG_KEY_READ_ONLY_HINT].asBool();
		}

		if (jMsg.isMember(MSG_KEY_DESTRUCTIVE_HINT) && jMsg[MSG_KEY_DESTRUCTIVE_HINT].isBool())
		{
			bHasDestructiveHint = true;
			bDestructiveHint = jMsg[MSG_KEY_DESTRUCTIVE_HINT].asBool();
		}

		if (jMsg.isMember(MSG_KEY_IDEMPOTENT_HINT) && jMsg[MSG_KEY_IDEMPOTENT_HINT].isBool())
		{
			bHasIdempotentHint = true;
			bIdempotentHint = jMsg[MSG_KEY_IDEMPOTENT_HINT].asBool();
		}

		if (jMsg.isMember(MSG_KEY_OPEN_WORLD_HINT) && jMsg[MSG_KEY_OPEN_WORLD_HINT].isBool())
		{
			bHasOpenWorldHint = true;
			bOpenWorldHint = jMsg[MSG_KEY_OPEN_WORLD_HINT].asBool();
		}

		return ERRNO_OK;
	}

	bool ToolAnnotations::IsValid() const
	{
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// RequestId
	int RequestId::DoSerialize(Json::Value & jMsg) const
	{
		std::string strMsgKey = MSG_KEY_ID;
		if (!_strMsgKey.empty())
			strMsgKey = _strMsgKey;

		if (DataType_String == eIdDataType)
		{
			Json::Value jId(strId);
			jMsg[strMsgKey] = jId;
		}
		else if (DataType_Integer == eIdDataType)
		{
			Json::Value jId(iId);
			jMsg[strMsgKey] = jId;
		}

		return ERRNO_OK;
	}

	int RequestId::DoDeserialize(const Json::Value& jMsg)
	{
		std::string strMsgKey = MSG_KEY_ID;
		if (!_strMsgKey.empty())
			strMsgKey = _strMsgKey;

		if (!jMsg.isMember(strMsgKey))
			return ERRNO_INVALID_REQUEST;
		bool bStrId = jMsg[strMsgKey].isString();
		bool bIntId = jMsg[strMsgKey].isIntegral();
		if (!bStrId && !bIntId)
			return ERRNO_INVALID_REQUEST;
		if (bStrId)
		{
			eIdDataType = DataType_String;
			strId = jMsg[strMsgKey].asString();
		}
		if (bIntId)
		{
			eIdDataType = DataType_Integer;
			iId = jMsg[strMsgKey].asInt();
		}

		return ERRNO_OK;
	}

	bool RequestId::IsValid() const
	{
		if (eIdDataType != DataType_String && eIdDataType != DataType_Integer)
			return false;

		if (DataType_String == eIdDataType && strId.empty())
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Implementation
	int Implementation::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jName(strName);
		jMsg[MSG_KEY_NAME] = jName;
		Json::Value jVersion(strVersion);
		jMsg[MSG_KEY_VERSION] = jVersion;

		return ERRNO_OK;
	}

	int Implementation::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_NAME) || !jMsg[MSG_KEY_NAME].isString())
			return ERRNO_PARSE_ERROR;
		strName = jMsg[MSG_KEY_NAME].asString();

		if (!jMsg.isMember(MSG_KEY_VERSION) || !jMsg[MSG_KEY_VERSION].isString())
			return ERRNO_PARSE_ERROR;
		strVersion = jMsg[MSG_KEY_VERSION].asString();

		return ERRNO_OK;
	}

	bool Implementation::IsValid() const
	{
		return true;
	}

	int Implementation::DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const
	{
		if (!ctxProtocol.Supports(Feature_ImplementationExtras))
			return ERRNO_OK;

		WriteStringIfNotEmpty(jMsg, MSG_KEY_TITLE, strTitle);
		WriteStringIfNotEmpty(jMsg, MSG_KEY_DESCRIPTION, strDescription);
		WriteIconsIfNotEmpty(jMsg, vecIcons);
		WriteStringIfNotEmpty(jMsg, MSG_KEY_WEBSITE_URL, strWebsiteUrl);

		return ERRNO_OK;
	}

	int Implementation::DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
	{
		if (!ctxProtocol.Supports(Feature_ImplementationExtras))
			return ERRNO_OK;

		if (jMsg.isMember(MSG_KEY_TITLE) && jMsg[MSG_KEY_TITLE].isString())
			strTitle = jMsg[MSG_KEY_TITLE].asString();
		if (jMsg.isMember(MSG_KEY_DESCRIPTION) && jMsg[MSG_KEY_DESCRIPTION].isString())
			strDescription = jMsg[MSG_KEY_DESCRIPTION].asString();
		ReadIconsIfPresent(jMsg, vecIcons);
		if (jMsg.isMember(MSG_KEY_WEBSITE_URL) && jMsg[MSG_KEY_WEBSITE_URL].isString())
			strWebsiteUrl = jMsg[MSG_KEY_WEBSITE_URL].asString();

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Prompts
	int Prompts::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jListChanged(bListChanged);
		jMsg[MSG_KEY_LISTCHANGED] = jListChanged;

		return ERRNO_OK;
	}

	int Prompts::DoDeserialize(const Json::Value& jMsg)
	{
		if (jMsg.isMember(MSG_KEY_LISTCHANGED) && jMsg[MSG_KEY_LISTCHANGED].isBool())
			bListChanged = jMsg[MSG_KEY_LISTCHANGED].asBool();

		return ERRNO_OK;
	}

	bool Prompts::IsValid() const
	{
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Resources
	int Resources::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jListChanged(bListChanged);
		jMsg[MSG_KEY_LISTCHANGED] = jListChanged;
		Json::Value jSubscribe(bSubscribe);
		jMsg[MSG_KEY_SUBSCRIBE] = jSubscribe;

		return ERRNO_OK;
	}

	int Resources::DoDeserialize(const Json::Value& jMsg)
	{
		if (jMsg.isMember(MSG_KEY_LISTCHANGED) && jMsg[MSG_KEY_LISTCHANGED].isBool())
			bListChanged = jMsg[MSG_KEY_LISTCHANGED].asBool();
		if (jMsg.isMember(MSG_KEY_SUBSCRIBE) && jMsg[MSG_KEY_SUBSCRIBE].isBool())
			bSubscribe = jMsg[MSG_KEY_SUBSCRIBE].asBool();

		return ERRNO_OK;
	}

	bool Resources::IsValid() const
	{
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Tools
	int Tools::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jListChanged(bListChanged);
		jMsg[MSG_KEY_LISTCHANGED] = jListChanged;

		return ERRNO_OK;
	}

	int Tools::DoDeserialize(const Json::Value& jMsg)
	{
		if (jMsg.isMember(MSG_KEY_LISTCHANGED) && jMsg[MSG_KEY_LISTCHANGED].isBool())
			bListChanged = jMsg[MSG_KEY_LISTCHANGED].asBool();

		return ERRNO_OK;
	}

	bool Tools::IsValid() const
	{
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Experimental
	int Experimental::DoSerialize(Json::Value& jMsg) const
	{
		if (bCompletion)
		{
			Json::Value jCompletion(Json::objectValue);
			jMsg[MSG_KEY_COMPLETION] = jCompletion;
		}

		return ERRNO_OK;
	}

	int Experimental::DoDeserialize(const Json::Value& jMsg)
	{
		return ERRNO_OK;
	}

	bool Experimental::IsValid() const
	{
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ServerCapabilities
	int ServerCapabilities::DoSerialize(Json::Value& jMsg) const
	{
		auto fnSerializeMember = [&jMsg](const auto& msgObj, const char* lpcszKey)
		{
			if (msgObj.bExist)
			{
				Json::Value jMember(Json::objectValue);
				if (ERRNO_OK == msgObj.DoSerialize(jMember))
				{
					jMsg[lpcszKey] = jMember;
				}
			}
		};

		fnSerializeMember(prompts, MSG_KEY_PROMPTS);
		fnSerializeMember(resources, MSG_KEY_RESOURCES);
		fnSerializeMember(tools, MSG_KEY_TOOLS);
		fnSerializeMember(experimental, MSG_KEY_EXPERIMENTAL);
		fnSerializeMember(logging, MSG_KEY_LOGGING);

		return ERRNO_OK;
	}

	int ServerCapabilities::DoDeserialize(const Json::Value& jMsg)
	{
		auto fnDeserializeMember = [&jMsg](auto& msgObj, const char* lpcszKey)
		{
			if (jMsg.isMember(lpcszKey) && jMsg[lpcszKey].isObject())
			{
				if (ERRNO_OK == msgObj.DoDeserialize(jMsg[lpcszKey]))
				{
					msgObj.bExist = true;
				}
			}
		};
		fnDeserializeMember(prompts, MSG_KEY_PROMPTS);
		fnDeserializeMember(resources, MSG_KEY_RESOURCES);
		fnDeserializeMember(tools, MSG_KEY_TOOLS);
		fnDeserializeMember(experimental, MSG_KEY_EXPERIMENTAL);
		fnDeserializeMember(logging, MSG_KEY_LOGGING);

		if (jMsg.isMember(MSG_KEY_COMPLETIONS))
			bCompletions = true;

		if (jMsg.isMember(MSG_KEY_EXTENSIONS) && jMsg[MSG_KEY_EXTENSIONS].isObject())
		{
			mapExtensions.clear();
			for (auto it = jMsg[MSG_KEY_EXTENSIONS].begin(); it != jMsg[MSG_KEY_EXTENSIONS].end(); ++it)
				mapExtensions[it.name()] = *it;
		}

		return ERRNO_OK;
	}

	bool ServerCapabilities::IsValid() const
	{
		return true;
	}

	int ServerCapabilities::DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const
	{
		if (bCompletions && ctxProtocol.Supports(Feature_CompletionsCapability))
		{
			Json::Value jCompletions(Json::objectValue);
			jMsg[MSG_KEY_COMPLETIONS] = jCompletions;
		}

		// Handshake profiles only emit extensions when the application
		// explicitly registered one (bExtensionPassthrough).
		bool bWriteExtensions = !mapExtensions.empty()
			&& (ctxProtocol.Supports(Feature_Extensions) || bExtensionPassthrough);
		if (bWriteExtensions)
		{
			Json::Value jExtensions(Json::objectValue);
			for (const auto& pair : mapExtensions)
				jExtensions[pair.first] = pair.second;
			jMsg[MSG_KEY_EXTENSIONS] = jExtensions;
		}

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Tool
	int Tool::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jName(strName);
		jMsg[MSG_KEY_NAME] = jName;

		if (!strDescription.empty())
		{
			Json::Value jDesc(strDescription);
			jMsg[MSG_KEY_DESCRIPTION] = jDesc;
		}

		jMsg[MSG_KEY_INPUT_SCHEMA] = jInputSchema;

		return ERRNO_OK;
	}

	int Tool::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_NAME) || !jMsg[MSG_KEY_NAME].isString())
			return ERRNO_PARSE_ERROR;
		strName = jMsg[MSG_KEY_NAME].asString();

		if (jMsg.isMember(MSG_KEY_DESCRIPTION) && jMsg[MSG_KEY_DESCRIPTION].isString())
			strDescription = jMsg[MSG_KEY_DESCRIPTION].asString();

		if (!jMsg.isMember(MSG_KEY_INPUT_SCHEMA) || !jMsg[MSG_KEY_INPUT_SCHEMA].isObject())
			return ERRNO_PARSE_ERROR;
		jInputSchema = jMsg[MSG_KEY_INPUT_SCHEMA];

		return ERRNO_OK;
	}

	bool Tool::IsValid() const
	{
		if (strName.empty())
			return false;

		if (!jInputSchema.isObject())
			return false;

		return true;
	}

	int Tool::DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const
	{
		if (ctxProtocol.Supports(Feature_ToolTitle))
		{
			WriteStringIfNotEmpty(jMsg, MSG_KEY_TITLE, strTitle);
			WriteIconsIfNotEmpty(jMsg, vecIcons);

			if (jOutputSchema.isObject() && !jOutputSchema.empty())
				jMsg[MSG_KEY_OUTPUT_SCHEMA] = jOutputSchema;
		}

		if (ctxProtocol.Supports(Feature_ToolAnnotations) && annotations.HasValue())
		{
			Json::Value jAnnotations(Json::objectValue);
			if (ERRNO_OK == annotations.DoSerialize(jAnnotations))
				jMsg[MSG_KEY_ANNOTATIONS] = jAnnotations;
		}

		if (ctxProtocol.Supports(Feature_EntityMeta))
			WriteMetaIfPresent(jMsg, jMeta);

		return ERRNO_OK;
	}

	int Tool::DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
	{
		if (ctxProtocol.Supports(Feature_ToolTitle))
		{
			if (jMsg.isMember(MSG_KEY_TITLE) && jMsg[MSG_KEY_TITLE].isString())
				strTitle = jMsg[MSG_KEY_TITLE].asString();
			ReadIconsIfPresent(jMsg, vecIcons);

			if (jMsg.isMember(MSG_KEY_OUTPUT_SCHEMA) && jMsg[MSG_KEY_OUTPUT_SCHEMA].isObject())
				jOutputSchema = jMsg[MSG_KEY_OUTPUT_SCHEMA];
		}

		if (ctxProtocol.Supports(Feature_ToolAnnotations)
			&& jMsg.isMember(MSG_KEY_ANNOTATIONS) && jMsg[MSG_KEY_ANNOTATIONS].isObject())
		{
			annotations.DoDeserialize(jMsg[MSG_KEY_ANNOTATIONS]);
		}

		if (ctxProtocol.Supports(Feature_EntityMeta)
			&& jMsg.isMember(MSG_KEY_META) && jMsg[MSG_KEY_META].isObject())
		{
			jMeta = jMsg[MSG_KEY_META];
		}

		return ERRNO_OK;
	}

	void Tool::SetUiResourceUri(const std::string& strUri)
	{
		if (!jMeta.isObject())
			jMeta = Json::Value(Json::objectValue);
		if (!jMeta[MSG_KEY_UI].isObject())
			jMeta[MSG_KEY_UI] = Json::Value(Json::objectValue);

		jMeta[MSG_KEY_UI][MSG_KEY_RESOURCE_URI] = strUri;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// TextContent
	int TextContent::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jText(strText);
		jMsg[MSG_KEY_TEXT] = jText;
		Json::Value jType(strType);
		jMsg[MSG_KEY_TYPE] = jType;

		return ERRNO_OK;
	}

	int TextContent::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_TEXT) || !jMsg[MSG_KEY_TEXT].isString())
			return ERRNO_PARSE_ERROR;
		strText = jMsg[MSG_KEY_TEXT].asString();
		if (!jMsg.isMember(MSG_KEY_TYPE) || !jMsg[MSG_KEY_TYPE].isString())
			return ERRNO_PARSE_ERROR;
		strType = jMsg[MSG_KEY_TYPE].asString();

		return ERRNO_OK;
	}

	bool TextContent::IsValid() const
	{
		if (strText.empty() || strType.empty())
			return false;

		if (strType.compare(CONST_TEXT) != 0)
			return false;

		return true;
	}

	int TextContent::DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const
	{
		WriteContentVersionExtras(jMsg, ctxProtocol, jMeta, annotations);
		return ERRNO_OK;
	}

	int TextContent::DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
	{
		ReadContentVersionExtras(jMsg, ctxProtocol, jMeta, annotations);
		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ImageContent
	int ImageContent::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jType(strType);
		jMsg[MSG_KEY_TYPE] = jType;
		Json::Value jMimeType(strMimeType);
		jMsg[MSG_KEY_MIMETYPE] = jMimeType;
		Json::Value jData(strData);
		jMsg[MSG_KEY_DATA] = jData;

		return ERRNO_OK;
	}

	int ImageContent::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_TYPE) || !jMsg[MSG_KEY_TYPE].isString())
			return ERRNO_PARSE_ERROR;
		strType = jMsg[MSG_KEY_TYPE].asString();
		if (!jMsg.isMember(MSG_KEY_MIMETYPE) || !jMsg[MSG_KEY_MIMETYPE].isString())
			return ERRNO_PARSE_ERROR;
		strMimeType = jMsg[MSG_KEY_MIMETYPE].asString();
		if (!jMsg.isMember(MSG_KEY_DATA) || !jMsg[MSG_KEY_DATA].isString())
			return ERRNO_PARSE_ERROR;
		strData = jMsg[MSG_KEY_DATA].asString();

		return ERRNO_OK;
	}

	bool ImageContent::IsValid() const
	{
		if (strType.empty() || strMimeType.empty() || strData.empty())
			return false;

		if (strType.compare(CONST_IMAGE) != 0)
			return false;

		return true;
	}

	int ImageContent::DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const
	{
		WriteContentVersionExtras(jMsg, ctxProtocol, jMeta, annotations);
		return ERRNO_OK;
	}

	int ImageContent::DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
	{
		ReadContentVersionExtras(jMsg, ctxProtocol, jMeta, annotations);
		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// AudioContent
	int AudioContent::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jType(strType);
		jMsg[MSG_KEY_TYPE] = jType;
		Json::Value jData(strData);
		jMsg[MSG_KEY_DATA] = jData;
		Json::Value jMimeType(strMimeType);
		jMsg[MSG_KEY_MIMETYPE] = jMimeType;

		return ERRNO_OK;
	}

	int AudioContent::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_TYPE) || !jMsg[MSG_KEY_TYPE].isString())
			return ERRNO_PARSE_ERROR;
		strType = jMsg[MSG_KEY_TYPE].asString();
		if (!jMsg.isMember(MSG_KEY_DATA) || !jMsg[MSG_KEY_DATA].isString())
			return ERRNO_PARSE_ERROR;
		strData = jMsg[MSG_KEY_DATA].asString();
		if (!jMsg.isMember(MSG_KEY_MIMETYPE) || !jMsg[MSG_KEY_MIMETYPE].isString())
			return ERRNO_PARSE_ERROR;
		strMimeType = jMsg[MSG_KEY_MIMETYPE].asString();

		return ERRNO_OK;
	}

	bool AudioContent::IsValid() const
	{
		if (strType.compare(CONST_AUDIO) != 0)
			return false;

		return !strData.empty() && !strMimeType.empty();
	}

	int AudioContent::DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const
	{
		WriteContentVersionExtras(jMsg, ctxProtocol, jMeta, annotations);
		return ERRNO_OK;
	}

	int AudioContent::DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
	{
		ReadContentVersionExtras(jMsg, ctxProtocol, jMeta, annotations);
		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ResourceLink
	int ResourceLink::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jType(strType);
		jMsg[MSG_KEY_TYPE] = jType;
		Json::Value jName(strName);
		jMsg[MSG_KEY_NAME] = jName;
		Json::Value jUri(strUri);
		jMsg[MSG_KEY_URI] = jUri;

		WriteStringIfNotEmpty(jMsg, MSG_KEY_TITLE, strTitle);
		WriteStringIfNotEmpty(jMsg, MSG_KEY_DESCRIPTION, strDescription);
		WriteStringIfNotEmpty(jMsg, MSG_KEY_MIMETYPE, strMimeType);

		if (iSize > 0)
		{
			Json::Value jSize(iSize);
			jMsg[MSG_KEY_SIZE] = jSize;
		}

		WriteIconsIfNotEmpty(jMsg, vecIcons);

		return ERRNO_OK;
	}

	int ResourceLink::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_TYPE) || !jMsg[MSG_KEY_TYPE].isString())
			return ERRNO_PARSE_ERROR;
		strType = jMsg[MSG_KEY_TYPE].asString();
		if (!jMsg.isMember(MSG_KEY_NAME) || !jMsg[MSG_KEY_NAME].isString())
			return ERRNO_PARSE_ERROR;
		strName = jMsg[MSG_KEY_NAME].asString();
		if (!jMsg.isMember(MSG_KEY_URI) || !jMsg[MSG_KEY_URI].isString())
			return ERRNO_PARSE_ERROR;
		strUri = jMsg[MSG_KEY_URI].asString();

		if (jMsg.isMember(MSG_KEY_TITLE) && jMsg[MSG_KEY_TITLE].isString())
			strTitle = jMsg[MSG_KEY_TITLE].asString();
		if (jMsg.isMember(MSG_KEY_DESCRIPTION) && jMsg[MSG_KEY_DESCRIPTION].isString())
			strDescription = jMsg[MSG_KEY_DESCRIPTION].asString();
		if (jMsg.isMember(MSG_KEY_MIMETYPE) && jMsg[MSG_KEY_MIMETYPE].isString())
			strMimeType = jMsg[MSG_KEY_MIMETYPE].asString();
		if (jMsg.isMember(MSG_KEY_SIZE) && jMsg[MSG_KEY_SIZE].isIntegral())
			iSize = jMsg[MSG_KEY_SIZE].asInt();

		ReadIconsIfPresent(jMsg, vecIcons);

		return ERRNO_OK;
	}

	bool ResourceLink::IsValid() const
	{
		if (strType.compare(CONST_RESOURCE_LINK) != 0)
			return false;

		return !strName.empty() && !strUri.empty();
	}

	int ResourceLink::DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const
	{
		WriteContentVersionExtras(jMsg, ctxProtocol, jMeta, annotations);
		return ERRNO_OK;
	}

	int ResourceLink::DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
	{
		ReadContentVersionExtras(jMsg, ctxProtocol, jMeta, annotations);
		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ToolUseContent
	int ToolUseContent::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jType(strType);
		jMsg[MSG_KEY_TYPE] = jType;
		Json::Value jId(strId);
		jMsg[MSG_KEY_ID] = jId;
		Json::Value jName(strName);
		jMsg[MSG_KEY_NAME] = jName;
		jMsg[MSG_KEY_INPUT] = jInput;

		return ERRNO_OK;
	}

	int ToolUseContent::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_TYPE) || !jMsg[MSG_KEY_TYPE].isString())
			return ERRNO_PARSE_ERROR;
		strType = jMsg[MSG_KEY_TYPE].asString();
		if (!jMsg.isMember(MSG_KEY_ID) || !jMsg[MSG_KEY_ID].isString())
			return ERRNO_PARSE_ERROR;
		strId = jMsg[MSG_KEY_ID].asString();
		if (!jMsg.isMember(MSG_KEY_NAME) || !jMsg[MSG_KEY_NAME].isString())
			return ERRNO_PARSE_ERROR;
		strName = jMsg[MSG_KEY_NAME].asString();
		if (!jMsg.isMember(MSG_KEY_INPUT) || !jMsg[MSG_KEY_INPUT].isObject())
			return ERRNO_PARSE_ERROR;
		jInput = jMsg[MSG_KEY_INPUT];

		return ERRNO_OK;
	}

	bool ToolUseContent::IsValid() const
	{
		if (strType.compare(CONST_TOOL_USE) != 0)
			return false;

		return !strId.empty() && !strName.empty() && jInput.isObject();
	}

	int ToolUseContent::DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const
	{
		WriteContentVersionExtras(jMsg, ctxProtocol, jMeta, annotations);
		return ERRNO_OK;
	}

	int ToolUseContent::DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
	{
		ReadContentVersionExtras(jMsg, ctxProtocol, jMeta, annotations);
		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ToolResultContent
	int ToolResultContent::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jType(strType);
		jMsg[MSG_KEY_TYPE] = jType;
		Json::Value jToolUseId(strToolUseId);
		jMsg[MSG_KEY_TOOL_USE_ID] = jToolUseId;

		if (jContent.isArray() && !jContent.empty())
			jMsg[MSG_KEY_CONTENT] = jContent;

		if (bHasIsError)
		{
			Json::Value jIsError(bIsError);
			jMsg[MSG_KEY_IS_ERROR] = jIsError;
		}

		return ERRNO_OK;
	}

	int ToolResultContent::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_TYPE) || !jMsg[MSG_KEY_TYPE].isString())
			return ERRNO_PARSE_ERROR;
		strType = jMsg[MSG_KEY_TYPE].asString();
		if (!jMsg.isMember(MSG_KEY_TOOL_USE_ID) || !jMsg[MSG_KEY_TOOL_USE_ID].isString())
			return ERRNO_PARSE_ERROR;
		strToolUseId = jMsg[MSG_KEY_TOOL_USE_ID].asString();

		if (jMsg.isMember(MSG_KEY_CONTENT) && jMsg[MSG_KEY_CONTENT].isArray())
			jContent = jMsg[MSG_KEY_CONTENT];

		if (jMsg.isMember(MSG_KEY_IS_ERROR) && jMsg[MSG_KEY_IS_ERROR].isBool())
		{
			bHasIsError = true;
			bIsError = jMsg[MSG_KEY_IS_ERROR].asBool();
		}

		if (jMsg.isMember(MSG_KEY_STRUCTURED_CONTENT))
			jStructuredContent = jMsg[MSG_KEY_STRUCTURED_CONTENT];

		return ERRNO_OK;
	}

	bool ToolResultContent::IsValid() const
	{
		if (strType.compare(CONST_TOOL_RESULT) != 0)
			return false;

		return !strToolUseId.empty();
	}

	int ToolResultContent::DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const
	{
		if (ctxProtocol.Supports(Feature_ContentToolUse) && !jStructuredContent.isNull())
			jMsg[MSG_KEY_STRUCTURED_CONTENT] = jStructuredContent;

		WriteContentVersionExtras(jMsg, ctxProtocol, jMeta, annotations);

		return ERRNO_OK;
	}

	int ToolResultContent::DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
	{
		if (ctxProtocol.Supports(Feature_ContentToolUse)
			&& jMsg.isMember(MSG_KEY_STRUCTURED_CONTENT))
		{
			jStructuredContent = jMsg[MSG_KEY_STRUCTURED_CONTENT];
		}

		ReadContentVersionExtras(jMsg, ctxProtocol, jMeta, annotations);

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// EmbeddedResource
	int EmbeddedResource::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jType(strType);
		jMsg[MSG_KEY_TYPE] = jType;

		if (textResource.IsValid())
		{
			Json::Value jTextResource(Json::objectValue);
			int iErrCode = textResource.DoSerialize(jTextResource);
			if (ERRNO_OK != iErrCode)
				return iErrCode;
			jMsg[MSG_KEY_RESOURCE] = jTextResource;
		}
		else if (blobResource.IsValid())
		{
			Json::Value jBlobResouce(Json::objectValue);
			int iErrCode = blobResource.DoSerialize(jBlobResouce);
			if (ERRNO_OK != iErrCode)
				return iErrCode;
			jMsg[MSG_KEY_RESOURCE] = jBlobResouce;
		}

		return ERRNO_OK;
	}

	int EmbeddedResource::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_TYPE) || !jMsg[MSG_KEY_TYPE].isString())
			return ERRNO_PARSE_ERROR;
		strType = jMsg[MSG_KEY_TYPE].asString();

		if (!jMsg.isMember(MSG_KEY_RESOURCE) || !jMsg[MSG_KEY_RESOURCE].isObject())
			return ERRNO_PARSE_ERROR;
		auto& jResouce = jMsg[MSG_KEY_RESOURCE];
		if (jResouce.isMember(MSG_KEY_TEXT))
		{
			return textResource.DoDeserialize(jResouce);
		}
		else
		{
			return blobResource.DoDeserialize(jResouce);
		}
	}

	bool EmbeddedResource::IsValid() const
	{
		if (strType.compare(CONST_RESOURCE) != 0)
			return false;

		if (!textResource.IsValid() && !blobResource.IsValid())
			return false;

		return true;
	}

	int EmbeddedResource::DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const
	{
		WriteContentVersionExtras(jMsg, ctxProtocol, jMeta, annotations);

		if (!jMsg.isMember(MSG_KEY_RESOURCE) || !jMsg[MSG_KEY_RESOURCE].isObject())
			return ERRNO_OK;

		if (textResource.IsValid())
			return VersionExtrasAccess::Serialize(textResource, jMsg[MSG_KEY_RESOURCE], ctxProtocol);

		if (blobResource.IsValid())
			return VersionExtrasAccess::Serialize(blobResource, jMsg[MSG_KEY_RESOURCE], ctxProtocol);

		return ERRNO_OK;
	}

	int EmbeddedResource::DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
	{
		ReadContentVersionExtras(jMsg, ctxProtocol, jMeta, annotations);

		if (!jMsg.isMember(MSG_KEY_RESOURCE) || !jMsg[MSG_KEY_RESOURCE].isObject())
			return ERRNO_OK;

		if (textResource.IsValid())
			return VersionExtrasAccess::Deserialize(textResource, jMsg[MSG_KEY_RESOURCE], ctxProtocol);

		if (blobResource.IsValid())
			return VersionExtrasAccess::Deserialize(blobResource, jMsg[MSG_KEY_RESOURCE], ctxProtocol);

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// TextResourceContents
	int TextResourceContents::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jText(strText);
		jMsg[MSG_KEY_TEXT] = jText;
		Json::Value jUri(strUri);
		jMsg[MSG_KEY_URI] = jUri;
		if (!strMimeType.empty())
		{
			Json::Value jMimeType(strMimeType);
			jMsg[MSG_KEY_MIMETYPE] = jMimeType;
		}

		return ERRNO_OK;
	}

	int TextResourceContents::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_TEXT) || !jMsg[MSG_KEY_TEXT].isString())
			return ERRNO_PARSE_ERROR;
		strText = jMsg[MSG_KEY_TEXT].asString();
		if (!jMsg.isMember(MSG_KEY_URI) || !jMsg[MSG_KEY_URI].isString())
			return ERRNO_PARSE_ERROR;
		strUri = jMsg[MSG_KEY_URI].asString();
		if (jMsg.isMember(MSG_KEY_MIMETYPE) && jMsg[MSG_KEY_MIMETYPE].isString())
			strMimeType = jMsg[MSG_KEY_MIMETYPE].asString();

		return ERRNO_OK;
	}

	bool TextResourceContents::IsValid() const
	{
		if (strText.empty() || strUri.empty())
			return false;

		return true;
	}

	int TextResourceContents::DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const
	{
		if (ctxProtocol.Supports(Feature_EntityMeta))
			WriteMetaIfPresent(jMsg, jMeta);

		return ERRNO_OK;
	}

	int TextResourceContents::DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
	{
		if (ctxProtocol.Supports(Feature_EntityMeta)
			&& jMsg.isMember(MSG_KEY_META) && jMsg[MSG_KEY_META].isObject())
		{
			jMeta = jMsg[MSG_KEY_META];
		}

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// BlobResourceContents
	int BlobResourceContents::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jBlob(strBlob);
		jMsg[MSG_KEY_BLOB] = jBlob;
		Json::Value jUri(strUri);
		jMsg[MSG_KEY_URI] = jUri;
		if (!strMimeType.empty())
		{
			Json::Value jMimeType(strMimeType);
			jMsg[MSG_KEY_MIMETYPE] = jMimeType;
		}

		return ERRNO_OK;
	}

	int BlobResourceContents::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_BLOB) || !jMsg[MSG_KEY_BLOB].isString())
			return ERRNO_PARSE_ERROR;
		strBlob = jMsg[MSG_KEY_BLOB].asString();
		if (!jMsg.isMember(MSG_KEY_URI) || !jMsg[MSG_KEY_URI].isString())
			return ERRNO_PARSE_ERROR;
		strUri = jMsg[MSG_KEY_URI].asString();
		if (jMsg.isMember(MSG_KEY_MIMETYPE) && jMsg[MSG_KEY_MIMETYPE].isString())
			strMimeType = jMsg[MSG_KEY_MIMETYPE].asString();

		return ERRNO_OK;
	}

	bool BlobResourceContents::IsValid() const
	{
		if (strBlob.empty() || strUri.empty())
			return false;

		return true;
	}

	int BlobResourceContents::DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const
	{
		if (ctxProtocol.Supports(Feature_EntityMeta))
			WriteMetaIfPresent(jMsg, jMeta);

		return ERRNO_OK;
	}

	int BlobResourceContents::DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
	{
		if (ctxProtocol.Supports(Feature_EntityMeta)
			&& jMsg.isMember(MSG_KEY_META) && jMsg[MSG_KEY_META].isObject())
		{
			jMeta = jMsg[MSG_KEY_META];
		}

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Resource
	int Resource::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jName(strName);
		jMsg[MSG_KEY_NAME] = jName;
		Json::Value jUri(strUri);
		jMsg[MSG_KEY_URI] = jUri;
		if (!strDescription.empty())
		{
			Json::Value jDesc(strDescription);
			jMsg[MSG_KEY_DESCRIPTION] = jDesc;
		}
		if (!strMimeType.empty())
		{
			Json::Value jMimeType(strMimeType);
			jMsg[MSG_KEY_MIMETYPE] = jMimeType;
		}
		if (iSize > 0)
		{
			Json::Value jSize(iSize);
			jMsg[MSG_KEY_SIZE] = jSize;
		}

		return ERRNO_OK;
	}

	int Resource::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_NAME) || !jMsg[MSG_KEY_NAME].isString())
			return ERRNO_PARSE_ERROR;
		strName = jMsg[MSG_KEY_NAME].asString();
		if (!jMsg.isMember(MSG_KEY_URI) || !jMsg[MSG_KEY_URI].isString())
			return ERRNO_PARSE_ERROR;
		strUri = jMsg[MSG_KEY_URI].asString();
		if (jMsg.isMember(MSG_KEY_DESCRIPTION) && jMsg[MSG_KEY_DESCRIPTION].isString())
			strDescription = jMsg[MSG_KEY_DESCRIPTION].asString();
		if (jMsg.isMember(MSG_KEY_MIMETYPE) && jMsg[MSG_KEY_MIMETYPE].isString())
			strMimeType = jMsg[MSG_KEY_MIMETYPE].asString();
		if (jMsg.isMember(MSG_KEY_SIZE) && jMsg[MSG_KEY_SIZE].isIntegral())
			iSize = jMsg[MSG_KEY_SIZE].asInt();

		return ERRNO_OK;
	}

	bool Resource::IsValid() const
	{
		if (strName.empty() || strUri.empty())
			return false;

		return true;
	}

	int Resource::DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const
	{
		WriteEntityVersionExtras(jMsg, ctxProtocol, strTitle, vecIcons, jMeta,
			&annotations, Feature_ToolAnnotations);

		return ERRNO_OK;
	}

	int Resource::DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
	{
		ReadEntityVersionExtras(jMsg, ctxProtocol, strTitle, vecIcons, jMeta,
			&annotations, Feature_ToolAnnotations);

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ResourceTemplate
	int ResourceTemplate::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jName(strName);
		jMsg[MSG_KEY_NAME] = jName;
		Json::Value jUriTemplate(strUriTemplate);
		jMsg[MSG_KEY_URI_TEMPLATE] = jUriTemplate;
		if (!strDescription.empty())
		{
			Json::Value jDesc(strDescription);
			jMsg[MSG_KEY_DESCRIPTION] = jDesc;
		}
		if (!strMimeType.empty())
		{
			Json::Value jMimeType(strMimeType);
			jMsg[MSG_KEY_MIMETYPE] = jMimeType;
		}

		return ERRNO_OK;
	}

	int ResourceTemplate::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_NAME) || !jMsg[MSG_KEY_NAME].isString())
			return ERRNO_PARSE_ERROR;
		strName = jMsg[MSG_KEY_NAME].asString();
		if (!jMsg.isMember(MSG_KEY_URI_TEMPLATE) || !jMsg[MSG_KEY_URI_TEMPLATE].isString())
			return ERRNO_PARSE_ERROR;
		strUriTemplate = jMsg[MSG_KEY_URI_TEMPLATE].asString();
		if (jMsg.isMember(MSG_KEY_DESCRIPTION) && jMsg[MSG_KEY_DESCRIPTION].isString())
			strDescription = jMsg[MSG_KEY_DESCRIPTION].asString();
		if (jMsg.isMember(MSG_KEY_MIMETYPE) && jMsg[MSG_KEY_MIMETYPE].isString())
			strMimeType = jMsg[MSG_KEY_MIMETYPE].asString();

		return ERRNO_OK;
	}

	bool ResourceTemplate::IsValid() const
	{
		if (strName.empty() || strUriTemplate.empty())
			return false;

		return true;
	}

	int ResourceTemplate::DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const
	{
		WriteEntityVersionExtras(jMsg, ctxProtocol, strTitle, vecIcons, jMeta,
			&annotations, Feature_ToolAnnotations);

		return ERRNO_OK;
	}

	int ResourceTemplate::DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
	{
		ReadEntityVersionExtras(jMsg, ctxProtocol, strTitle, vecIcons, jMeta,
			&annotations, Feature_ToolAnnotations);

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProgressToken
	int ProgressToken::DoSerialize(Json::Value& jMsg) const
	{
		if (DataType_String == eTokenDataType)
		{
			Json::Value jId(strToken);
			jMsg[MSG_KEY_PROGRESS_TOKEN] = jId;
		}
		else if (DataType_Integer == eTokenDataType)
		{
			Json::Value jId(iToken);
			jMsg[MSG_KEY_PROGRESS_TOKEN] = jId;
		}

		return ERRNO_OK;
	}

	int ProgressToken::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_PROGRESS_TOKEN))
			return ERRNO_INVALID_REQUEST;
		bool bStrId = jMsg[MSG_KEY_PROGRESS_TOKEN].isString();
		bool bIntId = jMsg[MSG_KEY_PROGRESS_TOKEN].isIntegral();
		if (!bStrId && !bIntId)
			return ERRNO_INVALID_REQUEST;
		if (bStrId)
		{
			eTokenDataType = DataType_String;
			strToken = jMsg[MSG_KEY_PROGRESS_TOKEN].asString();
		}
		if (bIntId)
		{
			eTokenDataType = DataType_Integer;
			iToken = jMsg[MSG_KEY_PROGRESS_TOKEN].asInt();
		}

		return ERRNO_OK;
	}

	bool ProgressToken::IsValid() const
	{
		if (eTokenDataType != DataType_String && eTokenDataType != DataType_Integer)
			return false;

		if (DataType_String == eTokenDataType && strToken.empty())
			return false;

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// PromptArgument
	int PromptArgument::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jName(strName);
		jMsg[MSG_KEY_NAME] = jName;

		if (!strDescription.empty())
		{
			Json::Value jDesc(strDescription);
			jMsg[MSG_KEY_DESCRIPTION] = jDesc;
		}

		Json::Value jRequired(bRequired);
		jMsg[MSG_KEY_REQUIRED] = jRequired;

		return ERRNO_OK;
	}

	int PromptArgument::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_NAME) || !jMsg[MSG_KEY_NAME].isString())
			return ERRNO_PARSE_ERROR;
		strName = jMsg[MSG_KEY_NAME].asString();

		if (jMsg.isMember(MSG_KEY_DESCRIPTION) && jMsg[MSG_KEY_DESCRIPTION].isString())
			strDescription = jMsg[MSG_KEY_DESCRIPTION].asString();

		if (jMsg.isMember(MSG_KEY_REQUIRED) && jMsg[MSG_KEY_REQUIRED].isBool())
			bRequired = jMsg[MSG_KEY_REQUIRED].asBool();

		return ERRNO_OK;
	}

	bool PromptArgument::IsValid() const
	{
		return !strName.empty();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Prompt
	int Prompt::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jName(strName);
		jMsg[MSG_KEY_NAME] = jName;

		if (!strDescription.empty())
		{
			Json::Value jDesc(strDescription);
			jMsg[MSG_KEY_DESCRIPTION] = jDesc;
		}

		if (!vecArguments.empty())
		{
			Json::Value jArgs(Json::arrayValue);
			for (const auto& arg : vecArguments)
			{
				Json::Value jArg(Json::objectValue);
				arg.DoSerialize(jArg);
				jArgs.append(jArg);
			}
			jMsg[MSG_KEY_ARGUMENTS] = jArgs;
		}

		return ERRNO_OK;
	}

	int Prompt::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_NAME) || !jMsg[MSG_KEY_NAME].isString())
			return ERRNO_PARSE_ERROR;
		strName = jMsg[MSG_KEY_NAME].asString();

		if (jMsg.isMember(MSG_KEY_DESCRIPTION) && jMsg[MSG_KEY_DESCRIPTION].isString())
			strDescription = jMsg[MSG_KEY_DESCRIPTION].asString();

		if (jMsg.isMember(MSG_KEY_ARGUMENTS) && jMsg[MSG_KEY_ARGUMENTS].isArray())
		{
			vecArguments.clear();
			for (const auto& jArg : jMsg[MSG_KEY_ARGUMENTS])
			{
				PromptArgument arg;
				arg.DoDeserialize(jArg);
				vecArguments.push_back(arg);
			}
		}

		return ERRNO_OK;
	}

	bool Prompt::IsValid() const
	{
		return !strName.empty();
	}

	int Prompt::DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const
	{
		WriteEntityVersionExtras(jMsg, ctxProtocol, strTitle, vecIcons, jMeta,
			nullptr, Feature_ToolAnnotations);

		return ERRNO_OK;
	}

	int Prompt::DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
	{
		ReadEntityVersionExtras(jMsg, ctxProtocol, strTitle, vecIcons, jMeta,
			nullptr, Feature_ToolAnnotations);

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// PromptMessage
	int PromptMessage::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jRole(strRole);
		jMsg[MSG_KEY_ROLE] = jRole;

		Json::Value jContent(Json::objectValue);
		if (strContentType == CONST_TEXT)
		{
			textContent.DoSerialize(jContent);
		}
		else if (strContentType == CONST_IMAGE)
		{
			imageContent.DoSerialize(jContent);
		}
		else if (strContentType == CONST_RESOURCE)
		{
			embeddedResource.DoSerialize(jContent);
		}
		jMsg[MSG_KEY_CONTENT] = jContent;

		return ERRNO_OK;
	}

	int PromptMessage::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_ROLE) || !jMsg[MSG_KEY_ROLE].isString())
			return ERRNO_PARSE_ERROR;
		strRole = jMsg[MSG_KEY_ROLE].asString();

		if (!jMsg.isMember(MSG_KEY_CONTENT) || !jMsg[MSG_KEY_CONTENT].isObject())
			return ERRNO_PARSE_ERROR;

		const Json::Value& jContent = jMsg[MSG_KEY_CONTENT];
		if (!jContent.isMember(MSG_KEY_TYPE) || !jContent[MSG_KEY_TYPE].isString())
			return ERRNO_PARSE_ERROR;

		strContentType = jContent[MSG_KEY_TYPE].asString();

		if (strContentType == CONST_TEXT)
		{
			textContent.DoDeserialize(jContent);
		}
		else if (strContentType == CONST_IMAGE)
		{
			imageContent.DoDeserialize(jContent);
		}
		else if (strContentType == CONST_RESOURCE)
		{
			embeddedResource.DoDeserialize(jContent);
		}

		return ERRNO_OK;
	}

	bool PromptMessage::IsValid() const
	{
		return (strRole == CONST_USER || strRole == CONST_ASSISTANT) && !strContentType.empty();
	}

	int PromptMessage::DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const
	{
		if (!jMsg.isMember(MSG_KEY_CONTENT) || !jMsg[MSG_KEY_CONTENT].isObject())
			return ERRNO_OK;

		Json::Value& jContent = jMsg[MSG_KEY_CONTENT];
		if (strContentType == CONST_TEXT)
			return VersionExtrasAccess::Serialize(textContent, jContent, ctxProtocol);
		if (strContentType == CONST_IMAGE)
			return VersionExtrasAccess::Serialize(imageContent, jContent, ctxProtocol);
		if (strContentType == CONST_RESOURCE)
			return VersionExtrasAccess::Serialize(embeddedResource, jContent, ctxProtocol);

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// PromptReference
	int PromptReference::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jType(strType);
		jMsg[MSG_KEY_TYPE] = jType;
		Json::Value jName(strName);
		jMsg[MSG_KEY_NAME] = jName;

		return ERRNO_OK;
	}

	int PromptReference::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_TYPE) || !jMsg[MSG_KEY_TYPE].isString())
			return ERRNO_PARSE_ERROR;
		strType = jMsg[MSG_KEY_TYPE].asString();

		if (!jMsg.isMember(MSG_KEY_NAME) || !jMsg[MSG_KEY_NAME].isString())
			return ERRNO_PARSE_ERROR;
		strName = jMsg[MSG_KEY_NAME].asString();

		return ERRNO_OK;
	}

	bool PromptReference::IsValid() const
	{
		return strType == CONST_REF_PROMPT && !strName.empty();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ResourceReference
	int ResourceReference::DoSerialize(Json::Value& jMsg) const
	{
		Json::Value jType(strType);
		jMsg[MSG_KEY_TYPE] = jType;
		Json::Value jUri(strUri);
		jMsg[MSG_KEY_URI] = jUri;

		return ERRNO_OK;
	}

	int ResourceReference::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_TYPE) || !jMsg[MSG_KEY_TYPE].isString())
			return ERRNO_PARSE_ERROR;
		strType = jMsg[MSG_KEY_TYPE].asString();

		if (!jMsg.isMember(MSG_KEY_URI) || !jMsg[MSG_KEY_URI].isString())
			return ERRNO_PARSE_ERROR;
		strUri = jMsg[MSG_KEY_URI].asString();

		return ERRNO_OK;
	}

	bool ResourceReference::IsValid() const
	{
		return strType == CONST_REF_RESOURCE && !strUri.empty();
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// LoggingLevel
	int LoggingLevel::DoSerialize(Json::Value& jMsg) const
	{
		std::string strLevel;
		if (eLevel != LoggingLevelValue::LoggingLevel_Unknown)
		{
			switch (eLevel)
			{
				case LoggingLevelValue::LoggingLevel_Debug:
					strLevel = LOGGING_LEVEL_DEBUG;
					break;
				case LoggingLevelValue::LoggingLevel_Info:
					strLevel = LOGGING_LEVEL_INFO;
					break;
				case LoggingLevelValue::LoggingLevel_Notice:
					strLevel = LOGGING_LEVEL_NOTICE;
					break;
				case LoggingLevelValue::LoggingLevel_Warning:
					strLevel = LOGGING_LEVEL_WARNING;
					break;
				case LoggingLevelValue::LoggingLevel_Error:
					strLevel = LOGGING_LEVEL_ERROR;
					break;
				case LoggingLevelValue::LoggingLevel_Critical:
					strLevel = LOGGING_LEVEL_CRITICAL;
					break;
				case LoggingLevelValue::LoggingLevel_Alert:
					strLevel = LOGGING_LEVEL_ALERT;
					break;
				case LoggingLevelValue::LoggingLevel_Emergency:
					strLevel = LOGGING_LEVEL_EMERGENCY;
					break;
				default:
					break;
			}
		}
		Json::Value jLevel(strLevel);
		jMsg[MSG_KEY_LEVEL] = jLevel;

		return ERRNO_OK;
	}

	int LoggingLevel::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isMember(MSG_KEY_LEVEL) || !jMsg[MSG_KEY_LEVEL].isString())
			return ERRNO_PARSE_ERROR;
		std::string strLevel = jMsg[MSG_KEY_LEVEL].asString();

		if (strLevel == LOGGING_LEVEL_DEBUG)
			eLevel = LoggingLevelValue::LoggingLevel_Debug;
		else if (strLevel == LOGGING_LEVEL_INFO)
			eLevel = LoggingLevelValue::LoggingLevel_Info;
		else if (strLevel == LOGGING_LEVEL_NOTICE)
			eLevel = LoggingLevelValue::LoggingLevel_Notice;
		else if (strLevel == LOGGING_LEVEL_WARNING)
			eLevel = LoggingLevelValue::LoggingLevel_Warning;
		else if (strLevel == LOGGING_LEVEL_ERROR)
			eLevel = LoggingLevelValue::LoggingLevel_Error;
		else if (strLevel == LOGGING_LEVEL_CRITICAL)
			eLevel = LoggingLevelValue::LoggingLevel_Critical;
		else if (strLevel == LOGGING_LEVEL_ALERT)
			eLevel = LoggingLevelValue::LoggingLevel_Alert;
		else if (strLevel == LOGGING_LEVEL_EMERGENCY)
			eLevel = LoggingLevelValue::LoggingLevel_Emergency;
		else
			return ERRNO_PARSE_ERROR;

		return ERRNO_OK;
	}

	bool LoggingLevel::IsValid() const
	{
		return eLevel != LoggingLevelValue::LoggingLevel_Unknown;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Logging
	int Logging::DoSerialize(Json::Value& jMsg) const
	{
		return ERRNO_OK;
	}

	int Logging::DoDeserialize(const Json::Value& jMsg)
	{
		return ERRNO_OK;
	}

	bool Logging::IsValid() const
	{
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ClientCapabilities
	bool ClientCapabilities::HasValue() const
	{
		return (jRaw.isObject() && !jRaw.empty()) || !mapExtensions.empty();
	}

	bool ClientCapabilities::SupportsCapability(const std::string& strCapability) const
	{
		return jRaw.isObject() && jRaw.isMember(strCapability);
	}

	bool ClientCapabilities::SupportsElicitationMode(const std::string& strMode) const
	{
		if (!jRaw.isObject() || !jRaw.isMember(MSG_KEY_ELICITATION))
			return false;

		const Json::Value& jElicitation = jRaw[MSG_KEY_ELICITATION];
		return jElicitation.isObject() && jElicitation.isMember(strMode);
	}

	bool ClientCapabilities::SupportsExtension(const std::string& strExtensionId) const
	{
		if (mapExtensions.find(strExtensionId) != mapExtensions.end())
			return true;

		if (jRaw.isObject() && jRaw.isMember(MSG_KEY_EXTENSIONS) && jRaw[MSG_KEY_EXTENSIONS].isObject())
			return jRaw[MSG_KEY_EXTENSIONS].isMember(strExtensionId);

		return false;
	}

	int ClientCapabilities::DoSerialize(Json::Value& jMsg) const
	{
		if (jRaw.isObject())
		{
			for (auto it = jRaw.begin(); it != jRaw.end(); ++it)
				jMsg[it.name()] = *it;
		}

		if (!mapExtensions.empty())
		{
			Json::Value jExtensions(Json::objectValue);
			for (const auto& pair : mapExtensions)
				jExtensions[pair.first] = pair.second;
			jMsg[MSG_KEY_EXTENSIONS] = jExtensions;
		}

		return ERRNO_OK;
	}

	int ClientCapabilities::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isObject())
			return ERRNO_PARSE_ERROR;

		jRaw = jMsg;
		mapExtensions.clear();

		if (jMsg.isMember(MSG_KEY_EXTENSIONS) && jMsg[MSG_KEY_EXTENSIONS].isObject())
		{
			for (auto it = jMsg[MSG_KEY_EXTENSIONS].begin(); it != jMsg[MSG_KEY_EXTENSIONS].end(); ++it)
				mapExtensions[it.name()] = *it;
		}

		return ERRNO_OK;
	}

	bool ClientCapabilities::IsValid() const
	{
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// MetaObject
	bool MetaObject::IsReservedKey(const std::string& strKey)
	{
		return strKey == META_KEY_PROTOCOL_VERSION
			|| strKey == META_KEY_CLIENT_INFO
			|| strKey == META_KEY_CLIENT_CAPABILITIES
			|| strKey == META_KEY_LOG_LEVEL;
	}

	bool MetaObject::HasValue() const
	{
		bool bHasClientInfo = !clientInfo.strName.empty() || !clientInfo.strVersion.empty();
		return !strProtocolVersion.empty()
			|| bHasClientInfo
			|| bHasClientCapabilities
			|| !strLogLevel.empty()
			|| (jExtra.isObject() && !jExtra.empty());
	}

	int MetaObject::DoSerialize(Json::Value& jMsg) const
	{
		// Application defined keys first, then the protocol reserved keys.
		if (jExtra.isObject())
		{
			for (auto it = jExtra.begin(); it != jExtra.end(); ++it)
			{
				if (!IsReservedKey(it.name()))
					jMsg[it.name()] = *it;
			}
		}

		WriteStringIfNotEmpty(jMsg, META_KEY_PROTOCOL_VERSION, strProtocolVersion);

		if (!clientInfo.strName.empty() || !clientInfo.strVersion.empty())
		{
			Json::Value jClientInfo(Json::objectValue);
			clientInfo.DoSerialize(jClientInfo);
			jMsg[META_KEY_CLIENT_INFO] = jClientInfo;
		}

		if (bHasClientCapabilities || clientCapabilities.HasValue())
		{
			Json::Value jClientCapabilities(Json::objectValue);
			clientCapabilities.DoSerialize(jClientCapabilities);
			jMsg[META_KEY_CLIENT_CAPABILITIES] = jClientCapabilities;
		}

		WriteStringIfNotEmpty(jMsg, META_KEY_LOG_LEVEL, strLogLevel);

		return ERRNO_OK;
	}

	int MetaObject::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isObject())
			return ERRNO_PARSE_ERROR;

		for (auto it = jMsg.begin(); it != jMsg.end(); ++it)
		{
			const std::string strKey = it.name();
			if (strKey == META_KEY_PROTOCOL_VERSION)
			{
				if (it->isString())
					strProtocolVersion = it->asString();
			}
			else if (strKey == META_KEY_CLIENT_INFO)
			{
				if (it->isObject())
					clientInfo.DoDeserialize(*it);
			}
			else if (strKey == META_KEY_CLIENT_CAPABILITIES)
			{
				if (it->isObject())
				{
					bHasClientCapabilities = true;
					clientCapabilities.DoDeserialize(*it);
				}
			}
			else if (strKey == META_KEY_LOG_LEVEL)
			{
				if (it->isString())
					strLogLevel = it->asString();
			}
			else if (strKey != MSG_KEY_PROGRESS_TOKEN && strKey != MSG_KEY_TOOL_CALL_ID)
			{
				// Unknown keys are preserved as-is; progressToken/toolCallId stay
				// on their existing Request members and are never duplicated here.
				if (!jExtra.isObject())
					jExtra = Json::Value(Json::objectValue);
				jExtra[strKey] = *it;
			}
		}

		return ERRNO_OK;
	}

	bool MetaObject::IsValid() const
	{
		return true;
	}

	int MetaObject::DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const
	{
		if (!ctxProtocol.Supports(Feature_ImplementationExtras))
			return ERRNO_OK;

		if (!jMsg.isMember(META_KEY_CLIENT_INFO) || !jMsg[META_KEY_CLIENT_INFO].isObject())
			return ERRNO_OK;

		return VersionExtrasAccess::Serialize(clientInfo, jMsg[META_KEY_CLIENT_INFO], ctxProtocol);
	}

	int MetaObject::DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
	{
		if (!ctxProtocol.Supports(Feature_ImplementationExtras))
			return ERRNO_OK;

		if (!jMsg.isMember(META_KEY_CLIENT_INFO) || !jMsg[META_KEY_CLIENT_INFO].isObject())
			return ERRNO_OK;

		return VersionExtrasAccess::Deserialize(clientInfo, jMsg[META_KEY_CLIENT_INFO], ctxProtocol);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// InputRequests
	bool InputRequests::IsValid() const
	{
		return jItems.isObject() && !jItems.empty();
	}

	bool InputRequests::Has(const std::string& strKey) const
	{
		return jItems.isObject() && jItems.isMember(strKey);
	}

	const Json::Value& InputRequests::Get(const std::string& strKey) const
	{
		static const Json::Value s_jEmpty;
		if (!Has(strKey))
			return s_jEmpty;

		return jItems[strKey];
	}

	int InputRequests::DoSerialize(Json::Value& jMsg) const
	{
		if (!jItems.isObject())
			return ERRNO_OK;

		for (auto it = jItems.begin(); it != jItems.end(); ++it)
			jMsg[it.name()] = *it;

		return ERRNO_OK;
	}

	int InputRequests::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isObject())
			return ERRNO_PARSE_ERROR;

		jItems = jMsg;
		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// InputResponses
	bool InputResponses::IsValid() const
	{
		return jItems.isObject() && !jItems.empty();
	}

	bool InputResponses::Has(const std::string& strKey) const
	{
		return jItems.isObject() && jItems.isMember(strKey);
	}

	const Json::Value& InputResponses::Get(const std::string& strKey) const
	{
		static const Json::Value s_jEmpty;
		if (!Has(strKey))
			return s_jEmpty;

		return jItems[strKey];
	}

	int InputResponses::DoSerialize(Json::Value& jMsg) const
	{
		if (!jItems.isObject())
			return ERRNO_OK;

		for (auto it = jItems.begin(); it != jItems.end(); ++it)
			jMsg[it.name()] = *it;

		return ERRNO_OK;
	}

	int InputResponses::DoDeserialize(const Json::Value& jMsg)
	{
		if (!jMsg.isObject())
			return ERRNO_PARSE_ERROR;

		jItems = jMsg;
		return ERRNO_OK;
	}
}