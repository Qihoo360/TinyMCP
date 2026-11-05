#pragma once
// To ensure good cross-platform compatibility, all code within the MCP namespace is written in standard C++.
// The use of platform-specific system APIs is prohibited unless necessary.

#include <string>
#include <chrono>
#include <bitset>
#include <set>
#include <vector>
#include <json/json.h>

namespace MCP
{
	static constexpr const char* PROTOCOL_VER = "2024-11-05";
	static constexpr const char* PROTOCOL_VER_2024_11_05 = "2024-11-05";
	static constexpr const char* PROTOCOL_VER_2026_07_28 = "2026-07-28";
	static constexpr const char* JSON_RPC_VER = "2.0";

	static constexpr const char* MSG_KEY_JSONRPC = "jsonrpc";
	static constexpr const char* MSG_KEY_ID = "id";
	static constexpr const char* MSG_KEY_METHOD = "method";
	static constexpr const char* MSG_KEY_PARAMS = "params";
	static constexpr const char* MSG_KEY_RESULT = "result";
	static constexpr const char* MSG_KEY_ERROR = "error";
	static constexpr const char* MSG_KEY_CODE = "code";
	static constexpr const char* MSG_KEY_MESSAGE = "message";
	static constexpr const char* MSG_KEY_DATA = "data";
	static constexpr const char* MSG_KEY_PROTOCOL_VERSION = "protocolVersion";	
	static constexpr const char* MSG_KEY_CLIENT_INFO = "clientInfo";
	static constexpr const char* MSG_KEY_NAME = "name";
	static constexpr const char* MSG_KEY_VERSION = "version";
	static constexpr const char* MSG_KEY_SERVER_INFO = "serverInfo";
	static constexpr const char* MSG_KEY_CAPABILITIES = "capabilities";
	static constexpr const char* MSG_KEY_PROMPTS = "prompts";
	static constexpr const char* MSG_KEY_RESOURCES = "resources";
	static constexpr const char* MSG_KEY_TOOLS = "tools";
	static constexpr const char* MSG_KEY_LISTCHANGED = "listChanged";
	static constexpr const char* MSG_KEY_SUBSCRIBE = "subscribe";
	static constexpr const char* MSG_KEY_CURSOR = "cursor";
	static constexpr const char* MSG_KEY_NEXT_CURSOR = "nextCursor";
	static constexpr const char* MSG_KEY_DESCRIPTION = "description";
	static constexpr const char* MSG_KEY_INPUT_SCHEMA = "inputSchema";
	static constexpr const char* MSG_KEY_ARGUMENTS = "arguments";
	static constexpr const char* MSG_KEY_IS_ERROR = "isError";
	static constexpr const char* MSG_KEY_CONTENT = "content";
	static constexpr const char* MSG_KEY_CONTENTS = "contents";
	static constexpr const char* MSG_KEY_TEXT = "text";
	static constexpr const char* MSG_KEY_TYPE = "type";
	static constexpr const char* MSG_KEY_MIMETYPE = "mimeType";
	static constexpr const char* MSG_KEY_URI = "uri";
	static constexpr const char* MSG_KEY_BLOB = "blob";
	static constexpr const char* MSG_KEY_RESOURCE = "resource";
	static constexpr const char* MSG_KEY_PROGRESS_TOKEN = "progressToken";
	static constexpr const char* MSG_KEY_META = "_meta";
	static constexpr const char* MSG_KEY_PROGRESS = "progress";
	static constexpr const char* MSG_KEY_TOTAL = "total";
	static constexpr const char* MSG_KEY_REQUEST_ID = "requestId";
	static constexpr const char* MSG_KEY_TOOL_CALL_ID = "toolCallId";
	static constexpr const char* MSG_KEY_SIZE = "size";
	static constexpr const char* MSG_KEY_RESOURCE_TEMPLATES = "resourceTemplates";
	static constexpr const char* MSG_KEY_URI_TEMPLATE = "uriTemplate";
	static constexpr const char* MSG_KEY_PROMPTS_LIST = "prompts";
	static constexpr const char* MSG_KEY_ROLE = "role";
	static constexpr const char* MSG_KEY_MESSAGES = "messages";
	static constexpr const char* MSG_KEY_REQUIRED = "required";
	static constexpr const char* MSG_KEY_REF = "ref";
	static constexpr const char* MSG_KEY_COMPLETION = "completion";
	static constexpr const char* MSG_KEY_VALUES = "values";
	static constexpr const char* MSG_KEY_HAS_MORE = "hasMore";
	static constexpr const char* MSG_KEY_VALUE = "value";
	static constexpr const char* MSG_KEY_ARGUMENT = "argument";
	static constexpr const char* MSG_KEY_EXPERIMENTAL = "experimental";
	static constexpr const char* MSG_KEY_SUPPORTS_STREAMING = "supportsStreaming";
	static constexpr const char* MSG_KEY_LOGGING = "logging";
	static constexpr const char* MSG_KEY_LEVEL = "level";
	static constexpr const char* MSG_KEY_LOGGER = "logger";

