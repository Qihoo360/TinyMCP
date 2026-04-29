#include "BasicMessage.h"
#include <json/json.h>

namespace MCP
{
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

		return ERRNO_OK;
	}

	bool ServerCapabilities::IsValid() const
	{
		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ServerCapabilities
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
}