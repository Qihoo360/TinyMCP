#pragma once
// To ensure good cross-platform compatibility, all code within the MCP namespace is written in standard C++.
// The use of platform-specific system APIs is prohibited unless necessary.

#include "Message.h"

#include <map>
#include <set>

namespace MCP
{
	// Internal gateway for protocol revision serialization. Entity classes keep
	// their version-adaptation helpers out of the public API; SDK code and tests
	// reach them through this friend declared in Internal/VersionExtras.h.
	struct VersionExtrasAccess;

	// 2026-07-28 icon descriptor used by implementations and tools.
	struct Icon : public MCP::Message
	{
	public:
		Icon()
			: Message(MessageType_Icon, MessageCategory_Basic, false)
		{
		}

		std::string strSrc;
		std::string strMimeType;
		std::vector<std::string> vecSizes;
		std::string strTheme;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct Annotations : public MCP::Message
	{
	public:
		Annotations()
			: Message(MessageType_Unknown, MessageCategory_Basic, false)
		{
		}

		std::vector<std::string> vecAudience;
		double dPriority{ 0.0 };
		bool bHasPriority{ false };
		std::string strLastModified;

		bool HasValue() const;
		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ToolAnnotations : public MCP::Message
	{
	public:
		ToolAnnotations()
			: Message(MessageType_Unknown, MessageCategory_Basic, false)
		{
		}

		std::string strTitle;
		bool bHasReadOnlyHint{ false };
		bool bReadOnlyHint{ false };
		bool bHasDestructiveHint{ false };
		bool bDestructiveHint{ false };
		bool bHasIdempotentHint{ false };
		bool bIdempotentHint{ false };
		bool bHasOpenWorldHint{ false };
		bool bOpenWorldHint{ false };

		bool HasValue() const;
		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct RequestId : public MCP::Message
	{
	public:
		RequestId()
			: Message(MessageType_RequestId, MessageCategory_Basic, false)
		{

		}

		DataType eIdDataType{ DataType_Unknown };
		int iId{ -1 };
		std::string strId;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Writes only the scalar id value, preserving string/integer type.
		int WriteValue(Json::Value& jValue) const;

		inline void SetMsgKey(const std::string& strMsgKey)
		{
			_strMsgKey = strMsgKey;
		}
		inline bool IsEqual(const MCP::RequestId& rhs)
		{
			return eIdDataType == rhs.eIdDataType
				&& iId == rhs.iId
				&& strId == rhs.strId;
		}

	private:
		std::string _strMsgKey;
	};

	struct Implementation : public MCP::Message
	{
	public:
		Implementation()
			: Message(MessageType_Implementation, MessageCategory_Basic, false)
		{

		}

		std::string strName;
		std::string strVersion;
		std::string strTitle;
		std::string strDescription;
		std::vector<Icon> vecIcons;
		std::string strWebsiteUrl;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

	private:
		int DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const;
		int DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol);
	};

	struct Prompts : public MCP::Message
	{
	public:
		Prompts()
			: Message(MessageType_Prompts, MessageCategory_Basic, false)
		{

		}

		bool bListChanged{ false };

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct Resources : public MCP::Message
	{
	public:
		Resources()
			: Message(MessageType_Resources, MessageCategory_Basic, false)
		{

		}

		bool bListChanged{ false };
		bool bSubscribe{ false };

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct Tools : public MCP::Message
	{
	public:
		Tools()
			: Message(MessageType_Tools, MessageCategory_Basic, false)
		{

		}

		bool bListChanged{ false };

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct Experimental : public MCP::Message
	{
		Experimental()
			: Message(MessageType_Experimental, MessageCategory_Basic, false)
		{
		}

		bool bCompletion{ false };

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct LoggingLevel : public MCP::Message
	{
		LoggingLevel()
			: Message(MessageType_LoggingLevel, MessageCategory_Basic, false)
		{
		}

		LoggingLevelValue eLevel{ LoggingLevelValue::LoggingLevel_Unknown };

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct Logging : public MCP::Message
	{
		Logging()
			: Message(MessageType_Logging, MessageCategory_Basic, false)
		{
		}

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ServerCapabilities : public MCP::Message
	{
	public:
		ServerCapabilities()
			: Message(MessageType_ServerCapabilities, MessageCategory_Basic, false)
		{
			prompts.bExist = false;
			resources.bExist = false;
			tools.bExist = false;
			logging.bExist = false;
		}

		MCP::Prompts prompts;
		MCP::Resources resources;
		MCP::Tools tools;
		MCP::Experimental experimental;
		MCP::Logging logging;
		bool bCompletions{ false };
		std::map<std::string, Json::Value> mapExtensions;
		// Extensions that are only valid on the stateless era. They are dropped by
		// the legacy passthrough so the handshake surface never advertises them.
		std::set<std::string> setModernOnlyExtensions;
		// A registered extension is explicitly opted in by the application, so a
		// handshake-era initialize response must pass it through even though the
		// legacy profile does not declare Feature_Extensions.
		bool bExtensionPassthrough{ false };

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

	private:
		int DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const;
	};

	// JSON Schema object used by Tool.inputSchema/outputSchema. Only object
	// schemas are supported, matching the MCP schema requirement.
	struct JsonSchema
	{
	public:
		void Set(const Json::Value& jValue);
		const Json::Value& Get() const;
		bool HasValue() const;
		void Clear();
		bool IsValid() const;
		int DoSerialize(Json::Value& jValue) const;
		int DoDeserialize(const Json::Value& jValue);

	private:
		Json::Value m_jData;
	};

	struct Tool : public MCP::Message
	{
		Tool()
			: Message(MessageType_Tool, MessageCategory_Basic, false)
		{

		}

		std::string strName;
		std::string strDescription;
		MCP::JsonSchema inputSchema;
		std::string strTitle;
		std::vector<Icon> vecIcons;
		MCP::JsonSchema outputSchema;
		ToolAnnotations annotations;
		Json::Value jMeta;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

	private:
		int DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const;
		int DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol);

	public:
		// Writes _meta.ui.resourceUri used by the MCP Apps extension.
		void SetUiResourceUri(const std::string& strUri);
		// Writes _meta.ui.visibility while keeping resourceUri and unknown keys.
		void SetUiVisibility(const std::vector<std::string>& vecVisibility);
	};

	struct TextContent : public MCP::Message
	{
		TextContent()
			: Message(MessageType_TextContent, MessageCategory_Basic, false)
		{

		}

		std::string strText;
		std::string strType;
		Annotations annotations;
		Json::Value jMeta;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

	private:
		int DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const;
		int DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol);
	};

	struct ImageContent : public MCP::Message
	{
		ImageContent()
			: Message(MessageType_ImageContent, MessageCategory_Basic, false)
		{

		}

		std::string strType;
		std::string strMimeType;
		std::string strData;
		Annotations annotations;
		Json::Value jMeta;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

	private:
		int DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const;
		int DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol);
	};

	// 2026-07-28 audio content block.
	struct AudioContent : public MCP::Message
	{
		AudioContent()
			: Message(MessageType_AudioContent, MessageCategory_Basic, false)
		{
		}

		std::string strType;
		std::string strData;
		std::string strMimeType;
		Annotations annotations;
		Json::Value jMeta;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

	private:
		int DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const;
		int DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol);
	};

	// 2026-07-28 resource link content block.
	struct ResourceLink : public MCP::Message
	{
		ResourceLink()
			: Message(MessageType_ResourceLink, MessageCategory_Basic, false)
		{
		}

		std::string strType;
		std::string strName;
		std::string strUri;
		std::string strTitle;
		std::string strDescription;
		std::string strMimeType;
		int iSize{ 0 };
		std::vector<Icon> vecIcons;
		Annotations annotations;
		Json::Value jMeta;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

	private:
		int DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const;
		int DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol);
	};

	// 2026-07-28 tool use content block.
	struct ToolUseContent : public MCP::Message
	{
		ToolUseContent()
			: Message(MessageType_ToolUseContent, MessageCategory_Basic, false)
		{
		}

		std::string strType;
		std::string strId;
		std::string strName;
		Json::Value jInput;
		Annotations annotations;
		Json::Value jMeta;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

	private:
		int DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const;
		int DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol);
	};

	// structuredContent is a JSON object carried by tool results. It is not a
	// JSON-RPC message, so it is modelled as a plain field type.
	struct StructuredContent
	{
	public:
		void Set(const Json::Value& jValue);
		const Json::Value& Get() const;
		bool HasValue() const;
		void Clear();
		bool IsValid() const;
		int DoSerialize(Json::Value& jValue) const;
		int DoDeserialize(const Json::Value& jValue);

	private:
		Json::Value m_jData;
	};

	// 2026-07-28 tool result content block. Nested content blocks are kept as
	// raw JSON so callers can pass through client supplied structures.
	struct ToolResultContent : public MCP::Message
	{
		ToolResultContent()
			: Message(MessageType_ToolResultContent, MessageCategory_Basic, false)
		{
		}

		std::string strType;
		std::string strToolUseId;
		Json::Value jContent;
		bool bHasIsError{ false };
		bool bIsError{ false };
		MCP::StructuredContent structuredContent;
		Annotations annotations;
		Json::Value jMeta;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

	private:
		int DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const;
		int DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol);
	};