	// 2026-07-28 result and parameter keys.
	static constexpr const char* MSG_KEY_RESULT_TYPE = "resultType";
	static constexpr const char* MSG_KEY_INPUT_REQUESTS = "inputRequests";
	static constexpr const char* MSG_KEY_INPUT_RESPONSES = "inputResponses";
	static constexpr const char* MSG_KEY_REQUEST_STATE = "requestState";
	static constexpr const char* MSG_KEY_TTL_MS = "ttlMs";
	static constexpr const char* MSG_KEY_CACHE_SCOPE = "cacheScope";
	static constexpr const char* MSG_KEY_SUPPORTED_VERSIONS = "supportedVersions";
	static constexpr const char* MSG_KEY_SUPPORTED = "supported";
	static constexpr const char* MSG_KEY_INSTRUCTIONS = "instructions";
	static constexpr const char* MSG_KEY_NOTIFICATIONS = "notifications";
	static constexpr const char* MSG_KEY_TOOLS_LIST_CHANGED = "toolsListChanged";
	static constexpr const char* MSG_KEY_PROMPTS_LIST_CHANGED = "promptsListChanged";
	static constexpr const char* MSG_KEY_RESOURCES_LIST_CHANGED = "resourcesListChanged";
	static constexpr const char* MSG_KEY_RESOURCE_SUBSCRIPTIONS = "resourceSubscriptions";
	static constexpr const char* MSG_KEY_EXTENSIONS = "extensions";
	static constexpr const char* MSG_KEY_COMPLETIONS = "completions";
	static constexpr const char* MSG_KEY_LOG_LEVEL = "logLevel";
	static constexpr const char* MSG_KEY_MODE = "mode";
	static constexpr const char* MSG_KEY_REQUIRED_CAPABILITIES = "requiredCapabilities";
	static constexpr const char* EXTENSION_MCP_APPS_UI = "io.modelcontextprotocol/ui";
	static constexpr const char* MSG_KEY_TITLE = "title";
	static constexpr const char* MSG_KEY_ICONS = "icons";
	static constexpr const char* MSG_KEY_WEBSITE_URL = "websiteUrl";
	static constexpr const char* MSG_KEY_OUTPUT_SCHEMA = "outputSchema";
	static constexpr const char* MSG_KEY_STRUCTURED_CONTENT = "structuredContent";
	static constexpr const char* MSG_KEY_ANNOTATIONS = "annotations";
	static constexpr const char* MSG_KEY_TOOL_USE_ID = "toolUseId";
	static constexpr const char* MSG_KEY_UI = "ui";
	static constexpr const char* MSG_KEY_CSP = "csp";
	static constexpr const char* MSG_KEY_PERMISSIONS = "permissions";
	static constexpr const char* MSG_KEY_RESOURCE_URI = "resourceUri";

	// 2026-07-28 icon, annotation and client capability keys.
	static constexpr const char* MSG_KEY_SRC = "src";
	static constexpr const char* MSG_KEY_SIZES = "sizes";
	static constexpr const char* MSG_KEY_THEME = "theme";
	static constexpr const char* MSG_KEY_AUDIENCE = "audience";
	static constexpr const char* MSG_KEY_PRIORITY = "priority";
	static constexpr const char* MSG_KEY_LAST_MODIFIED = "lastModified";
	static constexpr const char* MSG_KEY_READ_ONLY_HINT = "readOnlyHint";
	static constexpr const char* MSG_KEY_DESTRUCTIVE_HINT = "destructiveHint";
	static constexpr const char* MSG_KEY_IDEMPOTENT_HINT = "idempotentHint";
	static constexpr const char* MSG_KEY_OPEN_WORLD_HINT = "openWorldHint";
	static constexpr const char* MSG_KEY_INPUT = "input";
	static constexpr const char* MSG_KEY_ROOTS = "roots";
	static constexpr const char* MSG_KEY_SAMPLING = "sampling";
	static constexpr const char* MSG_KEY_ELICITATION = "elicitation";
	static constexpr const char* MSG_KEY_FORM = "form";
	static constexpr const char* MSG_KEY_URL = "url";
	static constexpr const char* MSG_KEY_CONTEXT = "context";

