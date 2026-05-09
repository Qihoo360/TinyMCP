#pragma once
// To ensure good cross-platform compatibility, all code within the MCP namespace is written in standard C++.
// The use of platform-specific system APIs is prohibited unless necessary.

#include <string>
#include <chrono>
#include <json/json.h>

namespace MCP
{
	static constexpr const char* PROTOCOL_VER = "2024-11-05";
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

	static constexpr const char* CONST_TEXT = "text";
	static constexpr const char* CONST_IMAGE = "image";
	static constexpr const char* CONST_RESOURCE = "resource";
	static constexpr const char* CONST_REF_PROMPT = "ref/prompt";
	static constexpr const char* CONST_REF_RESOURCE = "ref/resource";

	static constexpr const char* ERROR_MESSAGE_PARSE_ERROR = u8"parse error";
	static constexpr const char* ERROR_MESSAGE_INVALID_REQUEST = u8"invalid request";
	static constexpr const char* ERROR_MESSAGE_METHOD_NOT_FOUND = u8"method not found";
	static constexpr const char* ERROR_MESSAGE_INVALID_PARAMS = u8"invalid params";
	static constexpr const char* ERROR_MESSAGE_INTERNAL_ERROR = u8"internal error";


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