	// MCP Apps resource metadata carried by _meta.ui. Unknown sub-keys are kept
	// verbatim so a newer revision can pass through fields the SDK does not model.
	struct UiResourceMeta
	{
	public:
		std::vector<std::string> vecConnectDomains;
		std::vector<std::string> vecResourceDomains;
		std::vector<std::string> vecFrameDomains;
		std::vector<std::string> vecBaseUriDomains;
		bool bCamera{ false };
		bool bMicrophone{ false };
		bool bGeolocation{ false };
		bool bClipboardWrite{ false };
		std::string strDomain;
		bool bHasDomain{ false };
		bool bPrefersBorder{ false };
		bool bHasPrefersBorder{ false };
		Json::Value jExtra;

		bool HasValue() const;
		int DoSerialize(Json::Value& jUi) const;
	};

	struct TextResourceContents : public MCP::Message
	{
		TextResourceContents()
			: Message(MessageType_TextResourceContents, MessageCategory_Basic, false)
		{

		}

		std::string strText;
		std::string strUri;
		std::string strMimeType;
		Json::Value jMeta;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

		// Writes _meta.ui while keeping unmodeled meta keys intact.
		void SetUiMeta(const UiResourceMeta& uiMeta);

	private:
		int DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const;
		int DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol);
	};