	// 2026-07-28 per-request metadata keys.
	static constexpr const char* META_KEY_PROTOCOL_VERSION = "io.modelcontextprotocol/protocolVersion";
	static constexpr const char* META_KEY_CLIENT_CAPABILITIES = "io.modelcontextprotocol/clientCapabilities";
	static constexpr const char* META_KEY_CLIENT_INFO = "io.modelcontextprotocol/clientInfo";
	static constexpr const char* META_KEY_LOG_LEVEL = "io.modelcontextprotocol/logLevel";
	static constexpr const char* META_KEY_SERVER_INFO = "io.modelcontextprotocol/serverInfo";
	static constexpr const char* META_KEY_SUBSCRIPTION_ID = "io.modelcontextprotocol/subscriptionId";

	static constexpr const char* RESULT_TYPE_COMPLETE = "complete";
	static constexpr const char* RESULT_TYPE_INPUT_REQUIRED = "input_required";
	static constexpr const char* CACHE_SCOPE_PUBLIC = "public";
	static constexpr const char* CACHE_SCOPE_PRIVATE = "private";
	static constexpr const char* EXTENSION_UI = "io.modelcontextprotocol/ui";
	static constexpr const char* MIME_TYPE_MCP_APP = "text/html;profile=mcp-app";
	
	static constexpr const char* LOGGING_LEVEL_DEBUG = "debug";
	static constexpr const char* LOGGING_LEVEL_INFO = "info";
	static constexpr const char* LOGGING_LEVEL_NOTICE = "notice";
	static constexpr const char* LOGGING_LEVEL_WARNING = "warning";
	static constexpr const char* LOGGING_LEVEL_ERROR = "error";
	static constexpr const char* LOGGING_LEVEL_CRITICAL = "critical";
	static constexpr const char* LOGGING_LEVEL_ALERT = "alert";
	static constexpr const char* LOGGING_LEVEL_EMERGENCY = "emergency";
	
	static constexpr const char* CONST_USER = "user";
	static constexpr const char* CONST_ASSISTANT = "assistant";

	static constexpr const char* METHOD_PING = "ping";
	static constexpr const char* METHOD_INITIALIZE = "initialize";
	static constexpr const char* METHOD_NOTIFICATION_INITIALIZED = "notifications/initialized";
	static constexpr const char* METHOD_NOTIFICATION_CANCELLED = "notifications/cancelled";
	static constexpr const char* METHOD_NOTIFICATION_PROGRESS = "notifications/progress";
	static constexpr const char* METHOD_TOOLS_LIST = "tools/list";
	static constexpr const char* METHOD_TOOLS_CALL = "tools/call";
	static constexpr const char* METHOD_RESOURCES_LIST = "resources/list";
	static constexpr const char* METHOD_RESOURCES_READ = "resources/read";
	static constexpr const char* METHOD_RESOURCES_SUBSCRIBE = "resources/subscribe";
	static constexpr const char* METHOD_RESOURCES_UNSUBSCRIBE = "resources/unsubscribe";
	static constexpr const char* METHOD_RESOURCES_TEMPLATES_LIST = "resources/templates/list";
	static constexpr const char* METHOD_NOTIFICATION_RESOURCES_LIST_CHANGED = "notifications/resources/list_changed";
	static constexpr const char* METHOD_NOTIFICATION_RESOURCES_UPDATED = "notifications/resources/updated";
	static constexpr const char* METHOD_PROMPTS_LIST = "prompts/list";
	static constexpr const char* METHOD_PROMPTS_GET = "prompts/get";
	static constexpr const char* METHOD_NOTIFICATION_PROMPTS_LIST_CHANGED = "notifications/prompts/list_changed";
	static constexpr const char* METHOD_COMPLETION_COMPLETE = "completion/complete";
	static constexpr const char* METHOD_LOGGING_SET_LEVEL = "logging/setLevel";
	static constexpr const char* METHOD_NOTIFICATION_MESSAGE = "notifications/message";
	static constexpr const char* METHOD_SERVER_DISCOVER = "server/discover";
	static constexpr const char* METHOD_SUBSCRIPTIONS_LISTEN = "subscriptions/listen";
	static constexpr const char* METHOD_NOTIFICATION_SUBSCRIPTIONS_ACKNOWLEDGED = "notifications/subscriptions/acknowledged";
	static constexpr const char* METHOD_NOTIFICATION_TOOLS_LIST_CHANGED = "notifications/tools/list_changed";
	static constexpr const char* METHOD_SAMPLING_CREATE_MESSAGE = "sampling/createMessage";
	static constexpr const char* METHOD_ROOTS_LIST = "roots/list";
	static constexpr const char* METHOD_ELICITATION_CREATE = "elicitation/create";

