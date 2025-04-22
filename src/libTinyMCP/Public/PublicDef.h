#pragma once
// 为保证较好的跨平台特性，MCP命名空间的代码均使用标准c++编码
// 非必要情况下，禁止使用特定系统平台API

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
	

	static constexpr const char* METHOD_INITIALIZE = "initialize";
	static constexpr const char* METHOD_NOTIFICATION_INITIALIZED = "notifications/initialized";
	static constexpr const char* METHOD_NOTIFICATION_CANCELLED = "notifications/cancelled";
	static constexpr const char* METHOD_NOTIFICATION_PROGRESS = "notifications/progress";
	static constexpr const char* METHOD_TOOLS_LIST = "tools/list";
	static constexpr const char* METHOD_TOOLS_CALL = "tools/call";

	static constexpr const char* CONST_TEXT = "text";
	static constexpr const char* CONST_IMAGE = "image";
	static constexpr const char* CONST_RESOURCE = "resource";

	static constexpr const char* ERROR_MESSAGE_PARSE_ERROR = u8"parse error";
	static constexpr const char* ERROR_MESSAGE_INVALID_REQUEST = u8"invalid request";
	static constexpr const char* ERROR_MESSAGE_METHOD_NOT_FOUND = u8"method not found";
	static constexpr const char* ERROR_MESSAGE_INVALID_PARAMS = u8"invalid params";
	static constexpr const char* ERROR_MESSAGE_INTERNAL_ERROR = u8"internal error";


	// json rpc 2.0标准错误码
	static constexpr const int ERRNO_OK = 0;
	static constexpr const int ERRNO_PARSE_ERROR = -32700;
	static constexpr const int ERRNO_INVALID_REQUEST = -32600;
	static constexpr const int ERRNO_METHOD_NOT_FOUND = -32601;
	static constexpr const int ERRNO_INVALID_PARAMS = -32602;
	static constexpr const int ERRNO_INTERNAL_ERROR = -32603;

	// server自定义错误码
	static constexpr const int ERRNO_SERVER_ERROR_FIRST = -32000;
	static constexpr const int ERRNO_INVALID_RESPONSE = -32001;
	static constexpr const int ERRNO_INVALID_NOTIFICATION = -32002;
	static constexpr const int ERRNO_INTERNAL_INPUT_TERMINATE = -32003;
	static constexpr const int ERRNO_INTERNAL_INPUT_ERROR = -32004;
	static constexpr const int ERRNO_INTERNAL_OUTPUT_ERROR = -32005;
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
	};
}