	struct BlobResourceContents : public MCP::Message
	{
		BlobResourceContents()
			: Message(MessageType_BlobResourceContents, MessageCategory_Basic, false)
		{

		}

		std::string strBlob;
		std::string strUri;
		std::string strMimeType;
		Json::Value jMeta;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

	private:
		int DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const;
		int DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol);
	};

	struct EmbeddedResource : public MCP::Message
	{
		EmbeddedResource()
			: Message(MessageType_EmbeddedResource, MessageCategory_Basic, false)
		{

		}

		std::string strType;
		TextResourceContents textResource;
		BlobResourceContents blobResource;
		Annotations annotations;
		Json::Value jMeta;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

	private:
		int DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const;
		int DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol);
	};

	struct Resource : public MCP::Message
	{
		Resource()
			: Message(MessageType_Resource, MessageCategory_Basic, false)
		{

		}

		std::string strName;
		std::string strUri;
		std::string strDescription;
		std::string strMimeType;
		int iSize{ 0 };
		std::string strTitle;
		std::vector<Icon> vecIcons;
		Annotations annotations;
		Json::Value jMeta;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

		// Writes _meta.ui while keeping unmodeled meta keys intact.
		void SetUiMeta(const UiResourceMeta& uiMeta);

	private:
		int DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const;
		int DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol);
	};

	// Builds the resources/read payload of an MCP App so that the contents
	// metadata always matches the resource declared in resources/list.
	int BuildUiResourceContents(const Resource& resource, const std::string& strHtml, TextResourceContents& contents);

	struct ResourceTemplate : public MCP::Message
	{
		ResourceTemplate()
			: Message(MessageType_ResourceTemplate, MessageCategory_Basic, false)
		{

		}

		std::string strName;
		std::string strUriTemplate;
		std::string strDescription;
		std::string strMimeType;
		std::string strTitle;
		std::vector<Icon> vecIcons;
		Annotations annotations;
		Json::Value jMeta;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

	private:
		int DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const;
		int DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol);
	};

	struct PromptArgument : public MCP::Message
	{
		PromptArgument()
			: Message(MessageType_PromptArgument, MessageCategory_Basic, false)
		{

		}

		std::string strName;
		std::string strDescription;
		bool bRequired{ false };

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct Prompt : public MCP::Message
	{
		Prompt()
			: Message(MessageType_Prompt, MessageCategory_Basic, false)
		{

		}

		std::string strName;
		std::string strDescription;
		std::vector<PromptArgument> vecArguments;
		std::string strTitle;
		std::vector<Icon> vecIcons;
		Json::Value jMeta;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

	private:
		int DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const;
		int DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol);
	};

	struct PromptMessage : public MCP::Message
	{
		PromptMessage()
			: Message(MessageType_PromptMessage, MessageCategory_Basic, false)
		{

		}

		std::string strRole; // "user" or "assistant"
		std::string strContentType; // "text", "image", or "resource"
		TextContent textContent;
		ImageContent imageContent;
		EmbeddedResource embeddedResource;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

	private:
		int DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const;
	};

	struct ProgressToken : public MCP::Message
	{
		ProgressToken()
			: Message(MessageType_ProgressToken, MessageCategory_Basic, false)
		{

		}

		DataType eTokenDataType{ DataType_Unknown };
		int iToken{ -1 };
		std::string strToken;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;

		inline bool IsEqual(const MCP::ProgressToken& rhs)
		{
			return eTokenDataType == rhs.eTokenDataType
				&& iToken == rhs.iToken
				&& strToken == rhs.strToken;
		}
	};

	struct PromptReference : public MCP::Message
	{
		PromptReference()
			: Message(MessageType_PromptReference, MessageCategory_Basic, false)
		{

		}

		std::string strType;
		std::string strName;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	struct ResourceReference : public MCP::Message
	{
		ResourceReference()
			: Message(MessageType_ResourceReference, MessageCategory_Basic, false)
		{

		}

		std::string strType;
		std::string strUri;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	// Client capabilities carried by modern request metadata and by the legacy
	// initialize handshake. Raw JSON is preserved so unknown sub-fields survive.
	struct ClientCapabilities : public MCP::Message
	{
	public:
		ClientCapabilities()
			: Message(MessageType_Unknown, MessageCategory_Basic, false)
		{
		}

		Json::Value jRaw;
		std::map<std::string, Json::Value> mapExtensions;

		bool HasValue() const;
		bool SupportsCapability(const std::string& strCapability) const;
		bool SupportsElicitationMode(const std::string& strMode) const;
		bool SupportsExtension(const std::string& strExtensionId) const;
		// Returns the extension settings object, or nullptr when the client did
		// not declare that extension.
		const Json::Value* FindExtensionSettings(const std::string& strExtensionId) const;
		// True when the extension was declared and lists strMimeType in its
		// non-empty string "mimeTypes" array.
		bool SupportsMimeType(const std::string& strExtensionId, const std::string& strMimeType) const;

		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
	};

	// Strong typed view of the modern per-request "_meta" object.
	struct MetaObject : public MCP::Message
	{
	public:
		MetaObject()
			: Message(MessageType_MetaObject, MessageCategory_Basic, false)
		{
		}

		std::string strProtocolVersion;
		Implementation clientInfo;
		ClientCapabilities clientCapabilities;
		bool bHasClientCapabilities{ false };
		std::string strLogLevel;
		Json::Value jExtra;

		bool HasValue() const;
		bool IsValid() const override;
		int DoSerialize(Json::Value& jMsg) const override;
		int DoDeserialize(const Json::Value& jMsg) override;
		// Protocol revision adaptation is SDK internal; callers use
		// VersionExtrasAccess in Internal/VersionExtras.h.
		friend struct VersionExtrasAccess;

	private:
		int DoSerializeVersionExtras(Json::Value& jMsg, const ProtocolContext& ctxProtocol) const;
		int DoDeserializeVersionExtras(const Json::Value& jMsg, const ProtocolContext& ctxProtocol);

	public:
		static bool IsReservedKey(const std::string& strKey);
	};

	// Object map containers used by MRTR. The SDK only transports application
	// defined payloads, so the entries stay raw JSON.
	struct InputRequests
	{
	public:
		void Set(const Json::Value& jItems);
		const Json::Value& Get() const;
		bool HasValue() const;
		void Clear();
		bool IsValid() const;
		bool Has(const std::string& strKey) const;
		const Json::Value& Get(const std::string& strKey) const;
		int DoSerialize(Json::Value& jMsg) const;
		int DoDeserialize(const Json::Value& jMsg);

	private:
		Json::Value m_jItems;
	};

	struct InputResponses
	{
	public:
		void Set(const Json::Value& jItems);
		const Json::Value& Get() const;
		bool HasValue() const;
		void Clear();
		bool IsValid() const;
		bool Has(const std::string& strKey) const;
		const Json::Value& Get(const std::string& strKey) const;
		int DoSerialize(Json::Value& jMsg) const;
		int DoDeserialize(const Json::Value& jMsg);

	private:
		Json::Value m_jItems;
	};
}