	static constexpr const char* CONST_TEXT = "text";
	static constexpr const char* CONST_IMAGE = "image";
	static constexpr const char* CONST_RESOURCE = "resource";
	static constexpr const char* CONST_AUDIO = "audio";
	static constexpr const char* CONST_RESOURCE_LINK = "resource_link";
	static constexpr const char* CONST_TOOL_USE = "tool_use";
	static constexpr const char* CONST_TOOL_RESULT = "tool_result";
	static constexpr const char* CONST_REF_PROMPT = "ref/prompt";
	static constexpr const char* CONST_REF_RESOURCE = "ref/resource";

	static constexpr const char* ERROR_MESSAGE_PARSE_ERROR = u8"parse error";
	static constexpr const char* ERROR_MESSAGE_INVALID_REQUEST = u8"invalid request";
	static constexpr const char* ERROR_MESSAGE_METHOD_NOT_FOUND = u8"method not found";
	static constexpr const char* ERROR_MESSAGE_INVALID_PARAMS = u8"invalid params";
	static constexpr const char* ERROR_MESSAGE_INTERNAL_ERROR = u8"internal error";
	static constexpr const char* ERROR_MESSAGE_UNSUPPORTED_PROTOCOL_VERSION = u8"unsupported protocol version";
	static constexpr const char* ERROR_MESSAGE_MISSING_REQUIRED_CLIENT_CAPABILITY = u8"missing required client capability";


	// JSON-RPC 2.0 Standard Error Codes
	static constexpr const int ERRNO_OK = 0;
	static constexpr const int ERRNO_PARSE_ERROR = -32700;
	static constexpr const int ERRNO_INVALID_REQUEST = -32600;
	static constexpr const int ERRNO_METHOD_NOT_FOUND = -32601;
	static constexpr const int ERRNO_INVALID_PARAMS = -32602;
	static constexpr const int ERRNO_INTERNAL_ERROR = -32603;

	// Server-side custom error codes
	static constexpr const int ERRNO_SERVER_ERROR_FIRST = -32000;
	static constexpr const int ERRNO_HEADER_MISMATCH = -32001;
	static constexpr const int ERRNO_RESOURCE_NOT_FOUND = -32002;
	static constexpr const int ERRNO_HEADER_MISMATCH_MODERN = -32020;
	static constexpr const int ERRNO_MISSING_REQUIRED_CLIENT_CAPABILITY = -32021;
	static constexpr const int ERRNO_UNSUPPORTED_PROTOCOL_VERSION = -32022;
	static constexpr const int ERRNO_INVALID_RESPONSE = -32094;
	static constexpr const int ERRNO_INVALID_NOTIFICATION = -32095;
	static constexpr const int ERRNO_INTERNAL_INPUT_TERMINATE = -32096;
	static constexpr const int ERRNO_INTERNAL_INPUT_ERROR = -32097;
	static constexpr const int ERRNO_INTERNAL_OUTPUT_ERROR = -32098;
	static constexpr const int ERRNO_SERVER_ERROR_LAST = -32099;

	enum DataType
	{
		DataType_Unknown,
		DataType_String,
		DataType_Integer,
	};

	enum MessageCategory
	{
		MessageCategory_Unknown,
		MessageCategory_Basic,
		MessageCategory_Request,
		MessageCategory_Response,
		MessageCategory_Notification,
	};

	enum MessageType
	{
		MessageType_Unknown,
		MessageType_RequestId,
		MessageType_Implementation,
		MessageType_PingRequest,
		MessageType_InitializeRequest,
		MessageType_InitializeResult,
		MessageType_InitializedNotification,
		MessageType_ServerCapabilities,
		MessageType_Prompts,
		MessageType_Resources,
		MessageType_Tools,
		MessageType_ListToolsRequest,
		MessageType_ListToolsResult,
		MessageType_Tool,
		MessageType_CallToolRequest,
		MessageType_CallToolResult,
		MessageType_TextContent,
		MessageType_ImageContent,
		MessageType_EmbeddedResource,
		MessageType_TextResourceContents,
		MessageType_BlobResourceContents,
		MessageType_CancelledNotification,
		MessageType_ProgressToken,
		MessageType_ProgressNotification,
		MessageType_ErrorResponse,
		MessageType_EmptyResponse,
		MessageType_ListResourcesRequest,
		MessageType_ListResourcesResult,
		MessageType_ReadResourceRequest,
		MessageType_ReadResourceResult,
		MessageType_SubscribeResourceRequest,
		MessageType_UnsubscribeResourceRequest,
		MessageType_ListResourceTemplatesRequest,
		MessageType_ListResourceTemplatesResult,
		MessageType_ResourceListChangedNotification,
		MessageType_ResourceUpdatedNotification,
		MessageType_Resource,
		MessageType_ResourceTemplate,
		MessageType_PromptArgument,
		MessageType_Prompt,
		MessageType_PromptMessage,
		MessageType_ListPromptsRequest,
		MessageType_ListPromptsResult,
		MessageType_GetPromptRequest,
		MessageType_GetPromptResult,
		MessageType_PromptListChangedNotification,
		MessageType_ToolListChangedNotification,
		MessageType_CompleteRequest,
		MessageType_CompleteResult,
		MessageType_PromptReference,
		MessageType_ResourceReference,
		MessageType_Completion,
		MessageType_Experimental,
		MessageType_SetLevelRequest,
		MessageType_LoggingLevel,
		MessageType_Logging,
		MessageType_LoggingMessageNotification,
		MessageType_DiscoverRequest,
		MessageType_DiscoverResult,
		MessageType_SubscriptionsListenRequest,
		MessageType_SubscriptionsListenResult,
		MessageType_SubscriptionsAcknowledgedNotification,
		MessageType_MetaObject,
		MessageType_Icon,
		MessageType_AudioContent,
		MessageType_ResourceLink,
		MessageType_ToolUseContent,
		MessageType_ToolResultContent,
	};

	// Interaction model of a protocol revision. This is the only architectural
	// extension point: adding a version number must not add a new era.
	enum ProtocolEra
	{
		Era_Handshake,
		Era_Stateless,
	};

	// Feature flags gate every version-specific field. Business logic must call
	// ProtocolContext::Supports() instead of comparing version strings.
	enum ProtocolFeature
	{
		Feature_ResultMeta,
		Feature_EntityMeta,
		Feature_ImplementationExtras,
		Feature_ResultType,
		Feature_Discover,
		Feature_Caching,
		Feature_Mrtr,
		Feature_ListenSubscriptions,
		Feature_Extensions,
		Feature_CompletionsCapability,
		Feature_ToolTitle,
		Feature_ToolAnnotations,
		Feature_StructuredContent,
		Feature_ContentAudio,
		Feature_ContentResourceLink,
		Feature_ContentToolUse,
		Feature_ContentAnnotations,
		Feature_LogLevelInMeta,
		Feature_Count,
	};

	struct ProtocolProfile
	{
		std::string strVersion;
		ProtocolEra eEra{ Era_Handshake };
		std::bitset<Feature_Count> bitsFeatures;
		std::set<std::string> setSupportedMethods;
		int iResourceNotFoundCode{ ERRNO_RESOURCE_NOT_FOUND };

		bool Supports(ProtocolFeature eFeature) const
		{
			return bitsFeatures.test(static_cast<size_t>(eFeature));
		}

		bool SupportsMethod(const std::string& strMethod) const
		{
			return setSupportedMethods.find(strMethod) != setSupportedMethods.end();
		}
	};

	// Version context carried by every message. The default construction always
	// resolves the built-in 2024-11-05 handshake profile so objects created by
	// existing code keep their legacy behavior.
	struct ProtocolContext
	{
		std::string strVersion;
		const ProtocolProfile* pProfile{ nullptr };

		ProtocolContext();
		static ProtocolContext Invalid(const std::string& strRequested);

		bool IsValid() const;
		bool IsStateless() const;
		bool Supports(ProtocolFeature eFeature) const;
	};

	// Read-only version table. Version differences are declared as profile data;
	// the order of the profile list defines version priority.
	class CProtocolRegistry
	{
	public:
		explicit CProtocolRegistry(const std::vector<ProtocolProfile>& vecProfiles);

		static const CProtocolRegistry& GetGlobalRegistry();

		const ProtocolProfile* Find(const std::string& strVersion) const;
		const ProtocolProfile* GetDefaultHandshakeProfile() const;
		const ProtocolProfile* GetLatestStatelessProfile() const;
		std::vector<std::string> GetSupportedVersions() const;
		std::vector<std::string> GetSupportedVersions(ProtocolEra eEra) const;

	private:
		std::vector<ProtocolProfile> m_vecProfiles;
	};

	enum class LoggingLevelValue
	{
		LoggingLevel_Unknown,
		LoggingLevel_Debug,
		LoggingLevel_Info,
		LoggingLevel_Notice,
		LoggingLevel_Warning,
		LoggingLevel_Error,
		LoggingLevel_Critical,
		LoggingLevel_Alert,
		LoggingLevel_Emergency,
	};
}
