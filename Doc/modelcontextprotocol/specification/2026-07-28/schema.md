> ## Documentation Index
> Fetch the complete documentation index at: https://modelcontextprotocol.io/llms.txt
> Use this file to discover all available pages before exploring further.

# Schema Reference

## JSON-RPC

### `JSONRPCErrorResponse`

interface JSONRPCErrorResponse \{\
  [jsonrpc](#jsonrpcerrorresponse-jsonrpc): "2.0";\
  [id](#jsonrpcerrorresponse-id)?: [RequestId](#requestid);\
  [error](#jsonrpcerrorresponse-error): [Error](#error);\
}A response to a request that indicates an error occurred.  jsonrpc: "2.0"[Permalink](#jsonrpcerrorresponse-jsonrpc)  id?: RequestId[Permalink](#jsonrpcerrorresponse-id)  error: Error[Permalink](#jsonrpcerrorresponse-error)&#x20;

### `JSONRPCMessage`

JSONRPCMessage: [JSONRPCRequest](#jsonrpcrequest) | [JSONRPCNotification](#jsonrpcnotification) | [JSONRPCResponse](#jsonrpcresponse)Refers to any valid JSON-RPC object that can be decoded off the wire, or encoded to be sent.&#x20;

### `JSONRPCNotification`

interface JSONRPCNotification \{\
  [method](#jsonrpcnotification-method): string;\
  [params](#jsonrpcnotification-params)?: \{ \[key: string]: any };\
  [jsonrpc](#jsonrpcnotification-jsonrpc): "2.0";\
}A notification which does not expect a response.  method: string[Permalink](#jsonrpcnotification-method)  Inherited from Notification.method params?: \{ \[key: string]: any }[Permalink](#jsonrpcnotification-params)  Inherited from Notification.params jsonrpc: "2.0"[Permalink](#jsonrpcnotification-jsonrpc)&#x20;

### `JSONRPCRequest`

interface JSONRPCRequest \{\
  [method](#jsonrpcrequest-method): string;\
  [params](#jsonrpcrequest-params)?: \{ \[key: string]: any };\
  [jsonrpc](#jsonrpcrequest-jsonrpc): "2.0";\
  [id](#jsonrpcrequest-id): [RequestId](#requestid);\
}A request that expects a response.  method: string[Permalink](#jsonrpcrequest-method)  Inherited from Request.method params?: \{ \[key: string]: any }[Permalink](#jsonrpcrequest-params)  Inherited from Request.params jsonrpc: "2.0"[Permalink](#jsonrpcrequest-jsonrpc)  id: RequestId[Permalink](#jsonrpcrequest-id)&#x20;

### `JSONRPCResponse`

JSONRPCResponse: [JSONRPCResultResponse](#jsonrpcresultresponse) | [JSONRPCErrorResponse](#jsonrpcerrorresponse)A response to a request, containing either the result or error.&#x20;

### `JSONRPCResultResponse`

interface JSONRPCResultResponse \{\
  [jsonrpc](#jsonrpcresultresponse-jsonrpc): "2.0";\
  [id](#jsonrpcresultresponse-id): [RequestId](#requestid);\
  [result](#jsonrpcresultresponse-result): [Result](#result);\
}A successful (non-error) response to a request.  jsonrpc: "2.0"[Permalink](#jsonrpcresultresponse-jsonrpc)  id: RequestId[Permalink](#jsonrpcresultresponse-id)  result: Result[Permalink](#jsonrpcresultresponse-result)&#x20;

## Common Types

### `Annotations`

interface Annotations \{\
  [audience](#annotations-audience)?: [Role](#role)\[];\
  [priority](#annotations-priority)?: number;\
  [lastModified](#annotations-lastmodified)?: string;\
}Optional annotations for the client. The client can use annotations to inform how objects are used or displayed  audience?: Role\[][Permalink](#annotations-audience) Describes who the intended audience of this object or data is. It can include multiple entries to indicate content useful for multiple audiences (e.g., `["user", "assistant"]`).  priority?: number[Permalink](#annotations-priority) Describes how important this data is for operating the server. A value of 1 means "most important," and indicates that the data is
effectively required, while 0 means "least important," and indicates that
the data is entirely optional.  lastModified?: string[Permalink](#annotations-lastmodified) The moment the resource was last modified, as an ISO 8601 formatted string. Should be an ISO 8601 formatted string (e.g., "2025-01-12T15:00:58Z"). Examples: last activity timestamp in an open file, timestamp when the resource
was attached, etc.&#x20;

### `Cursor`

Cursor: stringAn opaque token used to represent a cursor for pagination.&#x20;

### `EmptyResult`

EmptyResult: [Result](#result)A result that indicates success but carries no data.&#x20;

### `Icon`

interface Icon \{\
  [src](#icon-src): string;\
  [mimeType](#icon-mimetype)?: string;\
  [sizes](#icon-sizes)?: string\[];\
  [theme](#icon-theme)?: "light" | "dark";\
}An optionally-sized icon that can be displayed in a user interface.  src: string[Permalink](#icon-src) A standard URI pointing to an icon resource. May be an HTTP/HTTPS URL or a `data:` URI with Base64-encoded image data. Consumers SHOULD take steps to ensure URLs serving icons are from the
same domain as the client/server or a trusted domain. Consumers SHOULD take appropriate precautions when consuming SVGs as they can contain
executable JavaScript.  mimeType?: string[Permalink](#icon-mimetype) Optional MIME type override if the source MIME type is missing or generic.
For example: `"image/png"`, `"image/jpeg"`, or `"image/svg+xml"`.  sizes?: string\[][Permalink](#icon-sizes) Optional array of strings that specify sizes at which the icon can be used.
Each string should be in WxH format (e.g., `"48x48"`, `"96x96"`) or `"any"` for scalable formats like SVG. If not provided, the client should assume that the icon can be used at any size.  theme?: "light" | "dark"[Permalink](#icon-theme) Optional specifier for the theme this icon is designed for. `"light"` indicates
the icon is designed to be used with a light background, and `"dark"` indicates
the icon is designed to be used with a dark background. If not provided, the client should assume the icon can be used with any theme.&#x20;

### `InputResponseRequestParams`

interface InputResponseRequestParams \{\
  [\_meta](#inputresponserequestparams-_meta): [RequestMetaObject](#requestmetaobject);\
  [inputResponses](#inputresponserequestparams-inputresponses)?: [InputResponses](#inputresponses);\
  [requestState](#inputresponserequestparams-requeststate)?: string;\
}Common params for any request.  \_meta: RequestMetaObject[Permalink](#inputresponserequestparams-_meta)  Inherited from [RequestParams](#requestparams).[\_meta](#requestparams-_meta) inputResponses?: InputResponses[Permalink](#inputresponserequestparams-inputresponses)  requestState?: string[Permalink](#inputresponserequestparams-requeststate)&#x20;

### `JSONArray`

JSONArray: [JSONValue](#jsonvalue)\[]

### `JSONObject`

JSONObject: \{ \[key: string]: [JSONValue](#jsonvalue) } Type Declaration <ul> <li> \[key: string]: [JSONValue](#jsonvalue)</li></ul>

### `JSONValue`

JSONValue: string | number | boolean | null | [JSONObject](#jsonobject) | [JSONArray](#jsonarray)

### `LoggingLevel`

LoggingLevel:\
  | "debug"\
  | "info"\
  | "notice"\
  | "warning"\
  | "error"\
  | "critical"\
  | "alert"\
  | "emergency"The severity of a log message. These map to syslog message severities, as specified in RFC-5424: [[https://datatracker.ietf.org/doc/html/rfc5424#section-6.2.1](https://datatracker.ietf.org/doc/html/rfc5424#section-6.2.1)](https://datatracker.ietf.org/doc/html/rfc5424#section-6.2.1)   DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.&#x20;

### `MetaObject`

MetaObject: Record\<string, unknown>Represents the contents of a `_meta` field, which clients and servers use to attach additional metadata to their interactions. Certain key names are reserved by MCP for protocol-level metadata; implementations MUST NOT make assumptions about values at these keys. Additionally, specific schema definitions may reserve particular names for purpose-specific metadata, as declared in those definitions. Valid keys have two segments: **Prefix:** <ul> <li>Optional — if specified, MUST be a series of *labels* separated by dots (`.`), followed by a slash (`/`).</li> <li>Labels MUST start with a letter and end with a letter or digit. Interior characters may be letters, digits, or hyphens (`-`).</li> <li>Implementations SHOULD use reverse DNS notation (e.g., `com.example/` rather than `example.com/`).</li> <li>Any prefix where the second label is `modelcontextprotocol` or `mcp` is **reserved** for MCP use. For example: `io.modelcontextprotocol/`, `dev.mcp/`, `org.modelcontextprotocol.api/`, and `com.mcp.tools/` are all reserved. However, `com.example.mcp/` is NOT reserved, as the second label is `example`.</li> </ul> **Name:** <ul> <li>Unless empty, MUST start and end with an alphanumeric character (`[a-z0-9A-Z]`).</li> <li>Interior characters may be alphanumeric, hyphens (`-`), underscores (`_`), or dots (`.`).</li> </ul>   See[General fields: `_meta`](https://modelcontextprotocol.io/specification/2026-07-28/basic/index#meta) for more details.&#x20;

### `NotificationMetaObject`

interface NotificationMetaObject \{\
  ["io.modelcontextprotocol/subscriptionId"](#notificationmetaobject-iomodelcontextprotocolsubscriptionid)?: [RequestId](#requestid);\
  \[key: string]: unknown;\
}Extends [MetaObject](#metaobject) with additional notification-specific fields. All key naming rules from `MetaObject` apply.   See<ul> <li>[MetaObject](#metaobject) for key naming rules and reserved prefixes.</li> <li>[General fields: `_meta`](https://modelcontextprotocol.io/specification/2026-07-28/basic/index#meta) for more details.</li> </ul>  "io.modelcontextprotocol/subscriptionId"?: RequestId[Permalink](#notificationmetaobject-iomodelcontextprotocolsubscriptionid) Identifies the subscription stream a notification was delivered on. The
server MUST include this key on every notification delivered via a [subscriptions/listen](#subscriptionslistenrequest) stream, so the
client can correlate the notification with the originating subscription.
The key is absent on notifications not delivered via a subscription
stream (e.g. progress notifications for an in-flight request), which is
why it is optional here. The value is the JSON-RPC ID of the `subscriptions/listen` request that
opened the stream.&#x20;

### `NotificationParams`

interface NotificationParams \{\
  [\_meta](#notificationparams-_meta)?: [NotificationMetaObject](#notificationmetaobject);\
}Common params for any notification.  \_meta?: NotificationMetaObject[Permalink](#notificationparams-_meta)&#x20;

### `PaginatedRequestParams`

interface PaginatedRequestParams \{\
  [\_meta](#paginatedrequestparams-_meta): [RequestMetaObject](#requestmetaobject);\
  [cursor](#paginatedrequestparams-cursor)?: string;\
}Common params for paginated requests.  <details> <summary>Example: List request with cursor[Permalink](#paginatedrequestparams-example-list-request-with-cursor)</summary><pre id="paginatedrequestparams-example-list-request-with-cursor">`{  "_meta": {    "io.modelcontextprotocol/protocolVersion": "2026-07-28",    "io.modelcontextprotocol/clientInfo": {      "name": "ExampleClient",      "version": "1.0.0"    },    "io.modelcontextprotocol/clientCapabilities": {}  },  "cursor": "eyJwYWdlIjogMn0="} `Copy</pre> </details> \_meta: RequestMetaObject[Permalink](#paginatedrequestparams-_meta)  Inherited from [RequestParams](#requestparams).[\_meta](#requestparams-_meta) cursor?: string[Permalink](#paginatedrequestparams-cursor) An opaque token representing the current pagination position.
If provided, the server should return results starting after this cursor.&#x20;

### `ProgressToken`

ProgressToken: string | numberA progress token, used to associate progress notifications with the original request.&#x20;

### `RequestId`

RequestId: string | numberA uniquely identifying ID for a request in JSON-RPC.&#x20;

### `RequestMetaObject`

interface RequestMetaObject \{\
  [progressToken](#requestmetaobject-progresstoken)?: [ProgressToken](#progresstoken);\
  ["io.modelcontextprotocol/protocolVersion"](#requestmetaobject-iomodelcontextprotocolprotocolversion): string;\
  ["io.modelcontextprotocol/clientInfo"](#requestmetaobject-iomodelcontextprotocolclientinfo)?: [Implementation](#implementation);\
  ["io.modelcontextprotocol/clientCapabilities"](#requestmetaobject-iomodelcontextprotocolclientcapabilities): [ClientCapabilities](#clientcapabilities);\
  ["io.modelcontextprotocol/logLevel"](#requestmetaobject-iomodelcontextprotocolloglevel)?: [LoggingLevel](#logginglevel);\
  \[key: string]: unknown;\
}Extends [MetaObject](#metaobject) with additional request-specific fields. All key naming rules from `MetaObject` apply.   See<ul> <li>[MetaObject](#metaobject) for key naming rules and reserved prefixes.</li> <li>[General fields: `_meta`](https://modelcontextprotocol.io/specification/2026-07-28/basic/index#meta) for more details.</li> </ul>  progressToken?: ProgressToken[Permalink](#requestmetaobject-progresstoken) If specified, the caller is requesting out-of-band progress notifications for this request (as represented by [notifications/progress](#progressnotification)). The value of this parameter is an opaque token that will be attached to any subsequent notifications. The receiver is not obligated to provide these notifications.  "io.modelcontextprotocol/protocolVersion": string[Permalink](#requestmetaobject-iomodelcontextprotocolprotocolversion) The MCP Protocol Version being used for this request. Required. For the HTTP transport, this value MUST match the `MCP-Protocol-Version`
header; otherwise the server MUST return a `400 Bad Request`. If the
server does not support the requested version, it MUST return an [UnsupportedProtocolVersionError](#unsupportedprotocolversionerror).  "io.modelcontextprotocol/clientInfo"?: Implementation[Permalink](#requestmetaobject-iomodelcontextprotocolclientinfo) Identifies the client software making the request. Clients SHOULD
include this field on every request unless specifically configured not
to do so. The [Implementation](#implementation) schema requires `name` and `version`; other
fields are optional. The value is self-reported by the client and is not verified by the
protocol. It is intended for display, logging, and debugging. Servers
SHOULD NOT use it to change their behavior, and SHOULD NOT rely on it for
security decisions.  "io.modelcontextprotocol/clientCapabilities": ClientCapabilities[Permalink](#requestmetaobject-iomodelcontextprotocolclientcapabilities) The client's capabilities for this specific request. Required. Capabilities are declared per-request rather than once at initialization;
an empty object means the client supports no optional capabilities.
Servers MUST NOT infer capabilities from prior requests.  "io.modelcontextprotocol/logLevel"?: LoggingLevel[Permalink](#requestmetaobject-iomodelcontextprotocolloglevel) The desired log level for this request. Optional. If absent, the server MUST NOT send any [notifications/message](#loggingmessagenotification)
notifications for this request. The client opts in to log messages by
explicitly setting a level. Replaces the former `logging/setLevel` RPC.    DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.&#x20;

### `RequestParams`

interface RequestParams \{\
  [\_meta](#requestparams-_meta): [RequestMetaObject](#requestmetaobject);\
}Common params for any request.  \_meta: RequestMetaObject[Permalink](#requestparams-_meta)&#x20;

### `Result`

interface Result \{\
  [\_meta](#result-_meta)?: [ResultMetaObject](#resultmetaobject);\
  [resultType](#result-resulttype): string;\
  \[key: string]: unknown;\
}Common result fields.  \_meta?: ResultMetaObject[Permalink](#result-_meta)  resultType: string[Permalink](#result-resulttype) Indicates the type of the result, which allows the client to determine
how to parse the result object. Servers implementing this protocol version MUST include this field.
For backward compatibility, when a client receives a result from a
server implementing an earlier protocol version (which does not include `resultType`), the client MUST treat the absent field as `"complete"`.&#x20;

### `ResultMetaObject`

interface ResultMetaObject \{\
  ["io.modelcontextprotocol/serverInfo"](#resultmetaobject-iomodelcontextprotocolserverinfo)?: [Implementation](#implementation);\
  \[key: string]: unknown;\
}Extends [MetaObject](#metaobject) with additional result-specific fields. All key naming rules from `MetaObject` apply.   See<ul> <li>[MetaObject](#metaobject) for key naming rules and reserved prefixes.</li> <li>[General fields: `_meta`](https://modelcontextprotocol.io/specification/2026-07-28/basic/index#meta) for more details.</li> </ul>  "io.modelcontextprotocol/serverInfo"?: Implementation[Permalink](#resultmetaobject-iomodelcontextprotocolserverinfo) Identifies the server software producing the response. Servers SHOULD
include this field on every response unless specifically configured not
to do so. The [Implementation](#implementation) schema requires `name` and `version`; other
fields are optional. The value is self-reported by the server and is not verified by the
protocol. It is intended for display, logging, and debugging. Clients
SHOULD NOT use it to change their behavior, and SHOULD NOT rely on it for
security decisions.&#x20;

### `ResultType`

ResultType: "complete" | "input\_required" | stringIndicates the type of a [Result](#result) object, allowing the client to
determine how to parse the response. complete - the request completed successfully and the result contains the final content.
input\_required - the request requires additional input and the result contains an [InputRequiredResult](#inputrequiredresult) object with instructions for the client to provide additional input before retrying the original request.&#x20;

### `Role`

Role: "user" | "assistant"The sender or recipient of messages and data in a conversation.&#x20;

## Errors

### `Error`

interface Error \{\
  [code](#error-code): number;\
  [message](#error-message): string;\
  [data](#error-data)?: unknown;\
} code: number[Permalink](#error-code) The error type that occurred.  message: string[Permalink](#error-message) A short description of the error. The message SHOULD be limited to a concise single sentence.  data?: unknown[Permalink](#error-data) Additional information about the error. The value of this member is defined by the sender (e.g. detailed error information, nested errors etc.).&#x20;

### `HEADER_MISMATCH`

HEADER\_MISMATCH: -32020Error code returned when the HTTP headers of a request do not match the
corresponding values in the request body, or required headers are
missing or malformed.&#x20;

### `HeaderMismatchError`

interface HeaderMismatchError \{\
  [jsonrpc](#headermismatcherror-jsonrpc): "2.0";\
  [id](#headermismatcherror-id)?: [RequestId](#requestid);\
  [error](#headermismatcherror-error): [Error](#error) & \{ code: -32020 };\
}Returned when a server rejects a request because the values in the HTTP
headers do not match the corresponding values in the request body, or
because required headers are missing or malformed. For HTTP, the response
status code MUST be `400 Bad Request`.  <details> <summary>Example: Header mismatch[Permalink](#headermismatcherror-example-header-mismatch)</summary><pre id="headermismatcherror-example-header-mismatch">`{  "jsonrpc": "2.0",  "id": 1,  "error": {    "code": -32020,    "message": "Header mismatch: Mcp-Name header value 'foo' does not match body value 'bar'"  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#headermismatcherror-jsonrpc)  Inherited from [JSONRPCErrorResponse](#jsonrpcerrorresponse).[jsonrpc](#jsonrpcerrorresponse-jsonrpc) id?: RequestId[Permalink](#headermismatcherror-id)  Inherited from [JSONRPCErrorResponse](#jsonrpcerrorresponse).[id](#jsonrpcerrorresponse-id) error: Error & \{ code: -32020 }[Permalink](#headermismatcherror-error)&#x20;

### `InternalError`

interface InternalError \{\
  [message](#internalerror-message): string;\
  [data](#internalerror-data)?: unknown;\
  [code](#internalerror-code): -32603;\
}A JSON-RPC error indicating that an internal error occurred on the receiver. This error is returned when the receiver encounters an unexpected condition that prevents it from fulfilling the request.   See[JSON-RPC 2.0 Error Object](https://www.jsonrpc.org/specification#error_object)  <details> <summary>Example: Unexpected error[Permalink](#internalerror-example-unexpected-error)</summary><pre id="internalerror-example-unexpected-error">`{  "code": -32603,  "message": "Internal error"} `Copy</pre> </details> message: string[Permalink](#internalerror-message) A short description of the error. The message SHOULD be limited to a concise single sentence.  Inherited from [Error](#error).[message](#error-message) data?: unknown[Permalink](#internalerror-data) Additional information about the error. The value of this member is defined by the sender (e.g. detailed error information, nested errors etc.).  Inherited from [Error](#error).[data](#error-data) code: -32603[Permalink](#internalerror-code) The error type that occurred.  Overrides [Error](#error).[code](#error-code)

### `InvalidParamsError`

interface InvalidParamsError \{\
  [message](#invalidparamserror-message): string;\
  [data](#invalidparamserror-data)?: unknown;\
  [code](#invalidparamserror-code): -32602;\
}A JSON-RPC error indicating that the method parameters are invalid or malformed. In MCP, this error is returned in various contexts when request parameters fail validation: <ul> <li>**Tools**: Unknown tool name or invalid tool arguments</li> <li>**Prompts**: Unknown prompt name or missing required arguments</li> <li>**Pagination**: Invalid or expired cursor values</li> <li>**Logging**: Invalid log level</li> <li>**Elicitation**: Server requests an elicitation mode not declared in client capabilities</li> <li>**Sampling**: Missing tool result or tool results mixed with other content</li> </ul>   See[JSON-RPC 2.0 Error Object](https://www.jsonrpc.org/specification#error_object)  <details> <summary>Example: Unknown tool[Permalink](#invalidparamserror-example-unknown-tool)</summary><pre id="invalidparamserror-example-unknown-tool">`{  "code": -32602,  "message": "Unknown tool: invalid_tool_name"} `Copy</pre> </details> <details> <summary>Example: Invalid tool arguments[Permalink](#invalidparamserror-example-invalid-tool-arguments)</summary><pre id="invalidparamserror-example-invalid-tool-arguments">`{  "code": -32602,  "message": "Invalid arguments for tool calculate: Missing required property 'expression'"} `Copy</pre> </details> <details> <summary>Example: Unknown prompt[Permalink](#invalidparamserror-example-unknown-prompt)</summary><pre id="invalidparamserror-example-unknown-prompt">`{  "code": -32602,  "message": "Unknown prompt: invalid_prompt_name"} `Copy</pre> </details> <details> <summary>Example: Invalid cursor[Permalink](#invalidparamserror-example-invalid-cursor)</summary><pre id="invalidparamserror-example-invalid-cursor">`{  "code": -32602,  "message": "Invalid cursor"} `Copy</pre> </details> message: string[Permalink](#invalidparamserror-message) A short description of the error. The message SHOULD be limited to a concise single sentence.  Inherited from [Error](#error).[message](#error-message) data?: unknown[Permalink](#invalidparamserror-data) Additional information about the error. The value of this member is defined by the sender (e.g. detailed error information, nested errors etc.).  Inherited from [Error](#error).[data](#error-data) code: -32602[Permalink](#invalidparamserror-code) The error type that occurred.  Overrides [Error](#error).[code](#error-code)

### `InvalidRequestError`

interface InvalidRequestError \{\
  [message](#invalidrequesterror-message): string;\
  [data](#invalidrequesterror-data)?: unknown;\
  [code](#invalidrequesterror-code): -32600;\
}A JSON-RPC error indicating that the request is not a valid request object. This error is returned when the message structure does not conform to the JSON-RPC 2.0 specification requirements for a request (e.g., missing required fields like `jsonrpc` or `method`, or using invalid types for these fields).   See[JSON-RPC 2.0 Error Object](https://www.jsonrpc.org/specification#error_object)  message: string[Permalink](#invalidrequesterror-message) A short description of the error. The message SHOULD be limited to a concise single sentence.  Inherited from [Error](#error).[message](#error-message) data?: unknown[Permalink](#invalidrequesterror-data) Additional information about the error. The value of this member is defined by the sender (e.g. detailed error information, nested errors etc.).  Inherited from [Error](#error).[data](#error-data) code: -32600[Permalink](#invalidrequesterror-code) The error type that occurred.  Overrides [Error](#error).[code](#error-code)

### `MethodNotFoundError`

interface MethodNotFoundError \{\
  [message](#methodnotfounderror-message): string;\
  [data](#methodnotfounderror-data)?: unknown;\
  [code](#methodnotfounderror-code): -32601;\
}A JSON-RPC error indicating that the requested method does not exist or is not available. In MCP, a server returns this error when a client invokes a method the server does not implement — either a genuinely unknown method, or one gated behind a server capability the server did not advertise (e.g., calling `prompts/list` when the `prompts` capability was not advertised). A request that requires a client capability the client did not declare is signalled instead by [MissingRequiredClientCapabilityError](#missingrequiredclientcapabilityerror) (`-32021`).   See[JSON-RPC 2.0 Error Object](https://www.jsonrpc.org/specification#error_object)  <details> <summary>Example: Prompts not supported[Permalink](#methodnotfounderror-example-prompts-not-supported)</summary><pre id="methodnotfounderror-example-prompts-not-supported">`{  "code": -32601,  "message": "Prompts not supported",  "data": {    "reason": "Server does not support the prompts capability"  }} `Copy</pre> </details> message: string[Permalink](#methodnotfounderror-message) A short description of the error. The message SHOULD be limited to a concise single sentence.  Inherited from [Error](#error).[message](#error-message) data?: unknown[Permalink](#methodnotfounderror-data) Additional information about the error. The value of this member is defined by the sender (e.g. detailed error information, nested errors etc.).  Inherited from [Error](#error).[data](#error-data) code: -32601[Permalink](#methodnotfounderror-code) The error type that occurred.  Overrides [Error](#error).[code](#error-code)

### `MISSING_REQUIRED_CLIENT_CAPABILITY`

MISSING\_REQUIRED\_CLIENT\_CAPABILITY: -32021Error code returned when a server requires a client capability that was
not declared in the request's `clientCapabilities`.&#x20;

### `MissingRequiredClientCapabilityError`

interface MissingRequiredClientCapabilityError \{\
  [jsonrpc](#missingrequiredclientcapabilityerror-jsonrpc): "2.0";\
  [id](#missingrequiredclientcapabilityerror-id)?: [RequestId](#requestid);\
  [error](#missingrequiredclientcapabilityerror-error): [Error](#error) & \{\
    code: -32021;\
    data: \{ requiredCapabilities: [ClientCapabilities](#clientcapabilities) };\
  };\
}Returned when processing a request requires a capability the client did not
declare in `clientCapabilities`. For HTTP, the response status code MUST be `400 Bad Request`.  <details> <summary>Example: Missing elicitation capability[Permalink](#missingrequiredclientcapabilityerror-example-missing-elicitation-capability)</summary><pre id="missingrequiredclientcapabilityerror-example-missing-elicitation-capability">`{  "jsonrpc": "2.0",  "id": 1,  "error": {    "code": -32021,    "message": "Server requires the elicitation capability for this request",    "data": {      "requiredCapabilities": {        "elicitation": {}      }    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#missingrequiredclientcapabilityerror-jsonrpc)  Inherited from [JSONRPCErrorResponse](#jsonrpcerrorresponse).[jsonrpc](#jsonrpcerrorresponse-jsonrpc) id?: RequestId[Permalink](#missingrequiredclientcapabilityerror-id)  Inherited from [JSONRPCErrorResponse](#jsonrpcerrorresponse).[id](#jsonrpcerrorresponse-id) error: Error & \{    code: -32021;    data: \{ requiredCapabilities: ClientCapabilities }; }[Permalink](#missingrequiredclientcapabilityerror-error)&#x20;

### `ParseError`

interface ParseError \{\
  [message](#parseerror-message): string;\
  [data](#parseerror-data)?: unknown;\
  [code](#parseerror-code): -32700;\
}A JSON-RPC error indicating that invalid JSON was received by the server. This error is returned when the server cannot parse the JSON text of a message.   See[JSON-RPC 2.0 Error Object](https://www.jsonrpc.org/specification#error_object)  <details> <summary>Example: Invalid JSON[Permalink](#parseerror-example-invalid-json)</summary><pre id="parseerror-example-invalid-json">`{  "code": -32700,  "message": "Parse error: Invalid JSON"} `Copy</pre> </details> message: string[Permalink](#parseerror-message) A short description of the error. The message SHOULD be limited to a concise single sentence.  Inherited from [Error](#error).[message](#error-message) data?: unknown[Permalink](#parseerror-data) Additional information about the error. The value of this member is defined by the sender (e.g. detailed error information, nested errors etc.).  Inherited from [Error](#error).[data](#error-data) code: -32700[Permalink](#parseerror-code) The error type that occurred.  Overrides [Error](#error).[code](#error-code)

### `UNSUPPORTED_PROTOCOL_VERSION`

UNSUPPORTED\_PROTOCOL\_VERSION: -32022Error code returned when the request's protocol version is not supported
by the server.&#x20;

### `UnsupportedProtocolVersionError`

interface UnsupportedProtocolVersionError \{\
  [jsonrpc](#unsupportedprotocolversionerror-jsonrpc): "2.0";\
  [id](#unsupportedprotocolversionerror-id)?: [RequestId](#requestid);\
  [error](#unsupportedprotocolversionerror-error): [Error](#error) & \{\
    code: -32022;\
    data: \{ supported: string\[]; requested: string };\
  };\
}Returned when the request's protocol version is unknown to the server or
unsupported (e.g., a known experimental or draft version the server has
chosen not to implement). For HTTP, the response status code MUST be `400 Bad Request`.  <details> <summary>Example: Unsupported protocol version[Permalink](#unsupportedprotocolversionerror-example-unsupported-protocol-version)</summary><pre id="unsupportedprotocolversionerror-example-unsupported-protocol-version">`{  "jsonrpc": "2.0",  "id": 1,  "error": {    "code": -32022,    "message": "Unsupported protocol version",    "data": {      "supported": [        "2026-07-28",        "2025-11-25"      ],      "requested": "1900-01-01"    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#unsupportedprotocolversionerror-jsonrpc)  Inherited from [JSONRPCErrorResponse](#jsonrpcerrorresponse).[jsonrpc](#jsonrpcerrorresponse-jsonrpc) id?: RequestId[Permalink](#unsupportedprotocolversionerror-id)  Inherited from [JSONRPCErrorResponse](#jsonrpcerrorresponse).[id](#jsonrpcerrorresponse-id) error: Error & \{    code: -32022;    data: \{ supported: string\[]; requested: string }; }[Permalink](#unsupportedprotocolversionerror-error)&#x20;

## Content

### `AudioContent`

interface AudioContent \{\
  [type](#audiocontent-type): "audio";\
  [data](#audiocontent-data): string;\
  [mimeType](#audiocontent-mimetype): string;\
  [annotations](#audiocontent-annotations)?: [Annotations](#annotations);\
  [\_meta](#audiocontent-_meta)?: [MetaObject](#metaobject);\
}Audio provided to or from an LLM.  <details> <summary>Example: \`audio/wav\` content[Permalink](#audiocontent-example-audiowav-content)</summary><pre id="audiocontent-example-audiowav-content">`{  "type": "audio",  "data": "UklGRiQAAABXQVZFZm10IBAAAAABAAEARKwAAIhYAQACABAAZGF0YQAAAAA=",  "mimeType": "audio/wav"} `Copy</pre> </details> type: "audio"[Permalink](#audiocontent-type)  data: string[Permalink](#audiocontent-data) The base64-encoded audio data.  mimeType: string[Permalink](#audiocontent-mimetype) The MIME type of the audio. Different providers may support different audio types.  annotations?: Annotations[Permalink](#audiocontent-annotations) Optional annotations for the client.  \_meta?: MetaObject[Permalink](#audiocontent-_meta)&#x20;

### `BlobResourceContents`

interface BlobResourceContents \{\
  [uri](#blobresourcecontents-uri): string;\
  [mimeType](#blobresourcecontents-mimetype)?: string;\
  [\_meta](#blobresourcecontents-_meta)?: [MetaObject](#metaobject);\
  [blob](#blobresourcecontents-blob): string;\
} <details> <summary>Example: Image file contents[Permalink](#blobresourcecontents-example-image-file-contents)</summary><pre id="blobresourcecontents-example-image-file-contents">`{  "uri": "file:///example.png",  "mimeType": "image/png",  "blob": "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg=="} `Copy</pre> </details> uri: string[Permalink](#blobresourcecontents-uri) The URI of this resource.  Inherited from ResourceContents.uri mimeType?: string[Permalink](#blobresourcecontents-mimetype) The MIME type of this resource, if known.  Inherited from ResourceContents.mimeType \_meta?: MetaObject[Permalink](#blobresourcecontents-_meta)  Inherited from ResourceContents.\_meta blob: string[Permalink](#blobresourcecontents-blob) A base64-encoded string representing the binary data of the item.&#x20;

### `ContentBlock`

ContentBlock:\
  | [TextContent](#textcontent)\
  | [ImageContent](#imagecontent)\
  | [AudioContent](#audiocontent)\
  | [ResourceLink](#resourcelink)\
  | [EmbeddedResource](#embeddedresource)

### `EmbeddedResource`

interface EmbeddedResource \{\
  [type](#embeddedresource-type): "resource";\
  [resource](#embeddedresource-resource): [TextResourceContents](#textresourcecontents) | [BlobResourceContents](#blobresourcecontents);\
  [annotations](#embeddedresource-annotations)?: [Annotations](#annotations);\
  [\_meta](#embeddedresource-_meta)?: [MetaObject](#metaobject);\
}The contents of a resource, embedded into a prompt or tool call result. It is up to the client how best to render embedded resources for the benefit
of the LLM and/or the user.  <details> <summary>Example: Embedded file resource with annotations[Permalink](#embeddedresource-example-embedded-file-resource-with-annotations)</summary><pre id="embeddedresource-example-embedded-file-resource-with-annotations">`{  "type": "resource",  "resource": {    "uri": "file:///project/src/main.rs",    "mimeType": "text/x-rust",    "text": "fn main() {\n    println!(\"Hello world!\");\n}"  },  "annotations": {    "audience": ["user", "assistant"],    "priority": 0.7,    "lastModified": "2025-05-03T14:30:00Z"  }} `Copy</pre> </details> type: "resource"[Permalink](#embeddedresource-type)  resource: TextResourceContents | BlobResourceContents[Permalink](#embeddedresource-resource)  annotations?: Annotations[Permalink](#embeddedresource-annotations) Optional annotations for the client.  \_meta?: MetaObject[Permalink](#embeddedresource-_meta)&#x20;

### `ImageContent`

interface ImageContent \{\
  [type](#imagecontent-type): "image";\
  [data](#imagecontent-data): string;\
  [mimeType](#imagecontent-mimetype): string;\
  [annotations](#imagecontent-annotations)?: [Annotations](#annotations);\
  [\_meta](#imagecontent-_meta)?: [MetaObject](#metaobject);\
}An image provided to or from an LLM.  <details> <summary>Example: \`image/png\` content with annotations[Permalink](#imagecontent-example-imagepng-content-with-annotations)</summary><pre id="imagecontent-example-imagepng-content-with-annotations">`{  "type": "image",  "data": "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNk+M9QDwADhgGAWjR9awAAAABJRU5ErkJggg==",  "mimeType": "image/png",  "annotations": {    "audience": ["user"],    "priority": 0.9  }} `Copy</pre> </details> type: "image"[Permalink](#imagecontent-type)  data: string[Permalink](#imagecontent-data) The base64-encoded image data.  mimeType: string[Permalink](#imagecontent-mimetype) The MIME type of the image. Different providers may support different image types.  annotations?: Annotations[Permalink](#imagecontent-annotations) Optional annotations for the client.  \_meta?: MetaObject[Permalink](#imagecontent-_meta)&#x20;

### `ResourceLink`

interface ResourceLink \{\
  [icons](#resourcelink-icons)?: [Icon](#icon)\[];\
  [name](#resourcelink-name): string;\
  [title](#resourcelink-title)?: string;\
  [uri](#resourcelink-uri): string;\
  [description](#resourcelink-description)?: string;\
  [mimeType](#resourcelink-mimetype)?: string;\
  [annotations](#resourcelink-annotations)?: [Annotations](#annotations);\
  [size](#resourcelink-size)?: number;\
  [\_meta](#resourcelink-_meta)?: [MetaObject](#metaobject);\
  [type](#resourcelink-type): "resource\_link";\
}A resource that the server is capable of reading, included in a prompt or tool call result. Note: resource links returned by tools are not guaranteed to appear in the results of [resources/list](#listresourcesrequest) requests.  <details> <summary>Example: File resource link[Permalink](#resourcelink-example-file-resource-link)</summary><pre id="resourcelink-example-file-resource-link">`{  "type": "resource_link",  "uri": "file:///project/src/main.rs",  "name": "main.rs",  "description": "Primary application entry point",  "mimeType": "text/x-rust"} `Copy</pre> </details> icons?: Icon\[][Permalink](#resourcelink-icons) Optional set of sized icons that the client can display in a user interface. Clients that support rendering icons MUST support at least the following MIME types: <ul> <li>`image/png` - PNG images (safe, universal compatibility)</li> <li>`image/jpeg` (and `image/jpg`) - JPEG images (safe, universal compatibility)</li> </ul> Clients that support rendering icons SHOULD also support: <ul> <li>`image/svg+xml` - SVG images (scalable but requires security precautions)</li> <li>`image/webp` - WebP images (modern, efficient format)</li> </ul>  Inherited from [Resource](#resource).[icons](#resource-icons) name: string[Permalink](#resourcelink-name) Intended for programmatic or logical use, but used as a display name in past specs or fallback (if title isn't present).  Inherited from [Resource](#resource).[name](#resource-name) title?: string[Permalink](#resourcelink-title) Intended for UI and end-user contexts — optimized to be human-readable and easily understood,
even by those unfamiliar with domain-specific terminology. If not provided, the name should be used for display (except for [Tool](#tool),
where `annotations.title` should be given precedence over using `name`,
if present).  Inherited from [Resource](#resource).[title](#resource-title) uri: string[Permalink](#resourcelink-uri) The URI of this resource.  Inherited from [Resource](#resource).[uri](#resource-uri) description?: string[Permalink](#resourcelink-description) A description of what this resource represents. This can be used by clients to improve the LLM's understanding of available resources. It can be thought of like a "hint" to the model.  Inherited from [Resource](#resource).[description](#resource-description) mimeType?: string[Permalink](#resourcelink-mimetype) The MIME type of this resource, if known.  Inherited from [Resource](#resource).[mimeType](#resource-mimetype) annotations?: Annotations[Permalink](#resourcelink-annotations) Optional annotations for the client.  Inherited from [Resource](#resource).[annotations](#resource-annotations) size?: number[Permalink](#resourcelink-size) The size of the raw resource content, in bytes (i.e., before base64 encoding or any tokenization), if known. This can be used by Hosts to display file sizes and estimate context window usage.  Inherited from [Resource](#resource).[size](#resource-size) \_meta?: MetaObject[Permalink](#resourcelink-_meta)  Inherited from [Resource](#resource).[\_meta](#resource-_meta) type: "resource\_link"[Permalink](#resourcelink-type)&#x20;

### `TextContent`

interface TextContent \{\
  [type](#textcontent-type): "text";\
  [text](#textcontent-text): string;\
  [annotations](#textcontent-annotations)?: [Annotations](#annotations);\
  [\_meta](#textcontent-_meta)?: [MetaObject](#metaobject);\
}Text provided to or from an LLM.  <details> <summary>Example: Text content[Permalink](#textcontent-example-text-content)</summary><pre id="textcontent-example-text-content">`{  "type": "text",  "text": "Tool result text"} `Copy</pre> </details> type: "text"[Permalink](#textcontent-type)  text: string[Permalink](#textcontent-text) The text content of the message.  annotations?: Annotations[Permalink](#textcontent-annotations) Optional annotations for the client.  \_meta?: MetaObject[Permalink](#textcontent-_meta)&#x20;

### `TextResourceContents`

interface TextResourceContents \{\
  [uri](#textresourcecontents-uri): string;\
  [mimeType](#textresourcecontents-mimetype)?: string;\
  [\_meta](#textresourcecontents-_meta)?: [MetaObject](#metaobject);\
  [text](#textresourcecontents-text): string;\
} <details> <summary>Example: Text file contents[Permalink](#textresourcecontents-example-text-file-contents)</summary><pre id="textresourcecontents-example-text-file-contents">`{  "uri": "file:///example.txt",  "mimeType": "text/plain",  "text": "Resource content"} `Copy</pre> </details> uri: string[Permalink](#textresourcecontents-uri) The URI of this resource.  Inherited from ResourceContents.uri mimeType?: string[Permalink](#textresourcecontents-mimetype) The MIME type of this resource, if known.  Inherited from ResourceContents.mimeType \_meta?: MetaObject[Permalink](#textresourcecontents-_meta)  Inherited from ResourceContents.\_meta text: string[Permalink](#textresourcecontents-text) The text of the item. This must only be set if the item can actually be represented as text (not binary data).&#x20;

## `completion/complete`

### `CompleteRequest`

interface CompleteRequest \{\
  [jsonrpc](#completerequest-jsonrpc): "2.0";\
  [id](#completerequest-id): [RequestId](#requestid);\
  [method](#completerequest-method): "completion/complete";\
  [params](#completerequest-params): [CompleteRequestParams](#completerequestparams);\
}A request from the client to the server, to ask for completion options.  <details> <summary>Example: Completion request[Permalink](#completerequest-example-completion-request)</summary><pre id="completerequest-example-completion-request">`{  "jsonrpc": "2.0",  "id": "completion-example",  "method": "completion/complete",  "params": {    "_meta": {      "io.modelcontextprotocol/protocolVersion": "2026-07-28",      "io.modelcontextprotocol/clientInfo": {        "name": "ExampleClient",        "version": "1.0.0"      },      "io.modelcontextprotocol/clientCapabilities": {}    },    "ref": {      "type": "ref/prompt",      "name": "code_review"    },    "argument": {      "name": "language",      "value": "py"    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#completerequest-jsonrpc)  Inherited from [JSONRPCRequest](#jsonrpcrequest).[jsonrpc](#jsonrpcrequest-jsonrpc) id: RequestId[Permalink](#completerequest-id)  Inherited from [JSONRPCRequest](#jsonrpcrequest).[id](#jsonrpcrequest-id) method: "completion/complete"[Permalink](#completerequest-method)  Overrides [JSONRPCRequest](#jsonrpcrequest).[method](#jsonrpcrequest-method) params: CompleteRequestParams[Permalink](#completerequest-params)  Overrides [JSONRPCRequest](#jsonrpcrequest).[params](#jsonrpcrequest-params)

### `CompleteRequestParams`

interface CompleteRequestParams \{\
  [\_meta](#completerequestparams-_meta): [RequestMetaObject](#requestmetaobject);\
  [ref](#completerequestparams-ref): [PromptReference](#promptreference) | [ResourceTemplateReference](#resourcetemplatereference);\
  [argument](#completerequestparams-argument): \{ name: string; value: string };\
  [context](#completerequestparams-context)?: \{ arguments?: \{ \[key: string]: string } };\
}Parameters for a `completion/complete` request.  <details> <summary>Example: Prompt argument completion[Permalink](#completerequestparams-example-prompt-argument-completion)</summary><pre id="completerequestparams-example-prompt-argument-completion">`{  "_meta": {    "io.modelcontextprotocol/protocolVersion": "2026-07-28",    "io.modelcontextprotocol/clientInfo": {      "name": "ExampleClient",      "version": "1.0.0"    },    "io.modelcontextprotocol/clientCapabilities": {}  },  "ref": {    "type": "ref/prompt",    "name": "code_review"  },  "argument": {    "name": "language",    "value": "py"  }} `Copy</pre> </details> <details> <summary>Example: Prompt argument completion with context[Permalink](#completerequestparams-example-prompt-argument-completion-with-context)</summary><pre id="completerequestparams-example-prompt-argument-completion-with-context">`{  "_meta": {    "io.modelcontextprotocol/protocolVersion": "2026-07-28",    "io.modelcontextprotocol/clientInfo": {      "name": "ExampleClient",      "version": "1.0.0"    },    "io.modelcontextprotocol/clientCapabilities": {}  },  "ref": {    "type": "ref/prompt",    "name": "code_review"  },  "argument": {    "name": "framework",    "value": "fla"  },  "context": {    "arguments": {      "language": "python"    }  }} `Copy</pre> </details> \_meta: RequestMetaObject[Permalink](#completerequestparams-_meta)  Inherited from [RequestParams](#requestparams).[\_meta](#requestparams-_meta) ref: PromptReference | ResourceTemplateReference[Permalink](#completerequestparams-ref)  argument: \{ name: string; value: string }[Permalink](#completerequestparams-argument) The argument's information   Type Declaration <ul> <li> name: string The name of the argument </li> <li> value: string The value of the argument to use for completion matching. </li></ul> context?: \{ arguments?: \{ \[key: string]: string } }[Permalink](#completerequestparams-context) Additional, optional context for completions   Type Declaration <ul> <li> `Optional`arguments?: \{ \[key: string]: string } Previously-resolved variables in a URI template or prompt. </li></ul>

### `CompleteResultResponse`

interface CompleteResultResponse \{\
  [jsonrpc](#completeresultresponse-jsonrpc): "2.0";\
  [id](#completeresultresponse-id): [RequestId](#requestid);\
  [result](#completeresultresponse-result): [CompleteResult](#completeresult);\
}A successful response from the server for a [completion/complete](#completerequest) request.  <details> <summary>Example: Completion result response[Permalink](#completeresultresponse-example-completion-result-response)</summary><pre id="completeresultresponse-example-completion-result-response">`{  "jsonrpc": "2.0",  "id": "completion-example",  "result": {    "resultType": "complete",    "completion": {      "values": ["flask"],      "total": 1,      "hasMore": false    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#completeresultresponse-jsonrpc)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[jsonrpc](#jsonrpcresultresponse-jsonrpc) id: RequestId[Permalink](#completeresultresponse-id)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[id](#jsonrpcresultresponse-id) result: CompleteResult[Permalink](#completeresultresponse-result)  Overrides [JSONRPCResultResponse](#jsonrpcresultresponse).[result](#jsonrpcresultresponse-result)

### `CompleteResult`

interface CompleteResult \{\
  [\_meta](#completeresult-_meta)?: [ResultMetaObject](#resultmetaobject);\
  [resultType](#completeresult-resulttype): string;\
  [completion](#completeresult-completion): \{ values: string\[]; total?: number; hasMore?: boolean };\
  \[key: string]: unknown;\
}The result returned by the server for a [completion/complete](#completerequest) request.  <details> <summary>Example: Single completion value[Permalink](#completeresult-example-single-completion-value)</summary><pre id="completeresult-example-single-completion-value">`{  "resultType": "complete",  "completion": {    "values": ["flask"],    "total": 1,    "hasMore": false  }} `Copy</pre> </details> <details> <summary>Example: Multiple completion values with more available[Permalink](#completeresult-example-multiple-completion-values-with-more-available)</summary><pre id="completeresult-example-multiple-completion-values-with-more-available">`{  "resultType": "complete",  "completion": {    "values": ["python", "pytorch", "pyside"],    "total": 10,    "hasMore": true  }} `Copy</pre> </details> \_meta?: ResultMetaObject[Permalink](#completeresult-_meta)  Inherited from [Result](#result).[\_meta](#result-_meta) resultType: string[Permalink](#completeresult-resulttype) Indicates the type of the result, which allows the client to determine
how to parse the result object. Servers implementing this protocol version MUST include this field.
For backward compatibility, when a client receives a result from a
server implementing an earlier protocol version (which does not include `resultType`), the client MUST treat the absent field as `"complete"`.  Inherited from [Result](#result).[resultType](#result-resulttype) completion: \{ values: string\[]; total?: number; hasMore?: boolean }[Permalink](#completeresult-completion)  Type Declaration <ul> <li> values: string\[] An array of completion values. Must not exceed 100 items. </li> <li> `Optional`total?: number The total number of completion options available. This can exceed the number of values actually sent in the response. </li> <li> `Optional`hasMore?: boolean Indicates whether there are additional completion options beyond those provided in the current response, even if the exact total is unknown. </li></ul>

### `PromptReference`

interface PromptReference \{\
  [name](#promptreference-name): string;\
  [title](#promptreference-title)?: string;\
  [type](#promptreference-type): "ref/prompt";\
}Identifies a prompt.  name: string[Permalink](#promptreference-name) Intended for programmatic or logical use, but used as a display name in past specs or fallback (if title isn't present).  Inherited from BaseMetadata.name title?: string[Permalink](#promptreference-title) Intended for UI and end-user contexts — optimized to be human-readable and easily understood,
even by those unfamiliar with domain-specific terminology. If not provided, the name should be used for display (except for [Tool](#tool),
where `annotations.title` should be given precedence over using `name`,
if present).  Inherited from BaseMetadata.title type: "ref/prompt"[Permalink](#promptreference-type)&#x20;

### `ResourceTemplateReference`

interface ResourceTemplateReference \{\
  [type](#resourcetemplatereference-type): "ref/resource";\
  [uri](#resourcetemplatereference-uri): string;\
}A reference to a resource or resource template definition.  type: "ref/resource"[Permalink](#resourcetemplatereference-type)  uri: string[Permalink](#resourcetemplatereference-uri) The URI or URI template of the resource.&#x20;

## `elicitation/create`

### `ElicitRequest`

interface ElicitRequest \{\
  [method](#elicitrequest-method): "elicitation/create";\
  [params](#elicitrequest-params): [ElicitRequestParams](#elicitrequestparams);\
}A request from the server to elicit additional information from the user via the client.  <details> <summary>Example: Elicitation request[Permalink](#elicitrequest-example-elicitation-request)</summary><pre id="elicitrequest-example-elicitation-request">`{  "method": "elicitation/create",  "params": {    "mode": "form",    "message": "Please provide your GitHub username",    "requestedSchema": {      "type": "object",      "properties": {        "name": {          "type": "string",          "title": "GitHub Username",          "description": "Your GitHub username"        }      },      "required": ["name"]    }  }} `Copy</pre> </details> method: "elicitation/create"[Permalink](#elicitrequest-method)  params: ElicitRequestParams[Permalink](#elicitrequest-params)&#x20;

### `ElicitRequestParams`

ElicitRequestParams: [ElicitRequestFormParams](#elicitrequestformparams) | [ElicitRequestURLParams](#elicitrequesturlparams)The parameters for a request to elicit additional information from the user via the client.&#x20;

### `ElicitResult`

interface ElicitResult \{\
  [action](#elicitresult-action): "accept" | "decline" | "cancel";\
  [content](#elicitresult-content)?: \{ \[key: string]: string | number | boolean | string\[] };\
}The result returned by the client for an [elicitation/create](#elicitrequest) request.  <details> <summary>Example: Input single field[Permalink](#elicitresult-example-input-single-field)</summary><pre id="elicitresult-example-input-single-field">`{  "action": "accept",  "content": {    "name": "octocat"  }} `Copy</pre> </details> <details> <summary>Example: Input multiple fields[Permalink](#elicitresult-example-input-multiple-fields)</summary><pre id="elicitresult-example-input-multiple-fields">`{  "action": "accept",  "content": {    "name": "Monalisa Octocat",    "email": "octocat@github.com",    "age": 30  }} `Copy</pre> </details> <details> <summary>Example: Accept URL mode (no content)[Permalink](#elicitresult-example-accept-url-mode-no-content)</summary><pre id="elicitresult-example-accept-url-mode-no-content">`{  "action": "accept"} `Copy</pre> </details> action: "accept" | "decline" | "cancel"[Permalink](#elicitresult-action) The user action in response to the elicitation. <ul> <li>`"accept"`: User submitted the form/confirmed the action</li> <li>`"decline"`: User explicitly declined the action</li> <li>`"cancel"`: User dismissed without making an explicit choice</li> </ul>  content?: \{ \[key: string]: string | number | boolean | string\[] }[Permalink](#elicitresult-content) The submitted form data, only present when action is `"accept"` and mode was `"form"`.
Contains values matching the requested schema.
Omitted for out-of-band mode responses.&#x20;

### `BooleanSchema`

interface BooleanSchema \{\
  [type](#booleanschema-type): "boolean";\
  [title](#booleanschema-title)?: string;\
  [description](#booleanschema-description)?: string;\
  [default](#booleanschema-default)?: boolean;\
} <details> <summary>Example: Boolean input schema[Permalink](#booleanschema-example-boolean-input-schema)</summary><pre id="booleanschema-example-boolean-input-schema">`{  "type": "boolean",  "title": "Display Name",  "description": "Description text",  "default": false} `Copy</pre> </details> type: "boolean"[Permalink](#booleanschema-type)  title?: string[Permalink](#booleanschema-title)  description?: string[Permalink](#booleanschema-description)  default?: boolean[Permalink](#booleanschema-default)&#x20;

### `ElicitRequestFormParams`

interface ElicitRequestFormParams \{\
  [mode](#elicitrequestformparams-mode)?: "form";\
  [message](#elicitrequestformparams-message): string;\
  [requestedSchema](#elicitrequestformparams-requestedschema): \{\
    \$schema?: string;\
    type: "object";\
    properties: \{ \[key: string]: [PrimitiveSchemaDefinition](#primitiveschemadefinition) };\
    required?: string\[];\
  };\
}The parameters for a request to elicit non-sensitive information from the user via a form in the client.  <details> <summary>Example: Elicit single field[Permalink](#elicitrequestformparams-example-elicit-single-field)</summary><pre id="elicitrequestformparams-example-elicit-single-field">`{  "mode": "form",  "message": "Please provide your GitHub username",  "requestedSchema": {    "type": "object",    "properties": {      "name": {        "type": "string"      }    },    "required": ["name"]  }} `Copy</pre> </details> <details> <summary>Example: Elicit multiple fields[Permalink](#elicitrequestformparams-example-elicit-multiple-fields)</summary><pre id="elicitrequestformparams-example-elicit-multiple-fields">`{  "mode": "form",  "message": "Please provide your contact information",  "requestedSchema": {    "type": "object",    "properties": {      "name": {        "type": "string",        "description": "Your full name"      },      "email": {        "type": "string",        "format": "email",        "description": "Your email address"      },      "age": {        "type": "number",        "minimum": 18,        "description": "Your age"      }    },    "required": ["name", "email"]  }} `Copy</pre> </details> mode?: "form"[Permalink](#elicitrequestformparams-mode) The elicitation mode.  message: string[Permalink](#elicitrequestformparams-message) The message to present to the user describing what information is being requested.  requestedSchema: \{    \$schema?: string;    type: "object";    properties: \{ \[key: string]: PrimitiveSchemaDefinition };    required?: string\[]; }[Permalink](#elicitrequestformparams-requestedschema) A restricted subset of JSON Schema.
Only top-level properties are allowed, without nesting.&#x20;

### `ElicitRequestURLParams`

interface ElicitRequestURLParams \{\
  [mode](#elicitrequesturlparams-mode): "url";\
  [message](#elicitrequesturlparams-message): string;\
  [url](#elicitrequesturlparams-url): string;\
}The parameters for a request to elicit information from the user via a URL in the client.  <details> <summary>Example: Elicit sensitive data[Permalink](#elicitrequesturlparams-example-elicit-sensitive-data)</summary><pre id="elicitrequesturlparams-example-elicit-sensitive-data">`{  "mode": "url",  "url": "https://mcp.example.com/ui/set&#x5F;api&#x5F;key",  "message": "Please provide your API key to continue."} `Copy</pre> </details> mode: "url"[Permalink](#elicitrequesturlparams-mode) The elicitation mode.  message: string[Permalink](#elicitrequesturlparams-message) The message to present to the user explaining why the interaction is needed.  url: string[Permalink](#elicitrequesturlparams-url) The URL that the user should navigate to.&#x20;

### `EnumSchema`

EnumSchema:\
  | [SingleSelectEnumSchema](#singleselectenumschema)\
  | [MultiSelectEnumSchema](#multiselectenumschema)\
  | [LegacyTitledEnumSchema](#legacytitledenumschema)

### `LegacyTitledEnumSchema`

interface LegacyTitledEnumSchema \{\
  [type](#legacytitledenumschema-type): "string";\
  [title](#legacytitledenumschema-title)?: string;\
  [description](#legacytitledenumschema-description)?: string;\
  [enum](#legacytitledenumschema-enum): string\[];\
  [enumNames](#legacytitledenumschema-enumnames)?: string\[];\
  [default](#legacytitledenumschema-default)?: string;\
}Use [TitledSingleSelectEnumSchema](#titledsingleselectenumschema) instead.
This interface will be removed in a future version.  type: "string"[Permalink](#legacytitledenumschema-type)  title?: string[Permalink](#legacytitledenumschema-title)  description?: string[Permalink](#legacytitledenumschema-description)  enum: string\[][Permalink](#legacytitledenumschema-enum)  enumNames?: string\[][Permalink](#legacytitledenumschema-enumnames) (Legacy) Display names for enum values.
Non-standard according to JSON schema 2020-12.  default?: string[Permalink](#legacytitledenumschema-default)&#x20;

### `MultiSelectEnumSchema`

MultiSelectEnumSchema:\
  | [UntitledMultiSelectEnumSchema](#untitledmultiselectenumschema)\
  | [TitledMultiSelectEnumSchema](#titledmultiselectenumschema)

### `NumberSchema`

interface NumberSchema \{\
  [type](#numberschema-type): "number" | "integer";\
  [title](#numberschema-title)?: string;\
  [description](#numberschema-description)?: string;\
  [minimum](#numberschema-minimum)?: number;\
  [maximum](#numberschema-maximum)?: number;\
  [default](#numberschema-default)?: number;\
} <details> <summary>Example: Number input schema[Permalink](#numberschema-example-number-input-schema)</summary><pre id="numberschema-example-number-input-schema">`{  "type": "number",  "title": "Display Name",  "description": "Description text",  "minimum": 0,  "maximum": 100,  "default": 50} `Copy</pre> </details> type: "number" | "integer"[Permalink](#numberschema-type)  title?: string[Permalink](#numberschema-title)  description?: string[Permalink](#numberschema-description)  minimum?: number[Permalink](#numberschema-minimum)  maximum?: number[Permalink](#numberschema-maximum)  default?: number[Permalink](#numberschema-default)&#x20;

### `PrimitiveSchemaDefinition`

PrimitiveSchemaDefinition:\
  | [StringSchema](#stringschema)\
  | [NumberSchema](#numberschema)\
  | [BooleanSchema](#booleanschema)\
  | [EnumSchema](#enumschema)Restricted schema definitions that only allow primitive types
without nested objects or arrays.&#x20;

### `SingleSelectEnumSchema`

SingleSelectEnumSchema:\
  | [UntitledSingleSelectEnumSchema](#untitledsingleselectenumschema)\
  | [TitledSingleSelectEnumSchema](#titledsingleselectenumschema)

### `StringSchema`

interface StringSchema \{\
  [type](#stringschema-type): "string";\
  [title](#stringschema-title)?: string;\
  [description](#stringschema-description)?: string;\
  [minLength](#stringschema-minlength)?: number;\
  [maxLength](#stringschema-maxlength)?: number;\
  [format](#stringschema-format)?: "uri" | "email" | "date" | "date-time";\
  [default](#stringschema-default)?: string;\
} <details> <summary>Example: Email input schema[Permalink](#stringschema-example-email-input-schema)</summary><pre id="stringschema-example-email-input-schema">`{  "type": "string",  "title": "Display Name",  "description": "Description text",  "minLength": 3,  "maxLength": 50,  "format": "email",  "default": "user@example.com"} `Copy</pre> </details> type: "string"[Permalink](#stringschema-type)  title?: string[Permalink](#stringschema-title)  description?: string[Permalink](#stringschema-description)  minLength?: number[Permalink](#stringschema-minlength)  maxLength?: number[Permalink](#stringschema-maxlength)  format?: "uri" | "email" | "date" | "date-time"[Permalink](#stringschema-format)  default?: string[Permalink](#stringschema-default)&#x20;

### `TitledMultiSelectEnumSchema`

interface TitledMultiSelectEnumSchema \{\
  [type](#titledmultiselectenumschema-type): "array";\
  [title](#titledmultiselectenumschema-title)?: string;\
  [description](#titledmultiselectenumschema-description)?: string;\
  [minItems](#titledmultiselectenumschema-minitems)?: number;\
  [maxItems](#titledmultiselectenumschema-maxitems)?: number;\
  [items](#titledmultiselectenumschema-items): \{ anyOf: \{ const: string; title: string }\[] };\
  [default](#titledmultiselectenumschema-default)?: string\[];\
}Schema for multiple-selection enumeration with display titles for each option.  <details> <summary>Example: Titled color multi-select schema[Permalink](#titledmultiselectenumschema-example-titled-color-multi-select-schema)</summary><pre id="titledmultiselectenumschema-example-titled-color-multi-select-schema">`{  "type": "array",  "title": "Color Selection",  "description": "Choose your favorite colors",  "minItems": 1,  "maxItems": 2,  "items": {    "anyOf": [      { "const": "#FF0000", "title": "Red" },      { "const": "#00FF00", "title": "Green" },      { "const": "#0000FF", "title": "Blue" }    ]  },  "default": ["#FF0000", "#00FF00"]} `Copy</pre> </details> type: "array"[Permalink](#titledmultiselectenumschema-type)  title?: string[Permalink](#titledmultiselectenumschema-title) Optional title for the enum field.  description?: string[Permalink](#titledmultiselectenumschema-description) Optional description for the enum field.  minItems?: number[Permalink](#titledmultiselectenumschema-minitems) Minimum number of items to select.  maxItems?: number[Permalink](#titledmultiselectenumschema-maxitems) Maximum number of items to select.  items: \{ anyOf: \{ const: string; title: string }\[] }[Permalink](#titledmultiselectenumschema-items) Schema for array items with enum options and display labels.   Type Declaration <ul> <li> anyOf: \{ const: string; title: string }\[] Array of enum options with values and display labels. </li></ul> default?: string\[][Permalink](#titledmultiselectenumschema-default) Optional default value.&#x20;

### `TitledSingleSelectEnumSchema`

interface TitledSingleSelectEnumSchema \{\
  [type](#titledsingleselectenumschema-type): "string";\
  [title](#titledsingleselectenumschema-title)?: string;\
  [description](#titledsingleselectenumschema-description)?: string;\
  [oneOf](#titledsingleselectenumschema-oneof): \{ const: string; title: string }\[];\
  [default](#titledsingleselectenumschema-default)?: string;\
}Schema for single-selection enumeration with display titles for each option.  <details> <summary>Example: Titled color select schema[Permalink](#titledsingleselectenumschema-example-titled-color-select-schema)</summary><pre id="titledsingleselectenumschema-example-titled-color-select-schema">`{  "type": "string",  "title": "Color Selection",  "description": "Choose your favorite color",  "oneOf": [    { "const": "#FF0000", "title": "Red" },    { "const": "#00FF00", "title": "Green" },    { "const": "#0000FF", "title": "Blue" }  ],  "default": "#FF0000"} `Copy</pre> </details> type: "string"[Permalink](#titledsingleselectenumschema-type)  title?: string[Permalink](#titledsingleselectenumschema-title) Optional title for the enum field.  description?: string[Permalink](#titledsingleselectenumschema-description) Optional description for the enum field.  oneOf: \{ const: string; title: string }\[][Permalink](#titledsingleselectenumschema-oneof) Array of enum options with values and display labels.   Type Declaration <ul> <li> const: string The enum value. </li> <li> title: string Display label for this option. </li></ul> default?: string[Permalink](#titledsingleselectenumschema-default) Optional default value.&#x20;

### `UntitledMultiSelectEnumSchema`

interface UntitledMultiSelectEnumSchema \{\
  [type](#untitledmultiselectenumschema-type): "array";\
  [title](#untitledmultiselectenumschema-title)?: string;\
  [description](#untitledmultiselectenumschema-description)?: string;\
  [minItems](#untitledmultiselectenumschema-minitems)?: number;\
  [maxItems](#untitledmultiselectenumschema-maxitems)?: number;\
  [items](#untitledmultiselectenumschema-items): \{ type: "string"; enum: string\[] };\
  [default](#untitledmultiselectenumschema-default)?: string\[];\
}Schema for multiple-selection enumeration without display titles for options.  <details> <summary>Example: Color multi-select schema[Permalink](#untitledmultiselectenumschema-example-color-multi-select-schema)</summary><pre id="untitledmultiselectenumschema-example-color-multi-select-schema">`{  "type": "array",  "title": "Color Selection",  "description": "Choose your favorite colors",  "minItems": 1,  "maxItems": 2,  "items": {    "type": "string",    "enum": ["Red", "Green", "Blue"]  },  "default": ["Red", "Green"]} `Copy</pre> </details> type: "array"[Permalink](#untitledmultiselectenumschema-type)  title?: string[Permalink](#untitledmultiselectenumschema-title) Optional title for the enum field.  description?: string[Permalink](#untitledmultiselectenumschema-description) Optional description for the enum field.  minItems?: number[Permalink](#untitledmultiselectenumschema-minitems) Minimum number of items to select.  maxItems?: number[Permalink](#untitledmultiselectenumschema-maxitems) Maximum number of items to select.  items: \{ type: "string"; enum: string\[] }[Permalink](#untitledmultiselectenumschema-items) Schema for the array items.   Type Declaration <ul> <li> type: "string"</li> <li> enum: string\[] Array of enum values to choose from. </li></ul> default?: string\[][Permalink](#untitledmultiselectenumschema-default) Optional default value.&#x20;

### `UntitledSingleSelectEnumSchema`

interface UntitledSingleSelectEnumSchema \{\
  [type](#untitledsingleselectenumschema-type): "string";\
  [title](#untitledsingleselectenumschema-title)?: string;\
  [description](#untitledsingleselectenumschema-description)?: string;\
  [enum](#untitledsingleselectenumschema-enum): string\[];\
  [default](#untitledsingleselectenumschema-default)?: string;\
}Schema for single-selection enumeration without display titles for options.  <details> <summary>Example: Color select schema[Permalink](#untitledsingleselectenumschema-example-color-select-schema)</summary><pre id="untitledsingleselectenumschema-example-color-select-schema">`{  "type": "string",  "title": "Color Selection",  "description": "Choose your favorite color",  "enum": ["Red", "Green", "Blue"],  "default": "Red"} `Copy</pre> </details> type: "string"[Permalink](#untitledsingleselectenumschema-type)  title?: string[Permalink](#untitledsingleselectenumschema-title) Optional title for the enum field.  description?: string[Permalink](#untitledsingleselectenumschema-description) Optional description for the enum field.  enum: string\[][Permalink](#untitledsingleselectenumschema-enum) Array of enum values to choose from.  default?: string[Permalink](#untitledsingleselectenumschema-default) Optional default value.&#x20;

## `notifications/cancelled`

### `CancelledNotification`

interface CancelledNotification \{\
  [jsonrpc](#cancellednotification-jsonrpc): "2.0";\
  [method](#cancellednotification-method): "notifications/cancelled";\
  [params](#cancellednotification-params): [CancelledNotificationParams](#cancellednotificationparams);\
}This notification is sent by the client to indicate that it is cancelling a request it previously issued. On stdio, the server also sends this notification, solely to terminate a [subscriptions/listen](#subscriptionslistenrequest) stream: it references the ID of the `subscriptions/listen` request that opened the stream. Servers MUST NOT use this notification to cancel any other request. The request SHOULD still be in-flight, but due to communication latency, it is always possible that this notification MAY arrive after the request has already finished. This notification indicates that the result will be unused, so any associated processing SHOULD cease.  <details> <summary>Example: User-requested cancellation[Permalink](#cancellednotification-example-user-requested-cancellation)</summary><pre id="cancellednotification-example-user-requested-cancellation">`{  "jsonrpc": "2.0",  "method": "notifications/cancelled",  "params": {    "requestId": "123",    "reason": "User requested cancellation"  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#cancellednotification-jsonrpc)  Inherited from [JSONRPCNotification](#jsonrpcnotification).[jsonrpc](#jsonrpcnotification-jsonrpc) method: "notifications/cancelled"[Permalink](#cancellednotification-method)  Overrides [JSONRPCNotification](#jsonrpcnotification).[method](#jsonrpcnotification-method) params: CancelledNotificationParams[Permalink](#cancellednotification-params)  Overrides [JSONRPCNotification](#jsonrpcnotification).[params](#jsonrpcnotification-params)

### `CancelledNotificationParams`

interface CancelledNotificationParams \{\
  [\_meta](#cancellednotificationparams-_meta)?: [NotificationMetaObject](#notificationmetaobject);\
  [requestId](#cancellednotificationparams-requestid): [RequestId](#requestid);\
  [reason](#cancellednotificationparams-reason)?: string;\
}Parameters for a `notifications/cancelled` notification.  <details> <summary>Example: User-requested cancellation[Permalink](#cancellednotificationparams-example-user-requested-cancellation)</summary><pre id="cancellednotificationparams-example-user-requested-cancellation">`{  "requestId": "123",  "reason": "User requested cancellation"} `Copy</pre> </details> \_meta?: NotificationMetaObject[Permalink](#cancellednotificationparams-_meta)  Inherited from [NotificationParams](#notificationparams).[\_meta](#notificationparams-_meta) requestId: RequestId[Permalink](#cancellednotificationparams-requestid) The ID of the request to cancel. This MUST correspond to the ID of a request the client previously issued.  reason?: string[Permalink](#cancellednotificationparams-reason) An optional string describing the reason for the cancellation. This MAY be logged or presented to the user.&#x20;

## `notifications/message`

### `LoggingMessageNotification`

interface LoggingMessageNotification \{\
  [jsonrpc](#loggingmessagenotification-jsonrpc): "2.0";\
  [method](#loggingmessagenotification-method): "notifications/message";\
  [params](#loggingmessagenotification-params): [LoggingMessageNotificationParams](#loggingmessagenotificationparams);\
}JSONRPCNotification of a log message passed from server to client. The client opts in by setting `"io.modelcontextprotocol/logLevel"` in a request's `_meta`.   DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.  <details> <summary>Example: Log database connection failed[Permalink](#loggingmessagenotification-example-log-database-connection-failed)</summary><pre id="loggingmessagenotification-example-log-database-connection-failed">`{  "jsonrpc": "2.0",  "method": "notifications/message",  "params": {    "level": "error",    "logger": "database",    "data": {      "error": "Connection failed",      "details": {        "host": "localhost",        "port": 5432      }    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#loggingmessagenotification-jsonrpc)  Inherited from [JSONRPCNotification](#jsonrpcnotification).[jsonrpc](#jsonrpcnotification-jsonrpc) method: "notifications/message"[Permalink](#loggingmessagenotification-method)  Overrides [JSONRPCNotification](#jsonrpcnotification).[method](#jsonrpcnotification-method) params: LoggingMessageNotificationParams[Permalink](#loggingmessagenotification-params)  Overrides [JSONRPCNotification](#jsonrpcnotification).[params](#jsonrpcnotification-params)

### `LoggingMessageNotificationParams`

interface LoggingMessageNotificationParams \{\
  [\_meta](#loggingmessagenotificationparams-_meta)?: [NotificationMetaObject](#notificationmetaobject);\
  [level](#loggingmessagenotificationparams-level): [LoggingLevel](#logginglevel);\
  [logger](#loggingmessagenotificationparams-logger)?: string;\
  [data](#loggingmessagenotificationparams-data): unknown;\
}Parameters for a `notifications/message` notification.   DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.  <details> <summary>Example: Log database connection failed[Permalink](#loggingmessagenotificationparams-example-log-database-connection-failed)</summary><pre id="loggingmessagenotificationparams-example-log-database-connection-failed">`{  "level": "error",  "logger": "database",  "data": {    "error": "Connection failed",    "details": {      "host": "localhost",      "port": 5432    }  }} `Copy</pre> </details> \_meta?: NotificationMetaObject[Permalink](#loggingmessagenotificationparams-_meta)  Inherited from [NotificationParams](#notificationparams).[\_meta](#notificationparams-_meta) level: LoggingLevel[Permalink](#loggingmessagenotificationparams-level) The severity of this log message.  logger?: string[Permalink](#loggingmessagenotificationparams-logger) An optional name of the logger issuing this message.  data: unknown[Permalink](#loggingmessagenotificationparams-data) The data to be logged, such as a string message or an object. Any JSON serializable type is allowed here.&#x20;

## `notifications/progress`

### `ProgressNotification`

interface ProgressNotification \{\
  [jsonrpc](#progressnotification-jsonrpc): "2.0";\
  [method](#progressnotification-method): "notifications/progress";\
  [params](#progressnotification-params): [ProgressNotificationParams](#progressnotificationparams);\
}An out-of-band notification used to inform the receiver of a progress update for a long-running request.  <details> <summary>Example: Progress message[Permalink](#progressnotification-example-progress-message)</summary><pre id="progressnotification-example-progress-message">`{  "jsonrpc": "2.0",  "method": "notifications/progress",  "params": {    "progressToken": "oivaizmir",    "progress": 50,    "total": 100,    "message": "Reticulating splines..."  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#progressnotification-jsonrpc)  Inherited from [JSONRPCNotification](#jsonrpcnotification).[jsonrpc](#jsonrpcnotification-jsonrpc) method: "notifications/progress"[Permalink](#progressnotification-method)  Overrides [JSONRPCNotification](#jsonrpcnotification).[method](#jsonrpcnotification-method) params: ProgressNotificationParams[Permalink](#progressnotification-params)  Overrides [JSONRPCNotification](#jsonrpcnotification).[params](#jsonrpcnotification-params)

### `ProgressNotificationParams`

interface ProgressNotificationParams \{\
  [\_meta](#progressnotificationparams-_meta)?: [NotificationMetaObject](#notificationmetaobject);\
  [progressToken](#progressnotificationparams-progresstoken): [ProgressToken](#progresstoken);\
  [progress](#progressnotificationparams-progress): number;\
  [total](#progressnotificationparams-total)?: number;\
  [message](#progressnotificationparams-message)?: string;\
}Parameters for a [notifications/progress](#progressnotification) notification.  <details> <summary>Example: Progress message[Permalink](#progressnotificationparams-example-progress-message)</summary><pre id="progressnotificationparams-example-progress-message">`{  "progressToken": "oivaizmir",  "progress": 50,  "total": 100,  "message": "Reticulating splines..."} `Copy</pre> </details> \_meta?: NotificationMetaObject[Permalink](#progressnotificationparams-_meta)  Inherited from [NotificationParams](#notificationparams).[\_meta](#notificationparams-_meta) progressToken: ProgressToken[Permalink](#progressnotificationparams-progresstoken) The progress token which was given in the initial request, used to associate this notification with the request that is proceeding.  progress: number[Permalink](#progressnotificationparams-progress) The progress thus far. This should increase every time progress is made, even if the total is unknown.  total?: number[Permalink](#progressnotificationparams-total) Total number of items to process (or total progress required), if known.  message?: string[Permalink](#progressnotificationparams-message) An optional message describing the current progress.&#x20;

## `notifications/prompts/list_changed`

### `PromptListChangedNotification`

interface PromptListChangedNotification \{\
  [jsonrpc](#promptlistchangednotification-jsonrpc): "2.0";\
  [method](#promptlistchangednotification-method): "notifications/prompts/list\_changed";\
  [params](#promptlistchangednotification-params)?: [NotificationParams](#notificationparams);\
}An optional notification from the server to the client, informing it that the list of prompts it offers has changed. This is only delivered on a [subscriptions/listen](#subscriptionslistenrequest) stream when the client requested it via the `promptsListChanged` filter field.  <details> <summary>Example: Prompts list changed[Permalink](#promptlistchangednotification-example-prompts-list-changed)</summary><pre id="promptlistchangednotification-example-prompts-list-changed">`{  "jsonrpc": "2.0",  "method": "notifications/prompts/list_changed",  "params": {    "_meta": {      "io.modelcontextprotocol/subscriptionId": "listen-1"    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#promptlistchangednotification-jsonrpc)  Inherited from [JSONRPCNotification](#jsonrpcnotification).[jsonrpc](#jsonrpcnotification-jsonrpc) method: "notifications/prompts/list\_changed"[Permalink](#promptlistchangednotification-method)  Overrides [JSONRPCNotification](#jsonrpcnotification).[method](#jsonrpcnotification-method) params?: NotificationParams[Permalink](#promptlistchangednotification-params)  Overrides [JSONRPCNotification](#jsonrpcnotification).[params](#jsonrpcnotification-params)

## `notifications/resources/list_changed`

### `ResourceListChangedNotification`

interface ResourceListChangedNotification \{\
  [jsonrpc](#resourcelistchangednotification-jsonrpc): "2.0";\
  [method](#resourcelistchangednotification-method): "notifications/resources/list\_changed";\
  [params](#resourcelistchangednotification-params)?: [NotificationParams](#notificationparams);\
}An optional notification from the server to the client, informing it that the list of resources it can read from has changed. This is only delivered on a [subscriptions/listen](#subscriptionslistenrequest) stream when the client requested it via the `resourcesListChanged` filter field.  <details> <summary>Example: Resources list changed[Permalink](#resourcelistchangednotification-example-resources-list-changed)</summary><pre id="resourcelistchangednotification-example-resources-list-changed">`{  "jsonrpc": "2.0",  "method": "notifications/resources/list_changed",  "params": {    "_meta": {      "io.modelcontextprotocol/subscriptionId": "listen-1"    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#resourcelistchangednotification-jsonrpc)  Inherited from [JSONRPCNotification](#jsonrpcnotification).[jsonrpc](#jsonrpcnotification-jsonrpc) method: "notifications/resources/list\_changed"[Permalink](#resourcelistchangednotification-method)  Overrides [JSONRPCNotification](#jsonrpcnotification).[method](#jsonrpcnotification-method) params?: NotificationParams[Permalink](#resourcelistchangednotification-params)  Overrides [JSONRPCNotification](#jsonrpcnotification).[params](#jsonrpcnotification-params)

## `notifications/resources/updated`

### `ResourceUpdatedNotification`

interface ResourceUpdatedNotification \{\
  [jsonrpc](#resourceupdatednotification-jsonrpc): "2.0";\
  [method](#resourceupdatednotification-method): "notifications/resources/updated";\
  [params](#resourceupdatednotification-params): [ResourceUpdatedNotificationParams](#resourceupdatednotificationparams);\
}A notification from the server to the client, informing it that a resource has changed and may need to be read again. This is only sent for resources the client opted in to via the `resourceSubscriptions` field of a [subscriptions/listen](#subscriptionslistenrequest) request.  <details> <summary>Example: File resource updated notification[Permalink](#resourceupdatednotification-example-file-resource-updated-notification)</summary><pre id="resourceupdatednotification-example-file-resource-updated-notification">`{  "jsonrpc": "2.0",  "method": "notifications/resources/updated",  "params": {    "_meta": {      "io.modelcontextprotocol/subscriptionId": "listen-1"    },    "uri": "file:///project/src/main.rs"  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#resourceupdatednotification-jsonrpc)  Inherited from [JSONRPCNotification](#jsonrpcnotification).[jsonrpc](#jsonrpcnotification-jsonrpc) method: "notifications/resources/updated"[Permalink](#resourceupdatednotification-method)  Overrides [JSONRPCNotification](#jsonrpcnotification).[method](#jsonrpcnotification-method) params: ResourceUpdatedNotificationParams[Permalink](#resourceupdatednotification-params)  Overrides [JSONRPCNotification](#jsonrpcnotification).[params](#jsonrpcnotification-params)

### `ResourceUpdatedNotificationParams`

interface ResourceUpdatedNotificationParams \{\
  [\_meta](#resourceupdatednotificationparams-_meta)?: [NotificationMetaObject](#notificationmetaobject);\
  [uri](#resourceupdatednotificationparams-uri): string;\
}Parameters for a `notifications/resources/updated` notification.  <details> <summary>Example: File resource updated[Permalink](#resourceupdatednotificationparams-example-file-resource-updated)</summary><pre id="resourceupdatednotificationparams-example-file-resource-updated">`{  "uri": "file:///project/src/main.rs"} `Copy</pre> </details> \_meta?: NotificationMetaObject[Permalink](#resourceupdatednotificationparams-_meta)  Inherited from [NotificationParams](#notificationparams).[\_meta](#notificationparams-_meta) uri: string[Permalink](#resourceupdatednotificationparams-uri) The URI of the resource that has been updated. This might be a sub-resource of the one that the client actually subscribed to.&#x20;

## `notifications/subscriptions/acknowledged`

### `SubscriptionsAcknowledgedNotification`

interface SubscriptionsAcknowledgedNotification \{\
  [jsonrpc](#subscriptionsacknowledgednotification-jsonrpc): "2.0";\
  [method](#subscriptionsacknowledgednotification-method): "notifications/subscriptions/acknowledged";\
  [params](#subscriptionsacknowledgednotification-params): [SubscriptionsAcknowledgedNotificationParams](#subscriptionsacknowledgednotificationparams);\
}Sent by the server to acknowledge that a [subscriptions/listen](#subscriptionslistenrequest) subscription has been
established and to report which notification types it agreed to honor. This notification MUST be the first message the server sends carrying the
subscription's ID in `io.modelcontextprotocol/subscriptionId`. The server MUST
NOT send any notification on the subscription before acknowledging it. On
stdio, where every subscription shares one channel, this ordering is defined
per subscription ID and not per channel: messages belonging to other
subscriptions MAY be interleaved before it.  <details> <summary>Example: Listen acknowledged[Permalink](#subscriptionsacknowledgednotification-example-listen-acknowledged)</summary><pre id="subscriptionsacknowledgednotification-example-listen-acknowledged">`{  "jsonrpc": "2.0",  "method": "notifications/subscriptions/acknowledged",  "params": {    "_meta": {      "io.modelcontextprotocol/subscriptionId": "listen-1"    },    "notifications": {      "toolsListChanged": true,      "resourceSubscriptions": ["file:///project/config.json"]    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#subscriptionsacknowledgednotification-jsonrpc)  Inherited from [JSONRPCNotification](#jsonrpcnotification).[jsonrpc](#jsonrpcnotification-jsonrpc) method: "notifications/subscriptions/acknowledged"[Permalink](#subscriptionsacknowledgednotification-method)  Overrides [JSONRPCNotification](#jsonrpcnotification).[method](#jsonrpcnotification-method) params: SubscriptionsAcknowledgedNotificationParams[Permalink](#subscriptionsacknowledgednotification-params)  Overrides [JSONRPCNotification](#jsonrpcnotification).[params](#jsonrpcnotification-params)

### `SubscriptionsAcknowledgedNotificationParams`

interface SubscriptionsAcknowledgedNotificationParams \{\
  [\_meta](#subscriptionsacknowledgednotificationparams-_meta)?: [NotificationMetaObject](#notificationmetaobject);\
  [notifications](#subscriptionsacknowledgednotificationparams-notifications): [SubscriptionFilter](#subscriptionfilter);\
}Parameters for a [notifications/subscriptions/acknowledged](#subscriptionsacknowledgednotification) notification.  \_meta?: NotificationMetaObject[Permalink](#subscriptionsacknowledgednotificationparams-_meta)  Inherited from [NotificationParams](#notificationparams).[\_meta](#notificationparams-_meta) notifications: SubscriptionFilter[Permalink](#subscriptionsacknowledgednotificationparams-notifications) The subset of requested notification types the server agreed to honor.
Only includes notification types the server actually supports; if the
client requested an unsupported type (e.g., `promptsListChanged` when
the server has no prompts), it is omitted from this set.&#x20;

## `notifications/tools/list_changed`

### `ToolListChangedNotification`

interface ToolListChangedNotification \{\
  [jsonrpc](#toollistchangednotification-jsonrpc): "2.0";\
  [method](#toollistchangednotification-method): "notifications/tools/list\_changed";\
  [params](#toollistchangednotification-params)?: [NotificationParams](#notificationparams);\
}An optional notification from the server to the client, informing it that the list of tools it offers has changed. This is only delivered on a [subscriptions/listen](#subscriptionslistenrequest) stream when the client requested it via the `toolsListChanged` filter field.  <details> <summary>Example: Tools list changed[Permalink](#toollistchangednotification-example-tools-list-changed)</summary><pre id="toollistchangednotification-example-tools-list-changed">`{  "jsonrpc": "2.0",  "method": "notifications/tools/list_changed",  "params": {    "_meta": {      "io.modelcontextprotocol/subscriptionId": "listen-1"    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#toollistchangednotification-jsonrpc)  Inherited from [JSONRPCNotification](#jsonrpcnotification).[jsonrpc](#jsonrpcnotification-jsonrpc) method: "notifications/tools/list\_changed"[Permalink](#toollistchangednotification-method)  Overrides [JSONRPCNotification](#jsonrpcnotification).[method](#jsonrpcnotification-method) params?: NotificationParams[Permalink](#toollistchangednotification-params)  Overrides [JSONRPCNotification](#jsonrpcnotification).[params](#jsonrpcnotification-params)

## Multi Round-Trip

### `InputRequests`

InputRequests: anyA map of server-initiated requests that the client must fulfill.
Keys are server-assigned identifiers; values are the request objects.  <details> <summary>Example: Elicitation and sampling input requests[Permalink](#inputrequests-example-elicitation-and-sampling-input-requests)</summary><pre id="inputrequests-example-elicitation-and-sampling-input-requests">`{  "github_login": {    "method": "elicitation/create",    "params": {      "mode": "form",      "message": "Please provide your GitHub username",      "requestedSchema": {        "type": "object",        "properties": {          "name": {            "type": "string"          }        },        "required": ["name"]      }    }  },  "capital_of_france": {    "method": "sampling/createMessage",    "params": {      "messages": [        {          "role": "user",          "content": {            "type": "text",            "text": "What is the capital of France?"          }        }      ],      "maxTokens": 100    }  }} `Copy</pre> </details>

### `InputRequiredResult`

interface InputRequiredResult \{\
  [\_meta](#inputrequiredresult-_meta)?: [ResultMetaObject](#resultmetaobject);\
  [resultType](#inputrequiredresult-resulttype): string;\
  [inputRequests](#inputrequiredresult-inputrequests)?: [InputRequests](#inputrequests);\
  [requestState](#inputrequiredresult-requeststate)?: string;\
  \[key: string]: unknown;\
}An InputRequiredResult sent by the server to indicate that additional input is needed
before the request can be completed. At least one of `inputRequests` or `requestState` MUST be present.  <details> <summary>Example: InputRequiredResult with elicitation and sampling input requests and request state[Permalink](#inputrequiredresult-example-inputrequiredresult-with-elicitation-and-sampling-input-requests-and-request-state)</summary><pre id="inputrequiredresult-example-inputrequiredresult-with-elicitation-and-sampling-input-requests-and-request-state">`{  "resultType": "input_required",  "inputRequests": {    "github_login": {      "method": "elicitation/create",      "params": {        "message": "Please provide your GitHub username",        "requestedSchema": {          "type": "object",          "properties": {            "name": {              "type": "string"            }          },          "required": ["name"]        }      }    },    "capital_of_france": {      "method": "sampling/createMessage",      "params": {        "messages": [          {            "role": "user",            "content": {              "type": "text",              "text": "What is the capital of France?"            }          }        ],        "maxTokens": 100      }    }  },  "requestState": "eyJsb2NhdGlvbiI6Ik5ldyBZb3JrIn0"} `Copy</pre> </details> <details> <summary>Example: InputRequiredResult with request state only (load shedding)[Permalink](#inputrequiredresult-example-inputrequiredresult-with-request-state-only-load-shedding)</summary><pre id="inputrequiredresult-example-inputrequiredresult-with-request-state-only-load-shedding">`{  "resultType": "input_required",  "requestState": "eyJwcm9ncmVzcyI6IjUwJSIsInN0YXRlIjoicHJvY2Vzc2luZyJ9"} `Copy</pre> </details> \_meta?: ResultMetaObject[Permalink](#inputrequiredresult-_meta)  Inherited from [Result](#result).[\_meta](#result-_meta) resultType: string[Permalink](#inputrequiredresult-resulttype) Indicates the type of the result, which allows the client to determine
how to parse the result object. Servers implementing this protocol version MUST include this field.
For backward compatibility, when a client receives a result from a
server implementing an earlier protocol version (which does not include `resultType`), the client MUST treat the absent field as `"complete"`.  Inherited from [Result](#result).[resultType](#result-resulttype) inputRequests?: InputRequests[Permalink](#inputrequiredresult-inputrequests)  requestState?: string[Permalink](#inputrequiredresult-requeststate)&#x20;

### `InputResponses`

InputResponses: anyA map of client responses to server-initiated requests.
Keys correspond to the keys in the [InputRequests](#inputrequests) map;
values are the client's result for each request.  <details> <summary>Example: Elicitation and sampling input responses[Permalink](#inputresponses-example-elicitation-and-sampling-input-responses)</summary><pre id="inputresponses-example-elicitation-and-sampling-input-responses">`{  "github_login": {    "action": "accept",    "content": {      "name": "octocat"    }  },  "capital_of_france": {    "role": "assistant",    "content": {      "type": "text",      "text": "The capital of France is Paris."    },    "model": "claude-3-sonnet-20240307",    "stopReason": "endTurn"  }} `Copy</pre> </details>

## `prompts/get`

### `GetPromptRequest`

interface GetPromptRequest \{\
  [jsonrpc](#getpromptrequest-jsonrpc): "2.0";\
  [id](#getpromptrequest-id): [RequestId](#requestid);\
  [method](#getpromptrequest-method): "prompts/get";\
  [params](#getpromptrequest-params): [GetPromptRequestParams](#getpromptrequestparams);\
}Used by the client to get a prompt provided by the server.  <details> <summary>Example: Get prompt request[Permalink](#getpromptrequest-example-get-prompt-request)</summary><pre id="getpromptrequest-example-get-prompt-request">`{  "jsonrpc": "2.0",  "id": "get-prompt-example",  "method": "prompts/get",  "params": {    "_meta": {      "io.modelcontextprotocol/protocolVersion": "2026-07-28",      "io.modelcontextprotocol/clientInfo": {        "name": "ExampleClient",        "version": "1.0.0"      },      "io.modelcontextprotocol/clientCapabilities": {}    },    "name": "code_review",    "arguments": {      "code": "def hello():\n    print('world')"    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#getpromptrequest-jsonrpc)  Inherited from [JSONRPCRequest](#jsonrpcrequest).[jsonrpc](#jsonrpcrequest-jsonrpc) id: RequestId[Permalink](#getpromptrequest-id)  Inherited from [JSONRPCRequest](#jsonrpcrequest).[id](#jsonrpcrequest-id) method: "prompts/get"[Permalink](#getpromptrequest-method)  Overrides [JSONRPCRequest](#jsonrpcrequest).[method](#jsonrpcrequest-method) params: GetPromptRequestParams[Permalink](#getpromptrequest-params)  Overrides [JSONRPCRequest](#jsonrpcrequest).[params](#jsonrpcrequest-params)

### `GetPromptRequestParams`

interface GetPromptRequestParams \{\
  [\_meta](#getpromptrequestparams-_meta): [RequestMetaObject](#requestmetaobject);\
  [inputResponses](#getpromptrequestparams-inputresponses)?: [InputResponses](#inputresponses);\
  [requestState](#getpromptrequestparams-requeststate)?: string;\
  [name](#getpromptrequestparams-name): string;\
  [arguments](#getpromptrequestparams-arguments)?: \{ \[key: string]: string };\
}Parameters for a `prompts/get` request.  <details> <summary>Example: Get code review prompt[Permalink](#getpromptrequestparams-example-get-code-review-prompt)</summary><pre id="getpromptrequestparams-example-get-code-review-prompt">`{  "_meta": {    "io.modelcontextprotocol/protocolVersion": "2026-07-28",    "io.modelcontextprotocol/clientInfo": {      "name": "ExampleClient",      "version": "1.0.0"    },    "io.modelcontextprotocol/clientCapabilities": {}  },  "name": "code_review",  "arguments": {    "code": "def hello():\n    print('world')"  }} `Copy</pre> </details> \_meta: RequestMetaObject[Permalink](#getpromptrequestparams-_meta)  Inherited from [InputResponseRequestParams](#inputresponserequestparams).[\_meta](#inputresponserequestparams-_meta) inputResponses?: InputResponses[Permalink](#getpromptrequestparams-inputresponses)  Inherited from [InputResponseRequestParams](#inputresponserequestparams).[inputResponses](#inputresponserequestparams-inputresponses) requestState?: string[Permalink](#getpromptrequestparams-requeststate)  Inherited from [InputResponseRequestParams](#inputresponserequestparams).[requestState](#inputresponserequestparams-requeststate) name: string[Permalink](#getpromptrequestparams-name) The name of the prompt or prompt template.  arguments?: \{ \[key: string]: string }[Permalink](#getpromptrequestparams-arguments) Arguments to use for templating the prompt.&#x20;

### `GetPromptResultResponse`

interface GetPromptResultResponse \{\
  [jsonrpc](#getpromptresultresponse-jsonrpc): "2.0";\
  [id](#getpromptresultresponse-id): [RequestId](#requestid);\
  [result](#getpromptresultresponse-result): [InputRequiredResult](#inputrequiredresult) | [GetPromptResult](#getpromptresult);\
}A successful response from the server for a [prompts/get](#getpromptrequest) request.  <details> <summary>Example: Get prompt result response[Permalink](#getpromptresultresponse-example-get-prompt-result-response)</summary><pre id="getpromptresultresponse-example-get-prompt-result-response">`{  "jsonrpc": "2.0",  "id": "get-prompt-example",  "result": {    "resultType": "complete",    "description": "Code review prompt",    "messages": [      {        "role": "user",        "content": {          "type": "text",          "text": "Please review this Python code:\ndef hello():\n    print('world')"        }      }    ]  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#getpromptresultresponse-jsonrpc)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[jsonrpc](#jsonrpcresultresponse-jsonrpc) id: RequestId[Permalink](#getpromptresultresponse-id)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[id](#jsonrpcresultresponse-id) result: InputRequiredResult | GetPromptResult[Permalink](#getpromptresultresponse-result)  Overrides [JSONRPCResultResponse](#jsonrpcresultresponse).[result](#jsonrpcresultresponse-result)

### `GetPromptResult`

interface GetPromptResult \{\
  [\_meta](#getpromptresult-_meta)?: [ResultMetaObject](#resultmetaobject);\
  [resultType](#getpromptresult-resulttype): string;\
  [description](#getpromptresult-description)?: string;\
  [messages](#getpromptresult-messages): [PromptMessage](#promptmessage)\[];\
  \[key: string]: unknown;\
}The result returned by the server for a [prompts/get](#getpromptrequest) request.  <details> <summary>Example: Code review prompt[Permalink](#getpromptresult-example-code-review-prompt)</summary><pre id="getpromptresult-example-code-review-prompt">`{  "resultType": "complete",  "description": "Code review prompt",  "messages": [    {      "role": "user",      "content": {        "type": "text",        "text": "Please review this Python code:\ndef hello():\n    print('world')"      }    }  ]} `Copy</pre> </details> \_meta?: ResultMetaObject[Permalink](#getpromptresult-_meta)  Inherited from [Result](#result).[\_meta](#result-_meta) resultType: string[Permalink](#getpromptresult-resulttype) Indicates the type of the result, which allows the client to determine
how to parse the result object. Servers implementing this protocol version MUST include this field.
For backward compatibility, when a client receives a result from a
server implementing an earlier protocol version (which does not include `resultType`), the client MUST treat the absent field as `"complete"`.  Inherited from [Result](#result).[resultType](#result-resulttype) description?: string[Permalink](#getpromptresult-description) An optional description for the prompt.  messages: PromptMessage\[][Permalink](#getpromptresult-messages)&#x20;

### `PromptMessage`

interface PromptMessage \{\
  [role](#promptmessage-role): [Role](#role);\
  [content](#promptmessage-content): [ContentBlock](#contentblock);\
}Describes a message returned as part of a prompt. This is similar to [SamplingMessage](#samplingmessage), but also supports the embedding of
resources from the MCP server.  role: Role[Permalink](#promptmessage-role)  content: ContentBlock[Permalink](#promptmessage-content)&#x20;

## `prompts/list`

### `ListPromptsRequest`

interface ListPromptsRequest \{\
  [jsonrpc](#listpromptsrequest-jsonrpc): "2.0";\
  [id](#listpromptsrequest-id): [RequestId](#requestid);\
  [params](#listpromptsrequest-params): [PaginatedRequestParams](#paginatedrequestparams);\
  [method](#listpromptsrequest-method): "prompts/list";\
}Sent from the client to request a list of prompts and prompt templates the server has.  <details> <summary>Example: List prompts request[Permalink](#listpromptsrequest-example-list-prompts-request)</summary><pre id="listpromptsrequest-example-list-prompts-request">`{  "jsonrpc": "2.0",  "id": "list-prompts-example",  "method": "prompts/list",  "params": {    "_meta": {      "io.modelcontextprotocol/protocolVersion": "2026-07-28",      "io.modelcontextprotocol/clientInfo": {        "name": "ExampleClient",        "version": "1.0.0"      },      "io.modelcontextprotocol/clientCapabilities": {}    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#listpromptsrequest-jsonrpc)  Inherited from PaginatedRequest.jsonrpc id: RequestId[Permalink](#listpromptsrequest-id)  Inherited from PaginatedRequest.id params: PaginatedRequestParams[Permalink](#listpromptsrequest-params)  Inherited from PaginatedRequest.params method: "prompts/list"[Permalink](#listpromptsrequest-method)  Overrides PaginatedRequest.method

### `ListPromptsResultResponse`

interface ListPromptsResultResponse \{\
  [jsonrpc](#listpromptsresultresponse-jsonrpc): "2.0";\
  [id](#listpromptsresultresponse-id): [RequestId](#requestid);\
  [result](#listpromptsresultresponse-result): [ListPromptsResult](#listpromptsresult);\
}A successful response from the server for a [prompts/list](#listpromptsrequest) request.  <details> <summary>Example: List prompts result response[Permalink](#listpromptsresultresponse-example-list-prompts-result-response)</summary><pre id="listpromptsresultresponse-example-list-prompts-result-response">`{  "jsonrpc": "2.0",  "id": "list-prompts-example",  "result": {    "resultType": "complete",    "prompts": [      {        "name": "code_review",        "title": "Request Code Review",        "description": "Asks the LLM to analyze code quality and suggest improvements",        "arguments": [          {            "name": "code",            "description": "The code to review",            "required": true          }        ],        "icons": [          {            "src": "https://example.com/review-icon.svg",            "mimeType": "image/svg+xml",            "sizes": ["any"]          }        ]      }    ],    "nextCursor": "next-page-cursor",    "ttlMs": 600000,    "cacheScope": "public"  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#listpromptsresultresponse-jsonrpc)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[jsonrpc](#jsonrpcresultresponse-jsonrpc) id: RequestId[Permalink](#listpromptsresultresponse-id)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[id](#jsonrpcresultresponse-id) result: ListPromptsResult[Permalink](#listpromptsresultresponse-result)  Overrides [JSONRPCResultResponse](#jsonrpcresultresponse).[result](#jsonrpcresultresponse-result)

### `ListPromptsResult`

interface ListPromptsResult \{\
  [\_meta](#listpromptsresult-_meta)?: [ResultMetaObject](#resultmetaobject);\
  [resultType](#listpromptsresult-resulttype): string;\
  [nextCursor](#listpromptsresult-nextcursor)?: string;\
  [ttlMs](#listpromptsresult-ttlms): number;\
  [cacheScope](#listpromptsresult-cachescope): "public" | "private";\
  [prompts](#listpromptsresult-prompts): [Prompt](#prompt)\[];\
  \[key: string]: unknown;\
}The result returned by the server for a [prompts/list](#listpromptsrequest) request.  <details> <summary>Example: Prompts list with cursor and TTL[Permalink](#listpromptsresult-example-prompts-list-with-cursor-and-ttl)</summary><pre id="listpromptsresult-example-prompts-list-with-cursor-and-ttl">`{  "resultType": "complete",  "prompts": [    {      "name": "code_review",      "title": "Request Code Review",      "description": "Asks the LLM to analyze code quality and suggest improvements",      "arguments": [        {          "name": "code",          "description": "The code to review",          "required": true        }      ],      "icons": [        {          "src": "https://example.com/review-icon.svg",          "mimeType": "image/svg+xml",          "sizes": ["any"]        }      ]    }  ],  "nextCursor": "next-page-cursor",  "ttlMs": 600000,  "cacheScope": "public"} `Copy</pre> </details> \_meta?: ResultMetaObject[Permalink](#listpromptsresult-_meta)  Inherited from PaginatedResult.\_meta resultType: string[Permalink](#listpromptsresult-resulttype) Indicates the type of the result, which allows the client to determine
how to parse the result object. Servers implementing this protocol version MUST include this field.
For backward compatibility, when a client receives a result from a
server implementing an earlier protocol version (which does not include `resultType`), the client MUST treat the absent field as `"complete"`.  Inherited from PaginatedResult.resultType nextCursor?: string[Permalink](#listpromptsresult-nextcursor) An opaque token representing the pagination position after the last returned result.
If present, there may be more results available.  Inherited from PaginatedResult.nextCursor ttlMs: number[Permalink](#listpromptsresult-ttlms) A hint from the server indicating how long (in milliseconds) the
client MAY cache this response before re-fetching. Semantics are
analogous to HTTP Cache-Control max-age. <ul> <li>If 0, The response SHOULD be considered immediately stale,
The client MAY re-fetch every time the result is needed.</li> <li>If positive, the client SHOULD consider the result fresh for this many
milliseconds after receiving the response.</li> </ul>  Inherited from CacheableResult.ttlMs cacheScope: "public" | "private"[Permalink](#listpromptsresult-cachescope) Indicates the intended scope of the cached response, analogous to HTTP `Cache-Control: public` vs `Cache-Control: private`. <ul> <li>`"public"`: The response does not contain user-specific data. Any
client or intermediary (e.g., shared gateway, caching proxy) MAY cache
the response and serve it across authorization contexts.</li> <li>`"private"`: The response MAY be cached and reused only within the
same authorization context. Caches MUST NOT be shared across
authorization contexts (e.g., a different access token requires a
different cache).</li> </ul>  Inherited from CacheableResult.cacheScope prompts: Prompt\[][Permalink](#listpromptsresult-prompts)&#x20;

### `Prompt`

interface Prompt \{\
  [icons](#prompt-icons)?: [Icon](#icon)\[];\
  [name](#prompt-name): string;\
  [title](#prompt-title)?: string;\
  [description](#prompt-description)?: string;\
  [arguments](#prompt-arguments)?: [PromptArgument](#promptargument)\[];\
  [\_meta](#prompt-_meta)?: [MetaObject](#metaobject);\
}A prompt or prompt template that the server offers.  icons?: Icon\[][Permalink](#prompt-icons) Optional set of sized icons that the client can display in a user interface. Clients that support rendering icons MUST support at least the following MIME types: <ul> <li>`image/png` - PNG images (safe, universal compatibility)</li> <li>`image/jpeg` (and `image/jpg`) - JPEG images (safe, universal compatibility)</li> </ul> Clients that support rendering icons SHOULD also support: <ul> <li>`image/svg+xml` - SVG images (scalable but requires security precautions)</li> <li>`image/webp` - WebP images (modern, efficient format)</li> </ul>  Inherited from Icons.icons name: string[Permalink](#prompt-name) Intended for programmatic or logical use, but used as a display name in past specs or fallback (if title isn't present).  Inherited from BaseMetadata.name title?: string[Permalink](#prompt-title) Intended for UI and end-user contexts — optimized to be human-readable and easily understood,
even by those unfamiliar with domain-specific terminology. If not provided, the name should be used for display (except for [Tool](#tool),
where `annotations.title` should be given precedence over using `name`,
if present).  Inherited from BaseMetadata.title description?: string[Permalink](#prompt-description) An optional description of what this prompt provides  arguments?: PromptArgument\[][Permalink](#prompt-arguments) A list of arguments to use for templating the prompt.  \_meta?: MetaObject[Permalink](#prompt-_meta)&#x20;

### `PromptArgument`

interface PromptArgument \{\
  [name](#promptargument-name): string;\
  [title](#promptargument-title)?: string;\
  [description](#promptargument-description)?: string;\
  [required](#promptargument-required)?: boolean;\
}Describes an argument that a prompt can accept.  name: string[Permalink](#promptargument-name) Intended for programmatic or logical use, but used as a display name in past specs or fallback (if title isn't present).  Inherited from BaseMetadata.name title?: string[Permalink](#promptargument-title) Intended for UI and end-user contexts — optimized to be human-readable and easily understood,
even by those unfamiliar with domain-specific terminology. If not provided, the name should be used for display (except for [Tool](#tool),
where `annotations.title` should be given precedence over using `name`,
if present).  Inherited from BaseMetadata.title description?: string[Permalink](#promptargument-description) A human-readable description of the argument.  required?: boolean[Permalink](#promptargument-required) Whether this argument must be provided.&#x20;

## `resources/list`

### `ListResourcesRequest`

interface ListResourcesRequest \{\
  [jsonrpc](#listresourcesrequest-jsonrpc): "2.0";\
  [id](#listresourcesrequest-id): [RequestId](#requestid);\
  [params](#listresourcesrequest-params): [PaginatedRequestParams](#paginatedrequestparams);\
  [method](#listresourcesrequest-method): "resources/list";\
}Sent from the client to request a list of resources the server has.  <details> <summary>Example: List resources request[Permalink](#listresourcesrequest-example-list-resources-request)</summary><pre id="listresourcesrequest-example-list-resources-request">`{  "jsonrpc": "2.0",  "id": "list-resources-example",  "method": "resources/list",  "params": {    "_meta": {      "io.modelcontextprotocol/protocolVersion": "2026-07-28",      "io.modelcontextprotocol/clientInfo": {        "name": "ExampleClient",        "version": "1.0.0"      },      "io.modelcontextprotocol/clientCapabilities": {}    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#listresourcesrequest-jsonrpc)  Inherited from PaginatedRequest.jsonrpc id: RequestId[Permalink](#listresourcesrequest-id)  Inherited from PaginatedRequest.id params: PaginatedRequestParams[Permalink](#listresourcesrequest-params)  Inherited from PaginatedRequest.params method: "resources/list"[Permalink](#listresourcesrequest-method)  Overrides PaginatedRequest.method

### `ListResourcesResultResponse`

interface ListResourcesResultResponse \{\
  [jsonrpc](#listresourcesresultresponse-jsonrpc): "2.0";\
  [id](#listresourcesresultresponse-id): [RequestId](#requestid);\
  [result](#listresourcesresultresponse-result): [ListResourcesResult](#listresourcesresult);\
}A successful response from the server for a [resources/list](#listresourcesrequest) request.  <details> <summary>Example: List resources result response[Permalink](#listresourcesresultresponse-example-list-resources-result-response)</summary><pre id="listresourcesresultresponse-example-list-resources-result-response">`{  "jsonrpc": "2.0",  "id": "list-resources-example",  "result": {    "resultType": "complete",    "resources": [      {        "uri": "file:///project/src/main.rs",        "name": "main.rs",        "title": "Rust Software Application Main File",        "description": "Primary application entry point",        "mimeType": "text/x-rust",        "icons": [          {            "src": "https://example.com/rust-file-icon.png",            "mimeType": "image/png",            "sizes": ["48x48"]          }        ]      }    ],    "nextCursor": "eyJwYWdlIjogM30=",    "ttlMs": 600000,    "cacheScope": "private"  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#listresourcesresultresponse-jsonrpc)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[jsonrpc](#jsonrpcresultresponse-jsonrpc) id: RequestId[Permalink](#listresourcesresultresponse-id)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[id](#jsonrpcresultresponse-id) result: ListResourcesResult[Permalink](#listresourcesresultresponse-result)  Overrides [JSONRPCResultResponse](#jsonrpcresultresponse).[result](#jsonrpcresultresponse-result)

### `ListResourcesResult`

interface ListResourcesResult \{\
  [\_meta](#listresourcesresult-_meta)?: [ResultMetaObject](#resultmetaobject);\
  [resultType](#listresourcesresult-resulttype): string;\
  [nextCursor](#listresourcesresult-nextcursor)?: string;\
  [ttlMs](#listresourcesresult-ttlms): number;\
  [cacheScope](#listresourcesresult-cachescope): "public" | "private";\
  [resources](#listresourcesresult-resources): [Resource](#resource)\[];\
  \[key: string]: unknown;\
}The result returned by the server for a [resources/list](#listresourcesrequest) request.  <details> <summary>Example: Resources list with cursor and TTL[Permalink](#listresourcesresult-example-resources-list-with-cursor-and-ttl)</summary><pre id="listresourcesresult-example-resources-list-with-cursor-and-ttl">`{  "resultType": "complete",  "resources": [    {      "uri": "file:///project/src/main.rs",      "name": "main.rs",      "title": "Rust Software Application Main File",      "description": "Primary application entry point",      "mimeType": "text/x-rust",      "icons": [        {          "src": "https://example.com/rust-file-icon.png",          "mimeType": "image/png",          "sizes": ["48x48"]        }      ]    }  ],  "nextCursor": "eyJwYWdlIjogM30=",  "ttlMs": 600000,  "cacheScope": "private"} `Copy</pre> </details> \_meta?: ResultMetaObject[Permalink](#listresourcesresult-_meta)  Inherited from PaginatedResult.\_meta resultType: string[Permalink](#listresourcesresult-resulttype) Indicates the type of the result, which allows the client to determine
how to parse the result object. Servers implementing this protocol version MUST include this field.
For backward compatibility, when a client receives a result from a
server implementing an earlier protocol version (which does not include `resultType`), the client MUST treat the absent field as `"complete"`.  Inherited from PaginatedResult.resultType nextCursor?: string[Permalink](#listresourcesresult-nextcursor) An opaque token representing the pagination position after the last returned result.
If present, there may be more results available.  Inherited from PaginatedResult.nextCursor ttlMs: number[Permalink](#listresourcesresult-ttlms) A hint from the server indicating how long (in milliseconds) the
client MAY cache this response before re-fetching. Semantics are
analogous to HTTP Cache-Control max-age. <ul> <li>If 0, The response SHOULD be considered immediately stale,
The client MAY re-fetch every time the result is needed.</li> <li>If positive, the client SHOULD consider the result fresh for this many
milliseconds after receiving the response.</li> </ul>  Inherited from CacheableResult.ttlMs cacheScope: "public" | "private"[Permalink](#listresourcesresult-cachescope) Indicates the intended scope of the cached response, analogous to HTTP `Cache-Control: public` vs `Cache-Control: private`. <ul> <li>`"public"`: The response does not contain user-specific data. Any
client or intermediary (e.g., shared gateway, caching proxy) MAY cache
the response and serve it across authorization contexts.</li> <li>`"private"`: The response MAY be cached and reused only within the
same authorization context. Caches MUST NOT be shared across
authorization contexts (e.g., a different access token requires a
different cache).</li> </ul>  Inherited from CacheableResult.cacheScope resources: Resource\[][Permalink](#listresourcesresult-resources)&#x20;

### `Resource`

interface Resource \{\
  [icons](#resource-icons)?: [Icon](#icon)\[];\
  [name](#resource-name): string;\
  [title](#resource-title)?: string;\
  [uri](#resource-uri): string;\
  [description](#resource-description)?: string;\
  [mimeType](#resource-mimetype)?: string;\
  [annotations](#resource-annotations)?: [Annotations](#annotations);\
  [size](#resource-size)?: number;\
  [\_meta](#resource-_meta)?: [MetaObject](#metaobject);\
}A known resource that the server is capable of reading.  <details> <summary>Example: File resource with annotations[Permalink](#resource-example-file-resource-with-annotations)</summary><pre id="resource-example-file-resource-with-annotations">`{  "uri": "file:///project/README.md",  "name": "README.md",  "title": "Project Documentation",  "mimeType": "text/markdown",  "annotations": {    "audience": ["user"],    "priority": 0.8,    "lastModified": "2025-01-12T15:00:58Z"  }} `Copy</pre> </details> icons?: Icon\[][Permalink](#resource-icons) Optional set of sized icons that the client can display in a user interface. Clients that support rendering icons MUST support at least the following MIME types: <ul> <li>`image/png` - PNG images (safe, universal compatibility)</li> <li>`image/jpeg` (and `image/jpg`) - JPEG images (safe, universal compatibility)</li> </ul> Clients that support rendering icons SHOULD also support: <ul> <li>`image/svg+xml` - SVG images (scalable but requires security precautions)</li> <li>`image/webp` - WebP images (modern, efficient format)</li> </ul>  Inherited from Icons.icons name: string[Permalink](#resource-name) Intended for programmatic or logical use, but used as a display name in past specs or fallback (if title isn't present).  Inherited from BaseMetadata.name title?: string[Permalink](#resource-title) Intended for UI and end-user contexts — optimized to be human-readable and easily understood,
even by those unfamiliar with domain-specific terminology. If not provided, the name should be used for display (except for [Tool](#tool),
where `annotations.title` should be given precedence over using `name`,
if present).  Inherited from BaseMetadata.title uri: string[Permalink](#resource-uri) The URI of this resource.  description?: string[Permalink](#resource-description) A description of what this resource represents. This can be used by clients to improve the LLM's understanding of available resources. It can be thought of like a "hint" to the model.  mimeType?: string[Permalink](#resource-mimetype) The MIME type of this resource, if known.  annotations?: Annotations[Permalink](#resource-annotations) Optional annotations for the client.  size?: number[Permalink](#resource-size) The size of the raw resource content, in bytes (i.e., before base64 encoding or any tokenization), if known. This can be used by Hosts to display file sizes and estimate context window usage.  \_meta?: MetaObject[Permalink](#resource-_meta)&#x20;

## `resources/read`

### `ReadResourceRequest`

interface ReadResourceRequest \{\
  [jsonrpc](#readresourcerequest-jsonrpc): "2.0";\
  [id](#readresourcerequest-id): [RequestId](#requestid);\
  [method](#readresourcerequest-method): "resources/read";\
  [params](#readresourcerequest-params): [ReadResourceRequestParams](#readresourcerequestparams);\
}Sent from the client to the server, to read a specific resource URI.  <details> <summary>Example: Read resource request[Permalink](#readresourcerequest-example-read-resource-request)</summary><pre id="readresourcerequest-example-read-resource-request">`{  "jsonrpc": "2.0",  "id": "read-resource-example",  "method": "resources/read",  "params": {    "_meta": {      "io.modelcontextprotocol/protocolVersion": "2026-07-28",      "io.modelcontextprotocol/clientInfo": {        "name": "ExampleClient",        "version": "1.0.0"      },      "io.modelcontextprotocol/clientCapabilities": {}    },    "uri": "file:///project/src/main.rs"  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#readresourcerequest-jsonrpc)  Inherited from [JSONRPCRequest](#jsonrpcrequest).[jsonrpc](#jsonrpcrequest-jsonrpc) id: RequestId[Permalink](#readresourcerequest-id)  Inherited from [JSONRPCRequest](#jsonrpcrequest).[id](#jsonrpcrequest-id) method: "resources/read"[Permalink](#readresourcerequest-method)  Overrides [JSONRPCRequest](#jsonrpcrequest).[method](#jsonrpcrequest-method) params: ReadResourceRequestParams[Permalink](#readresourcerequest-params)  Overrides [JSONRPCRequest](#jsonrpcrequest).[params](#jsonrpcrequest-params)

### `ReadResourceRequestParams`

interface ReadResourceRequestParams \{\
  [\_meta](#readresourcerequestparams-_meta): [RequestMetaObject](#requestmetaobject);\
  [inputResponses](#readresourcerequestparams-inputresponses)?: [InputResponses](#inputresponses);\
  [requestState](#readresourcerequestparams-requeststate)?: string;\
  [uri](#readresourcerequestparams-uri): string;\
}Parameters for a `resources/read` request.  \_meta: RequestMetaObject[Permalink](#readresourcerequestparams-_meta)  Inherited from [InputResponseRequestParams](#inputresponserequestparams).[\_meta](#inputresponserequestparams-_meta) inputResponses?: InputResponses[Permalink](#readresourcerequestparams-inputresponses)  Inherited from [InputResponseRequestParams](#inputresponserequestparams).[inputResponses](#inputresponserequestparams-inputresponses) requestState?: string[Permalink](#readresourcerequestparams-requeststate)  Inherited from [InputResponseRequestParams](#inputresponserequestparams).[requestState](#inputresponserequestparams-requeststate) uri: string[Permalink](#readresourcerequestparams-uri) The URI of the resource. The URI can use any protocol; it is up to the server how to interpret it.  Inherited from ResourceRequestParams.uri

### `ReadResourceResultResponse`

interface ReadResourceResultResponse \{\
  [jsonrpc](#readresourceresultresponse-jsonrpc): "2.0";\
  [id](#readresourceresultresponse-id): [RequestId](#requestid);\
  [result](#readresourceresultresponse-result): [InputRequiredResult](#inputrequiredresult) | [ReadResourceResult](#readresourceresult);\
}A successful response from the server for a [resources/read](#readresourcerequest) request.  <details> <summary>Example: Read resource result response[Permalink](#readresourceresultresponse-example-read-resource-result-response)</summary><pre id="readresourceresultresponse-example-read-resource-result-response">`{  "jsonrpc": "2.0",  "id": "read-resource-example",  "result": {    "resultType": "complete",    "contents": [      {        "uri": "file:///project/src/main.rs",        "mimeType": "text/x-rust",        "text": "fn main() {\n    println!(\"Hello world!\");\n}"      }    ]  }} `Copy</pre> </details> <details> <summary>Example: Read resource result response with TTL[Permalink](#readresourceresultresponse-example-read-resource-result-response-with-ttl)</summary><pre id="readresourceresultresponse-example-read-resource-result-response-with-ttl">`{  "jsonrpc": "2.0",  "id": "read-resource-with-ttl-example",  "result": {    "resultType": "complete",    "contents": [      {        "uri": "file:///project/src/main.rs",        "mimeType": "text/x-rust",        "text": "fn main() {\n    println!(\"Hello world!\");\n}"      }    ],    "ttlMs": 60000,    "cacheScope": "private"  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#readresourceresultresponse-jsonrpc)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[jsonrpc](#jsonrpcresultresponse-jsonrpc) id: RequestId[Permalink](#readresourceresultresponse-id)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[id](#jsonrpcresultresponse-id) result: InputRequiredResult | ReadResourceResult[Permalink](#readresourceresultresponse-result)  Overrides [JSONRPCResultResponse](#jsonrpcresultresponse).[result](#jsonrpcresultresponse-result)

### `ReadResourceResult`

interface ReadResourceResult \{\
  [\_meta](#readresourceresult-_meta)?: [ResultMetaObject](#resultmetaobject);\
  [resultType](#readresourceresult-resulttype): string;\
  [ttlMs](#readresourceresult-ttlms): number;\
  [cacheScope](#readresourceresult-cachescope): "public" | "private";\
  [contents](#readresourceresult-contents): ([TextResourceContents](#textresourcecontents) | [BlobResourceContents](#blobresourcecontents))\[];\
  \[key: string]: unknown;\
}The result returned by the server for a [resources/read](#readresourcerequest) request.  <details> <summary>Example: File resource contents[Permalink](#readresourceresult-example-file-resource-contents)</summary><pre id="readresourceresult-example-file-resource-contents">`{  "resultType": "complete",  "contents": [    {      "uri": "file:///project/src/main.rs",      "mimeType": "text/x-rust",      "text": "fn main() {\n    println!(\"Hello world!\");\n}"    }  ],  "ttlMs": 60000,  "cacheScope": "private"} `Copy</pre> </details> \_meta?: ResultMetaObject[Permalink](#readresourceresult-_meta)  Inherited from CacheableResult.\_meta resultType: string[Permalink](#readresourceresult-resulttype) Indicates the type of the result, which allows the client to determine
how to parse the result object. Servers implementing this protocol version MUST include this field.
For backward compatibility, when a client receives a result from a
server implementing an earlier protocol version (which does not include `resultType`), the client MUST treat the absent field as `"complete"`.  Inherited from CacheableResult.resultType ttlMs: number[Permalink](#readresourceresult-ttlms) A hint from the server indicating how long (in milliseconds) the
client MAY cache this response before re-fetching. Semantics are
analogous to HTTP Cache-Control max-age. <ul> <li>If 0, The response SHOULD be considered immediately stale,
The client MAY re-fetch every time the result is needed.</li> <li>If positive, the client SHOULD consider the result fresh for this many
milliseconds after receiving the response.</li> </ul>  Inherited from CacheableResult.ttlMs cacheScope: "public" | "private"[Permalink](#readresourceresult-cachescope) Indicates the intended scope of the cached response, analogous to HTTP `Cache-Control: public` vs `Cache-Control: private`. <ul> <li>`"public"`: The response does not contain user-specific data. Any
client or intermediary (e.g., shared gateway, caching proxy) MAY cache
the response and serve it across authorization contexts.</li> <li>`"private"`: The response MAY be cached and reused only within the
same authorization context. Caches MUST NOT be shared across
authorization contexts (e.g., a different access token requires a
different cache).</li> </ul>  Inherited from CacheableResult.cacheScope contents: (TextResourceContents | BlobResourceContents)\[][Permalink](#readresourceresult-contents)&#x20;

## `resources/templates/list`

### `ListResourceTemplatesRequest`

interface ListResourceTemplatesRequest \{\
  [jsonrpc](#listresourcetemplatesrequest-jsonrpc): "2.0";\
  [id](#listresourcetemplatesrequest-id): [RequestId](#requestid);\
  [params](#listresourcetemplatesrequest-params): [PaginatedRequestParams](#paginatedrequestparams);\
  [method](#listresourcetemplatesrequest-method): "resources/templates/list";\
}Sent from the client to request a list of resource templates the server has.  <details> <summary>Example: List resource templates request[Permalink](#listresourcetemplatesrequest-example-list-resource-templates-request)</summary><pre id="listresourcetemplatesrequest-example-list-resource-templates-request">`{  "jsonrpc": "2.0",  "id": "list-resource-templates-example",  "method": "resources/templates/list",  "params": {    "_meta": {      "io.modelcontextprotocol/protocolVersion": "2026-07-28",      "io.modelcontextprotocol/clientInfo": {        "name": "ExampleClient",        "version": "1.0.0"      },      "io.modelcontextprotocol/clientCapabilities": {}    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#listresourcetemplatesrequest-jsonrpc)  Inherited from PaginatedRequest.jsonrpc id: RequestId[Permalink](#listresourcetemplatesrequest-id)  Inherited from PaginatedRequest.id params: PaginatedRequestParams[Permalink](#listresourcetemplatesrequest-params)  Inherited from PaginatedRequest.params method: "resources/templates/list"[Permalink](#listresourcetemplatesrequest-method)  Overrides PaginatedRequest.method

### `ListResourceTemplatesResultResponse`

interface ListResourceTemplatesResultResponse \{\
  [jsonrpc](#listresourcetemplatesresultresponse-jsonrpc): "2.0";\
  [id](#listresourcetemplatesresultresponse-id): [RequestId](#requestid);\
  [result](#listresourcetemplatesresultresponse-result): [ListResourceTemplatesResult](#listresourcetemplatesresult);\
}A successful response from the server for a [resources/templates/list](#listresourcetemplatesrequest) request.  <details> <summary>Example: List resource templates result response[Permalink](#listresourcetemplatesresultresponse-example-list-resource-templates-result-response)</summary><pre id="listresourcetemplatesresultresponse-example-list-resource-templates-result-response">`{  "jsonrpc": "2.0",  "id": "list-resource-templates-example",  "result": {    "resultType": "complete",    "resourceTemplates": [      {        "uriTemplate": "file:///{path}",        "name": "Project Files",        "title": "Project Files",        "description": "Access files in the project directory",        "mimeType": "application/octet-stream",        "icons": [          {            "src": "https://example.com/folder-icon.png",            "mimeType": "image/png",            "sizes": ["48x48"]          }        ]      }    ],    "ttlMs": 3600000,    "cacheScope": "public"  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#listresourcetemplatesresultresponse-jsonrpc)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[jsonrpc](#jsonrpcresultresponse-jsonrpc) id: RequestId[Permalink](#listresourcetemplatesresultresponse-id)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[id](#jsonrpcresultresponse-id) result: ListResourceTemplatesResult[Permalink](#listresourcetemplatesresultresponse-result)  Overrides [JSONRPCResultResponse](#jsonrpcresultresponse).[result](#jsonrpcresultresponse-result)

### `ListResourceTemplatesResult`

interface ListResourceTemplatesResult \{\
  [\_meta](#listresourcetemplatesresult-_meta)?: [ResultMetaObject](#resultmetaobject);\
  [resultType](#listresourcetemplatesresult-resulttype): string;\
  [nextCursor](#listresourcetemplatesresult-nextcursor)?: string;\
  [ttlMs](#listresourcetemplatesresult-ttlms): number;\
  [cacheScope](#listresourcetemplatesresult-cachescope): "public" | "private";\
  [resourceTemplates](#listresourcetemplatesresult-resourcetemplates): [ResourceTemplate](#resourcetemplate)\[];\
  \[key: string]: unknown;\
}The result returned by the server for a [resources/templates/list](#listresourcetemplatesrequest) request.  <details> <summary>Example: Resource templates list with cursor and TTL[Permalink](#listresourcetemplatesresult-example-resource-templates-list-with-cursor-and-ttl)</summary><pre id="listresourcetemplatesresult-example-resource-templates-list-with-cursor-and-ttl">`{  "resultType": "complete",  "resourceTemplates": [    {      "uriTemplate": "file:///{path}",      "name": "Project Files",      "title": "📁 Project Files",      "description": "Access files in the project directory",      "mimeType": "application/octet-stream",      "icons": [        {          "src": "https://example.com/folder-icon.png",          "mimeType": "image/png",          "sizes": ["48x48"]        }      ]    }  ],  "nextCursor": "next-page-cursor",  "ttlMs": 3600000,  "cacheScope": "public"} `Copy</pre> </details> \_meta?: ResultMetaObject[Permalink](#listresourcetemplatesresult-_meta)  Inherited from PaginatedResult.\_meta resultType: string[Permalink](#listresourcetemplatesresult-resulttype) Indicates the type of the result, which allows the client to determine
how to parse the result object. Servers implementing this protocol version MUST include this field.
For backward compatibility, when a client receives a result from a
server implementing an earlier protocol version (which does not include `resultType`), the client MUST treat the absent field as `"complete"`.  Inherited from PaginatedResult.resultType nextCursor?: string[Permalink](#listresourcetemplatesresult-nextcursor) An opaque token representing the pagination position after the last returned result.
If present, there may be more results available.  Inherited from PaginatedResult.nextCursor ttlMs: number[Permalink](#listresourcetemplatesresult-ttlms) A hint from the server indicating how long (in milliseconds) the
client MAY cache this response before re-fetching. Semantics are
analogous to HTTP Cache-Control max-age. <ul> <li>If 0, The response SHOULD be considered immediately stale,
The client MAY re-fetch every time the result is needed.</li> <li>If positive, the client SHOULD consider the result fresh for this many
milliseconds after receiving the response.</li> </ul>  Inherited from CacheableResult.ttlMs cacheScope: "public" | "private"[Permalink](#listresourcetemplatesresult-cachescope) Indicates the intended scope of the cached response, analogous to HTTP `Cache-Control: public` vs `Cache-Control: private`. <ul> <li>`"public"`: The response does not contain user-specific data. Any
client or intermediary (e.g., shared gateway, caching proxy) MAY cache
the response and serve it across authorization contexts.</li> <li>`"private"`: The response MAY be cached and reused only within the
same authorization context. Caches MUST NOT be shared across
authorization contexts (e.g., a different access token requires a
different cache).</li> </ul>  Inherited from CacheableResult.cacheScope resourceTemplates: ResourceTemplate\[][Permalink](#listresourcetemplatesresult-resourcetemplates)&#x20;

### `ResourceTemplate`

interface ResourceTemplate \{\
  [icons](#resourcetemplate-icons)?: [Icon](#icon)\[];\
  [name](#resourcetemplate-name): string;\
  [title](#resourcetemplate-title)?: string;\
  [uriTemplate](#resourcetemplate-uritemplate): string;\
  [description](#resourcetemplate-description)?: string;\
  [mimeType](#resourcetemplate-mimetype)?: string;\
  [annotations](#resourcetemplate-annotations)?: [Annotations](#annotations);\
  [\_meta](#resourcetemplate-_meta)?: [MetaObject](#metaobject);\
}A template description for resources available on the server.  icons?: Icon\[][Permalink](#resourcetemplate-icons) Optional set of sized icons that the client can display in a user interface. Clients that support rendering icons MUST support at least the following MIME types: <ul> <li>`image/png` - PNG images (safe, universal compatibility)</li> <li>`image/jpeg` (and `image/jpg`) - JPEG images (safe, universal compatibility)</li> </ul> Clients that support rendering icons SHOULD also support: <ul> <li>`image/svg+xml` - SVG images (scalable but requires security precautions)</li> <li>`image/webp` - WebP images (modern, efficient format)</li> </ul>  Inherited from Icons.icons name: string[Permalink](#resourcetemplate-name) Intended for programmatic or logical use, but used as a display name in past specs or fallback (if title isn't present).  Inherited from BaseMetadata.name title?: string[Permalink](#resourcetemplate-title) Intended for UI and end-user contexts — optimized to be human-readable and easily understood,
even by those unfamiliar with domain-specific terminology. If not provided, the name should be used for display (except for [Tool](#tool),
where `annotations.title` should be given precedence over using `name`,
if present).  Inherited from BaseMetadata.title uriTemplate: string[Permalink](#resourcetemplate-uritemplate) A URI template (according to RFC 6570) that can be used to construct resource URIs.  description?: string[Permalink](#resourcetemplate-description) A description of what this template is for. This can be used by clients to improve the LLM's understanding of available resources. It can be thought of like a "hint" to the model.  mimeType?: string[Permalink](#resourcetemplate-mimetype) The MIME type for all resources that match this template. This should only be included if all resources matching this template have the same type.  annotations?: Annotations[Permalink](#resourcetemplate-annotations) Optional annotations for the client.  \_meta?: MetaObject[Permalink](#resourcetemplate-_meta)&#x20;

## `roots/list`

### `ListRootsRequest`

interface ListRootsRequest \{\
  [method](#listrootsrequest-method): "roots/list";\
  [params](#listrootsrequest-params)?: \{ \_meta?: [MetaObject](#metaobject) };\
}Sent from the server to request a list of root URIs from the client. Roots allow
servers to ask for specific directories or files to operate on. A common example
for roots is providing a set of repositories or directories a server should operate
on. This request is typically used when the server needs to understand the file system
structure or access specific locations that the client has permission to read from.  <details> <summary>Example: List roots request[Permalink](#listrootsrequest-example-list-roots-request)</summary><pre id="listrootsrequest-example-list-roots-request">`{  "id": "list-roots-example",  "method": "roots/list"} `Copy</pre> </details>  DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.  method: "roots/list"[Permalink](#listrootsrequest-method)  params?: \{ \_meta?: MetaObject }[Permalink](#listrootsrequest-params)&#x20;

### `ListRootsResult`

interface ListRootsResult \{\
  [roots](#listrootsresult-roots): [Root](#root)\[];\
}The result returned by the client for a [roots/list](#listrootsrequest) request.
This result contains an array of [Root](#root) objects, each representing a root directory
or file that the server can operate on.  <details> <summary>Example: Single root directory[Permalink](#listrootsresult-example-single-root-directory)</summary><pre id="listrootsresult-example-single-root-directory">`{  "roots": [    {      "uri": "file:///home/user/projects/myproject",      "name": "My Project"    }  ]} `Copy</pre> </details> <details> <summary>Example: Multiple root directories[Permalink](#listrootsresult-example-multiple-root-directories)</summary><pre id="listrootsresult-example-multiple-root-directories">`{  "roots": [    {      "uri": "file:///home/user/repos/frontend",      "name": "Frontend Repository"    },    {      "uri": "file:///home/user/repos/backend",      "name": "Backend Repository"    }  ]} `Copy</pre> </details>  DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.  roots: Root\[][Permalink](#listrootsresult-roots)&#x20;

### `Root`

interface Root \{\
  [uri](#root-uri): string;\
  [name](#root-name)?: string;\
  [\_meta](#root-_meta)?: [MetaObject](#metaobject);\
}Represents a root directory or file that the server can operate on.  <details> <summary>Example: Project directory root[Permalink](#root-example-project-directory-root)</summary><pre id="root-example-project-directory-root">`{  "uri": "file:///home/user/projects/myproject",  "name": "My Project"} `Copy</pre> </details>  DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.  uri: string[Permalink](#root-uri) The URI identifying the root. This *must* start with `file://` for now.
This restriction may be relaxed in future versions of the protocol to allow
other URI schemes.  name?: string[Permalink](#root-name) An optional name for the root. This can be used to provide a human-readable
identifier for the root, which may be useful for display purposes or for
referencing the root in other parts of the application.  \_meta?: MetaObject[Permalink](#root-_meta)&#x20;

## `sampling/createMessage`

### `CreateMessageRequest`

interface CreateMessageRequest \{\
  [method](#createmessagerequest-method): "sampling/createMessage";\
  [params](#createmessagerequest-params): [CreateMessageRequestParams](#createmessagerequestparams);\
}A request from the server to sample an LLM via the client. The client has full discretion over which model to select. The client should also inform the user before beginning sampling, to allow them to inspect the request (human in the loop) and decide whether to approve it.  <details> <summary>Example: Sampling request[Permalink](#createmessagerequest-example-sampling-request)</summary><pre id="createmessagerequest-example-sampling-request">`{  "method": "sampling/createMessage",  "params": {    "messages": [      {        "role": "user",        "content": {          "type": "text",          "text": "What is the capital of France?"        }      }    ],    "modelPreferences": {      "hints": [        {          "name": "claude-3-sonnet"        }      ],      "intelligencePriority": 0.8,      "speedPriority": 0.5    },    "systemPrompt": "You are a helpful assistant.",    "maxTokens": 100  }} `Copy</pre> </details>  DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.  method: "sampling/createMessage"[Permalink](#createmessagerequest-method)  params: CreateMessageRequestParams[Permalink](#createmessagerequest-params)&#x20;

### `CreateMessageRequestParams`

interface CreateMessageRequestParams \{\
  [messages](#createmessagerequestparams-messages): [SamplingMessage](#samplingmessage)\[];\
  [modelPreferences](#createmessagerequestparams-modelpreferences)?: [ModelPreferences](#modelpreferences);\
  [systemPrompt](#createmessagerequestparams-systemprompt)?: string;\
  [includeContext](#createmessagerequestparams-includecontext)?: "none" | "thisServer" | "allServers";\
  [temperature](#createmessagerequestparams-temperature)?: number;\
  [maxTokens](#createmessagerequestparams-maxtokens): number;\
  [stopSequences](#createmessagerequestparams-stopsequences)?: string\[];\
  [metadata](#createmessagerequestparams-metadata)?: [JSONObject](#jsonobject);\
  [tools](#createmessagerequestparams-tools)?: [Tool](#tool)\[];\
  [toolChoice](#createmessagerequestparams-toolchoice)?: [ToolChoice](#toolchoice);\
}Parameters for a `sampling/createMessage` request.  <details> <summary>Example: Basic request[Permalink](#createmessagerequestparams-example-basic-request)</summary><pre id="createmessagerequestparams-example-basic-request">`{  "messages": [    {      "role": "user",      "content": {        "type": "text",        "text": "What is the capital of France?"      }    }  ],  "modelPreferences": {    "hints": [      {        "name": "claude-3-sonnet"      }    ],    "intelligencePriority": 0.8,    "speedPriority": 0.5  },  "systemPrompt": "You are a helpful assistant.",  "maxTokens": 100} `Copy</pre> </details> <details> <summary>Example: Request with tools[Permalink](#createmessagerequestparams-example-request-with-tools)</summary><pre id="createmessagerequestparams-example-request-with-tools">`{  "messages": [    {      "role": "user",      "content": {        "type": "text",        "text": "What's the weather like in Paris and London?"      }    }  ],  "tools": [    {      "name": "get_weather",      "description": "Get current weather for a city",      "inputSchema": {        "type": "object",        "properties": {          "city": {            "type": "string",            "description": "City name"          }        },        "required": ["city"]      }    }  ],  "toolChoice": {    "mode": "auto"  },  "maxTokens": 1000} `Copy</pre> </details> <details> <summary>Example: Follow-up request with tool results[Permalink](#createmessagerequestparams-example-follow-up-request-with-tool-results)</summary><pre id="createmessagerequestparams-example-follow-up-request-with-tool-results">`{  "messages": [    {      "role": "user",      "content": {        "type": "text",        "text": "What's the weather like in Paris and London?"      }    },    {      "role": "assistant",      "content": [        {          "type": "tool_use",          "id": "call_abc123",          "name": "get_weather",          "input": { "city": "Paris" }        },        {          "type": "tool_use",          "id": "call_def456",          "name": "get_weather",          "input": { "city": "London" }        }      ]    },    {      "role": "user",      "content": [        {          "type": "tool_result",          "toolUseId": "call_abc123",          "content": [            {              "type": "text",              "text": "Weather in Paris: 18°C, partly cloudy"            }          ]        },        {          "type": "tool_result",          "toolUseId": "call_def456",          "content": [            {              "type": "text",              "text": "Weather in London: 15°C, rainy"            }          ]        }      ]    }  ],  "tools": [    {      "name": "get_weather",      "description": "Get current weather for a city",      "inputSchema": {        "type": "object",        "properties": {          "city": { "type": "string" }        },        "required": ["city"]      }    }  ],  "maxTokens": 1000} `Copy</pre> </details>  DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.  messages: SamplingMessage\[][Permalink](#createmessagerequestparams-messages)  modelPreferences?: ModelPreferences[Permalink](#createmessagerequestparams-modelpreferences) The server's preferences for which model to select. The client MAY ignore these preferences.  systemPrompt?: string[Permalink](#createmessagerequestparams-systemprompt) An optional system prompt the server wants to use for sampling. The client MAY modify or omit this prompt.  includeContext?: "none" | "thisServer" | "allServers"[Permalink](#createmessagerequestparams-includecontext) A request to include context from one or more MCP servers (including the caller), to be attached to the prompt.
The client MAY ignore this request. Default is `"none"`. The values `"thisServer"` and `"allServers"` are deprecated (SEP-2596): servers SHOULD
omit this field or use `"none"`, and SHOULD only use the deprecated values if the client declares [ClientCapabilities.sampling.context](#clientcapabilities-sampling).    DeprecatedThe `"thisServer"` and `"allServers"` values are deprecated as of protocol version 2025-11-25
(SEP-2596) and will be removed no later than the Sampling feature itself (SEP-2577). Omit this field or use `"none"`.  temperature?: number[Permalink](#createmessagerequestparams-temperature)  maxTokens: number[Permalink](#createmessagerequestparams-maxtokens) The requested maximum number of tokens to sample (to prevent runaway completions). The client MAY choose to sample fewer tokens than the requested maximum.  stopSequences?: string\[][Permalink](#createmessagerequestparams-stopsequences)  metadata?: JSONObject[Permalink](#createmessagerequestparams-metadata) Optional metadata to pass through to the LLM provider. The format of this metadata is provider-specific.  tools?: Tool\[][Permalink](#createmessagerequestparams-tools) Tools that the model may use during generation.
The client MUST return an error if this field is provided but [ClientCapabilities.sampling.tools](#clientcapabilities-sampling) is not declared.  toolChoice?: ToolChoice[Permalink](#createmessagerequestparams-toolchoice) Controls how the model uses tools.
The client MUST return an error if this field is provided but [ClientCapabilities.sampling.tools](#clientcapabilities-sampling) is not declared.
Default is `{ mode: "auto" }`.&#x20;

### `CreateMessageResult`

interface CreateMessageResult \{\
  [model](#createmessageresult-model): string;\
  [stopReason](#createmessageresult-stopreason)?: string;\
  [role](#createmessageresult-role): [Role](#role);\
  [content](#createmessageresult-content): [SamplingMessageContentBlock](#samplingmessagecontentblock) | [SamplingMessageContentBlock](#samplingmessagecontentblock)\[];\
  [\_meta](#createmessageresult-_meta)?: [MetaObject](#metaobject);\
}The result returned by the client for a [sampling/createMessage](#createmessagerequest) request.
The client should inform the user before returning the sampled message, to allow them
to inspect the response (human in the loop) and decide whether to allow the server to see it.  <details> <summary>Example: Text response[Permalink](#createmessageresult-example-text-response)</summary><pre id="createmessageresult-example-text-response">`{  "role": "assistant",  "content": {    "type": "text",    "text": "The capital of France is Paris."  },  "model": "claude-3-sonnet-20240307",  "stopReason": "endTurn"} `Copy</pre> </details> <details> <summary>Example: Tool use response[Permalink](#createmessageresult-example-tool-use-response)</summary><pre id="createmessageresult-example-tool-use-response">`{  "role": "assistant",  "content": [    {      "type": "tool_use",      "id": "call_abc123",      "name": "get_weather",      "input": {        "city": "Paris"      }    },    {      "type": "tool_use",      "id": "call_def456",      "name": "get_weather",      "input": {        "city": "London"      }    }  ],  "model": "claude-3-sonnet-20240307",  "stopReason": "toolUse"} `Copy</pre> </details> <details> <summary>Example: Final response after tool use[Permalink](#createmessageresult-example-final-response-after-tool-use)</summary><pre id="createmessageresult-example-final-response-after-tool-use">`{  "role": "assistant",  "content": {    "type": "text",    "text": "Based on the current weather data:\n\n- **Paris**: 18°C and partly cloudy - quite pleasant!\n- **London**: 15°C and rainy - you'll want an umbrella.\n\nParis has slightly warmer and drier conditions today."  },  "model": "claude-3-sonnet-20240307",  "stopReason": "endTurn"} `Copy</pre> </details>  DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.  model: string[Permalink](#createmessageresult-model) The name of the model that generated the message.  stopReason?: string[Permalink](#createmessageresult-stopreason) The reason why sampling stopped, if known. Standard values: <ul> <li>`"endTurn"`: Natural end of the assistant's turn</li> <li>`"stopSequence"`: A stop sequence was encountered</li> <li>`"maxTokens"`: Maximum token limit was reached</li> <li>`"toolUse"`: The model wants to use one or more tools</li> </ul> This field is an open string to allow for provider-specific stop reasons.  role: Role[Permalink](#createmessageresult-role)  Inherited from [SamplingMessage](#samplingmessage).[role](#samplingmessage-role) content: SamplingMessageContentBlock | SamplingMessageContentBlock\[][Permalink](#createmessageresult-content)  Inherited from [SamplingMessage](#samplingmessage).[content](#samplingmessage-content) \_meta?: MetaObject[Permalink](#createmessageresult-_meta)  Inherited from [SamplingMessage](#samplingmessage).[\_meta](#samplingmessage-_meta)

### `ModelHint`

interface ModelHint \{\
  [name](#modelhint-name)?: string;\
}Hints to use for model selection. Keys not declared here are currently left unspecified by the spec and are up
to the client to interpret.   DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.  name?: string[Permalink](#modelhint-name) A hint for a model name. The client SHOULD treat this as a substring of a model name; for example: <ul> <li>`claude-3-5-sonnet` should match `claude-3-5-sonnet-20241022`</li> <li>`sonnet` should match `claude-3-5-sonnet-20241022`, `claude-3-sonnet-20240229`, etc.</li> <li>`claude` should match any Claude model</li> </ul> The client MAY also map the string to a different provider's model name or a different model family, as long as it fills a similar niche; for example: <ul> <li>`gemini-1.5-flash` could match `claude-3-haiku-20240307`</li> </ul>&#x20;

### `ModelPreferences`

interface ModelPreferences \{\
  [hints](#modelpreferences-hints)?: [ModelHint](#modelhint)\[];\
  [costPriority](#modelpreferences-costpriority)?: number;\
  [speedPriority](#modelpreferences-speedpriority)?: number;\
  [intelligencePriority](#modelpreferences-intelligencepriority)?: number;\
}The server's preferences for model selection, requested of the client during sampling. Because LLMs can vary along multiple dimensions, choosing the "best" model is
rarely straightforward.  Different models excel in different areas—some are
faster but less capable, others are more capable but more expensive, and so
on. This interface allows servers to express their priorities across multiple
dimensions to help clients make an appropriate selection for their use case. These preferences are always advisory. The client MAY ignore them. It is also
up to the client to decide how to interpret these preferences and how to
balance them against other considerations.  <details> <summary>Example: With hints and priorities[Permalink](#modelpreferences-example-with-hints-and-priorities)</summary><pre id="modelpreferences-example-with-hints-and-priorities">`{  "hints": [    { "name": "claude-3-sonnet" },    { "name": "claude" }  ],  "costPriority": 0.3,  "speedPriority": 0.8,  "intelligencePriority": 0.5} `Copy</pre> </details>  DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.  hints?: ModelHint\[][Permalink](#modelpreferences-hints) Optional hints to use for model selection. If multiple hints are specified, the client MUST evaluate them in order
(such that the first match is taken). The client SHOULD prioritize these hints over the numeric priorities, but
MAY still use the priorities to select from ambiguous matches.  costPriority?: number[Permalink](#modelpreferences-costpriority) How much to prioritize cost when selecting a model. A value of 0 means cost
is not important, while a value of 1 means cost is the most important
factor.  speedPriority?: number[Permalink](#modelpreferences-speedpriority) How much to prioritize sampling speed (latency) when selecting a model. A
value of 0 means speed is not important, while a value of 1 means speed is
the most important factor.  intelligencePriority?: number[Permalink](#modelpreferences-intelligencepriority) How much to prioritize intelligence and capabilities when selecting a
model. A value of 0 means intelligence is not important, while a value of 1
means intelligence is the most important factor.&#x20;

### `SamplingMessage`

interface SamplingMessage \{\
  [role](#samplingmessage-role): [Role](#role);\
  [content](#samplingmessage-content): [SamplingMessageContentBlock](#samplingmessagecontentblock) | [SamplingMessageContentBlock](#samplingmessagecontentblock)\[];\
  [\_meta](#samplingmessage-_meta)?: [MetaObject](#metaobject);\
}Describes a message issued to or received from an LLM API.  <details> <summary>Example: Single content block[Permalink](#samplingmessage-example-single-content-block)</summary><pre id="samplingmessage-example-single-content-block">`{  "role": "user",  "content": {    "type": "text",    "text": "What is the capital of France?"  }} `Copy</pre> </details> <details> <summary>Example: Multiple content blocks[Permalink](#samplingmessage-example-multiple-content-blocks)</summary><pre id="samplingmessage-example-multiple-content-blocks">`{  "role": "user",  "content": [    {      "type": "tool_result",      "toolUseId": "call_123",      "content": [{ "type": "text", "text": "Result 1" }]    },    {      "type": "tool_result",      "toolUseId": "call_456",      "content": [{ "type": "text", "text": "Result 2" }]    }  ]} `Copy</pre> </details>  DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.  role: Role[Permalink](#samplingmessage-role)  content: SamplingMessageContentBlock | SamplingMessageContentBlock\[][Permalink](#samplingmessage-content)  \_meta?: MetaObject[Permalink](#samplingmessage-_meta)&#x20;

### `SamplingMessageContentBlock`

SamplingMessageContentBlock:\
  | [TextContent](#textcontent)\
  | [ImageContent](#imagecontent)\
  | [AudioContent](#audiocontent)\
  | [ToolUseContent](#toolusecontent)\
  | [ToolResultContent](#toolresultcontent)  DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.&#x20;

### `ToolChoice`

interface ToolChoice \{\
  [mode](#toolchoice-mode)?: "none" | "required" | "auto";\
}Controls tool selection behavior for sampling requests.   DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.  mode?: "none" | "required" | "auto"[Permalink](#toolchoice-mode) Controls the tool use ability of the model: <ul> <li>`"auto"`: Model decides whether to use tools (default)</li> <li>`"required"`: Model MUST use at least one tool before completing</li> <li>`"none"`: Model MUST NOT use any tools</li> </ul>&#x20;

### `ToolResultContent`

interface ToolResultContent \{\
  [type](#toolresultcontent-type): "tool\_result";\
  [toolUseId](#toolresultcontent-tooluseid): string;\
  [content](#toolresultcontent-content): [ContentBlock](#contentblock)\[];\
  [structuredContent](#toolresultcontent-structuredcontent)?: unknown;\
  [isError](#toolresultcontent-iserror)?: boolean;\
  [\_meta](#toolresultcontent-_meta)?: [MetaObject](#metaobject);\
}The result of a tool use, provided by the user back to the assistant.  <details> <summary>Example: \`get\_weather\` tool result[Permalink](#toolresultcontent-example-get_weather-tool-result)</summary><pre id="toolresultcontent-example-get_weather-tool-result">`{  "type": "tool_result",  "toolUseId": "call_abc123",  "content": [    {      "type": "text",      "text": "Weather in Paris: 18°C, partly cloudy"    }  ]} `Copy</pre> </details>  DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.  type: "tool\_result"[Permalink](#toolresultcontent-type)  toolUseId: string[Permalink](#toolresultcontent-tooluseid) The ID of the tool use this result corresponds to. This MUST match the ID from a previous [ToolUseContent](#toolusecontent).  content: ContentBlock\[][Permalink](#toolresultcontent-content) The unstructured result content of the tool use. This has the same format as [CallToolResult.content](#calltoolresult-content) and can include text, images,
audio, resource links, and embedded resources.  structuredContent?: unknown[Permalink](#toolresultcontent-structuredcontent) An optional structured result value. This can be any JSON value (object, array, string, number, boolean, or null).
If the tool defined an [Tool.outputSchema](#tool-outputschema), this SHOULD conform to that schema.  isError?: boolean[Permalink](#toolresultcontent-iserror) Whether the tool use resulted in an error. If true, the content typically describes the error that occurred.
Default: false  \_meta?: MetaObject[Permalink](#toolresultcontent-_meta) Optional metadata about the tool result. Clients SHOULD preserve this field when
including tool results in subsequent sampling requests to enable caching optimizations.&#x20;

### `ToolUseContent`

interface ToolUseContent \{\
  [type](#toolusecontent-type): "tool\_use";\
  [id](#toolusecontent-id): string;\
  [name](#toolusecontent-name): string;\
  [input](#toolusecontent-input): \{ \[key: string]: unknown };\
  [\_meta](#toolusecontent-_meta)?: [MetaObject](#metaobject);\
}A request from the assistant to call a tool.  <details> <summary>Example: \`get\_weather\` tool use[Permalink](#toolusecontent-example-get_weather-tool-use)</summary><pre id="toolusecontent-example-get_weather-tool-use">`{  "type": "tool_use",  "id": "call_abc123",  "name": "get_weather",  "input": {    "city": "Paris"  }} `Copy</pre> </details>  DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.  type: "tool\_use"[Permalink](#toolusecontent-type)  id: string[Permalink](#toolusecontent-id) A unique identifier for this tool use. This ID is used to match tool results to their corresponding tool uses.  name: string[Permalink](#toolusecontent-name) The name of the tool to call.  input: \{ \[key: string]: unknown }[Permalink](#toolusecontent-input) The arguments to pass to the tool, conforming to the tool's input schema.  \_meta?: MetaObject[Permalink](#toolusecontent-_meta) Optional metadata about the tool use. Clients SHOULD preserve this field when
including tool uses in subsequent sampling requests to enable caching optimizations.&#x20;

## `server/discover`

### `DiscoverRequest`

interface DiscoverRequest \{\
  [jsonrpc](#discoverrequest-jsonrpc): "2.0";\
  [id](#discoverrequest-id): [RequestId](#requestid);\
  [method](#discoverrequest-method): "server/discover";\
  [params](#discoverrequest-params): [RequestParams](#requestparams);\
}A request from the client asking the server to advertise its supported
protocol versions, capabilities, and other metadata. Servers **MUST**
implement `server/discover`. Clients **MAY** call it but are not required
to — version negotiation can also happen inline via per-request `_meta`.  <details> <summary>Example: Discover request[Permalink](#discoverrequest-example-discover-request)</summary><pre id="discoverrequest-example-discover-request">`{  "jsonrpc": "2.0",  "id": "discover-1",  "method": "server/discover",  "params": {    "_meta": {      "io.modelcontextprotocol/protocolVersion": "2026-07-28",      "io.modelcontextprotocol/clientInfo": {        "name": "ExampleClient",        "version": "1.0.0"      },      "io.modelcontextprotocol/clientCapabilities": {}    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#discoverrequest-jsonrpc)  Inherited from [JSONRPCRequest](#jsonrpcrequest).[jsonrpc](#jsonrpcrequest-jsonrpc) id: RequestId[Permalink](#discoverrequest-id)  Inherited from [JSONRPCRequest](#jsonrpcrequest).[id](#jsonrpcrequest-id) method: "server/discover"[Permalink](#discoverrequest-method)  Overrides [JSONRPCRequest](#jsonrpcrequest).[method](#jsonrpcrequest-method) params: RequestParams[Permalink](#discoverrequest-params)  Overrides [JSONRPCRequest](#jsonrpcrequest).[params](#jsonrpcrequest-params)

### `DiscoverResultResponse`

interface DiscoverResultResponse \{\
  [jsonrpc](#discoverresultresponse-jsonrpc): "2.0";\
  [id](#discoverresultresponse-id): [RequestId](#requestid);\
  [result](#discoverresultresponse-result): [DiscoverResult](#discoverresult);\
}A successful response from the server for a [server/discover](#discoverrequest) request.  <details> <summary>Example: Discover result response[Permalink](#discoverresultresponse-example-discover-result-response)</summary><pre id="discoverresultresponse-example-discover-result-response">`{  "jsonrpc": "2.0",  "id": "discover-1",  "result": {    "resultType": "complete",    "supportedVersions": ["2026-07-28"],    "capabilities": {      "tools": {},      "resources": {}    },    "_meta": {      "io.modelcontextprotocol/serverInfo": {        "name": "ExampleServer",        "version": "1.0.0"      }    },    "ttlMs": 3600000,    "cacheScope": "public"  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#discoverresultresponse-jsonrpc)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[jsonrpc](#jsonrpcresultresponse-jsonrpc) id: RequestId[Permalink](#discoverresultresponse-id)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[id](#jsonrpcresultresponse-id) result: DiscoverResult[Permalink](#discoverresultresponse-result)  Overrides [JSONRPCResultResponse](#jsonrpcresultresponse).[result](#jsonrpcresultresponse-result)

### `DiscoverResult`

interface DiscoverResult \{\
  [\_meta](#discoverresult-_meta)?: [ResultMetaObject](#resultmetaobject);\
  [resultType](#discoverresult-resulttype): string;\
  [supportedVersions](#discoverresult-supportedversions): string\[];\
  [capabilities](#discoverresult-capabilities): [ServerCapabilities](#servercapabilities);\
  [instructions](#discoverresult-instructions)?: string;\
  [ttlMs](#discoverresult-ttlms): number;\
  [cacheScope](#discoverresult-cachescope): "public" | "private";\
  \[key: string]: unknown;\
}The result returned by the server for a [server/discover](#discoverrequest) request.  <details> <summary>Example: Server capabilities discovery[Permalink](#discoverresult-example-server-capabilities-discovery)</summary><pre id="discoverresult-example-server-capabilities-discovery">``{  "resultType": "complete",  "supportedVersions": ["2026-07-28"],  "capabilities": {    "tools": {},    "resources": {}  },  "_meta": {    "io.modelcontextprotocol/serverInfo": {      "name": "ExampleServer",      "version": "1.0.0"    }  },  "instructions": "This server provides weather and resource utilities. Prefer `get_weather` for forecast lookups.",  "ttlMs": 3600000,  "cacheScope": "public"} ``Copy</pre> </details> \_meta?: ResultMetaObject[Permalink](#discoverresult-_meta)  Inherited from CacheableResult.\_meta resultType: string[Permalink](#discoverresult-resulttype) Indicates the type of the result, which allows the client to determine
how to parse the result object. Servers implementing this protocol version MUST include this field.
For backward compatibility, when a client receives a result from a
server implementing an earlier protocol version (which does not include `resultType`), the client MUST treat the absent field as `"complete"`.  Inherited from CacheableResult.resultType supportedVersions: string\[][Permalink](#discoverresult-supportedversions) MCP Protocol Versions this server supports. The client should choose a
version from this list for use in subsequent requests.  capabilities: ServerCapabilities[Permalink](#discoverresult-capabilities) The capabilities of the server.  instructions?: string[Permalink](#discoverresult-instructions) Natural-language guidance describing the server and its features. This can be used by clients to improve an LLM's understanding of
available tools (e.g., by including it in a system prompt). It should
focus on information that helps the model use the server effectively
and should not duplicate information already in tool descriptions.  ttlMs: number[Permalink](#discoverresult-ttlms) A hint from the server indicating how long (in milliseconds) the
client MAY cache this response before re-fetching. Semantics are
analogous to HTTP Cache-Control max-age. <ul> <li>If 0, The response SHOULD be considered immediately stale,
The client MAY re-fetch every time the result is needed.</li> <li>If positive, the client SHOULD consider the result fresh for this many
milliseconds after receiving the response.</li> </ul>  Inherited from CacheableResult.ttlMs cacheScope: "public" | "private"[Permalink](#discoverresult-cachescope) Indicates the intended scope of the cached response, analogous to HTTP `Cache-Control: public` vs `Cache-Control: private`. <ul> <li>`"public"`: The response does not contain user-specific data. Any
client or intermediary (e.g., shared gateway, caching proxy) MAY cache
the response and serve it across authorization contexts.</li> <li>`"private"`: The response MAY be cached and reused only within the
same authorization context. Caches MUST NOT be shared across
authorization contexts (e.g., a different access token requires a
different cache).</li> </ul>  Inherited from CacheableResult.cacheScope

### `ClientCapabilities`

interface ClientCapabilities \{\
  [experimental](#clientcapabilities-experimental)?: \{ \[key: string]: [JSONObject](#jsonobject) };\
  [roots](#clientcapabilities-roots)?: \{};\
  [sampling](#clientcapabilities-sampling)?: \{ context?: [JSONObject](#jsonobject); tools?: [JSONObject](#jsonobject) };\
  [elicitation](#clientcapabilities-elicitation)?: \{ form?: [JSONObject](#jsonobject); url?: [JSONObject](#jsonobject) };\
  [extensions](#clientcapabilities-extensions)?: \{ \[key: string]: [JSONObject](#jsonobject) };\
}Capabilities a client may support. Known capabilities are defined here, in this schema, but this is not a closed set: any client can define its own, additional capabilities.  experimental?: \{ \[key: string]: JSONObject }[Permalink](#clientcapabilities-experimental) Experimental, non-standard capabilities that the client supports.  roots?: \{}[Permalink](#clientcapabilities-roots) Present if the client supports listing roots.    DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.  <details> <summary>Example: Roots — minimum baseline support[Permalink](#clientcapabilities-example-roots-minimum-baseline-support)</summary><pre id="clientcapabilities-example-roots-minimum-baseline-support">`{  "roots": {}} `Copy</pre> </details> sampling?: \{ context?: JSONObject; tools?: JSONObject }[Permalink](#clientcapabilities-sampling) Present if the client supports sampling from an LLM.   Type Declaration <ul> <li> `Optional`context?: [JSONObject](#jsonobject) Whether the client supports context inclusion via `includeContext` parameter.
If not declared, servers SHOULD only use `includeContext: "none"` (or omit it). </li> <li> `Optional`tools?: [JSONObject](#jsonobject) Whether the client supports tool use via `tools` and `toolChoice` parameters. </li></ul>   DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.  <details> <summary>Example: Sampling — minimum baseline support[Permalink](#clientcapabilities-example-sampling-minimum-baseline-support)</summary><pre id="clientcapabilities-example-sampling-minimum-baseline-support">`{  "sampling": {}} `Copy</pre> </details> <details> <summary>Example: Sampling — tool use support[Permalink](#clientcapabilities-example-sampling-tool-use-support)</summary><pre id="clientcapabilities-example-sampling-tool-use-support">`{  "sampling": {    "tools": {}  }} `Copy</pre> </details> <details> <summary>Example: Sampling — context inclusion support (deprecated)[Permalink](#clientcapabilities-example-sampling-context-inclusion-support-deprecated)</summary><pre id="clientcapabilities-example-sampling-context-inclusion-support-deprecated">`{  "sampling": {    "context": {}  }} `Copy</pre> </details> elicitation?: \{ form?: JSONObject; url?: JSONObject }[Permalink](#clientcapabilities-elicitation) Present if the client supports elicitation from the server.   <details> <summary>Example: Elicitation — form and URL mode support[Permalink](#clientcapabilities-example-elicitation-form-and-url-mode-support)</summary><pre id="clientcapabilities-example-elicitation-form-and-url-mode-support">`{  "elicitation": {    "form": {},    "url": {}  }} `Copy</pre> </details> <details> <summary>Example: Elicitation — form mode only (implicit)[Permalink](#clientcapabilities-example-elicitation-form-mode-only-implicit)</summary><pre id="clientcapabilities-example-elicitation-form-mode-only-implicit">`{  "elicitation": {}} `Copy</pre> </details> extensions?: \{ \[key: string]: JSONObject }[Permalink](#clientcapabilities-extensions) Optional MCP extensions that the client supports. Keys are extension identifiers
(e.g., "io.modelcontextprotocol/oauth-client-credentials"), and values are
per-extension settings objects. An empty object indicates support with no settings. Keys MUST follow the [`_meta` key naming rules](#metaobject), with a
mandatory prefix.   <details> <summary>Example: Extensions — MCP Apps (UI) extension with MIME type support[Permalink](#clientcapabilities-example-extensions-mcp-apps-ui-extension-with-mime-type-support)</summary><pre id="clientcapabilities-example-extensions-mcp-apps-ui-extension-with-mime-type-support">`{  "extensions": {    "io.modelcontextprotocol/ui": {      "mimeTypes": ["text/html;profile=mcp-app"]    }  }} `Copy</pre> </details>

### `Implementation`

interface Implementation \{\
  [icons](#implementation-icons)?: [Icon](#icon)\[];\
  [name](#implementation-name): string;\
  [title](#implementation-title)?: string;\
  [version](#implementation-version): string;\
  [description](#implementation-description)?: string;\
  [websiteUrl](#implementation-websiteurl)?: string;\
}Describes the MCP implementation.  icons?: Icon\[][Permalink](#implementation-icons) Optional set of sized icons that the client can display in a user interface. Clients that support rendering icons MUST support at least the following MIME types: <ul> <li>`image/png` - PNG images (safe, universal compatibility)</li> <li>`image/jpeg` (and `image/jpg`) - JPEG images (safe, universal compatibility)</li> </ul> Clients that support rendering icons SHOULD also support: <ul> <li>`image/svg+xml` - SVG images (scalable but requires security precautions)</li> <li>`image/webp` - WebP images (modern, efficient format)</li> </ul>  Inherited from Icons.icons name: string[Permalink](#implementation-name) Intended for programmatic or logical use, but used as a display name in past specs or fallback (if title isn't present).  Inherited from BaseMetadata.name title?: string[Permalink](#implementation-title) Intended for UI and end-user contexts — optimized to be human-readable and easily understood,
even by those unfamiliar with domain-specific terminology. If not provided, the name should be used for display (except for [Tool](#tool),
where `annotations.title` should be given precedence over using `name`,
if present).  Inherited from BaseMetadata.title version: string[Permalink](#implementation-version) The version of this implementation.  description?: string[Permalink](#implementation-description) An optional human-readable description of what this implementation does. This can be used by clients or servers to provide context about their purpose
and capabilities. For example, a server might describe the types of resources
or tools it provides, while a client might describe its intended use case.  websiteUrl?: string[Permalink](#implementation-websiteurl) An optional URL of the website for this implementation.&#x20;

### `ServerCapabilities`

interface ServerCapabilities \{\
  [experimental](#servercapabilities-experimental)?: \{ \[key: string]: [JSONObject](#jsonobject) };\
  [logging](#servercapabilities-logging)?: [JSONObject](#jsonobject);\
  [completions](#servercapabilities-completions)?: [JSONObject](#jsonobject);\
  [prompts](#servercapabilities-prompts)?: \{ listChanged?: boolean };\
  [resources](#servercapabilities-resources)?: \{ subscribe?: boolean; listChanged?: boolean };\
  [tools](#servercapabilities-tools)?: \{ listChanged?: boolean };\
  [extensions](#servercapabilities-extensions)?: \{ \[key: string]: [JSONObject](#jsonobject) };\
}Capabilities that a server may support. Known capabilities are defined here, in this schema, but this is not a closed set: any server can define its own, additional capabilities.  experimental?: \{ \[key: string]: JSONObject }[Permalink](#servercapabilities-experimental) Experimental, non-standard capabilities that the server supports.  logging?: JSONObject[Permalink](#servercapabilities-logging) Present if the server supports sending log messages to the client.    DeprecatedDeprecated as of protocol version 2026-07-28 (SEP-2577).
Remains in the specification for at least twelve months; see the
deprecated features registry.  <details> <summary>Example: Logging — minimum baseline support[Permalink](#servercapabilities-example-logging-minimum-baseline-support)</summary><pre id="servercapabilities-example-logging-minimum-baseline-support">`{  "logging": {}} `Copy</pre> </details> completions?: JSONObject[Permalink](#servercapabilities-completions) Present if the server supports argument autocompletion suggestions.   <details> <summary>Example: Completions — minimum baseline support[Permalink](#servercapabilities-example-completions-minimum-baseline-support)</summary><pre id="servercapabilities-example-completions-minimum-baseline-support">`{  "completions": {}} `Copy</pre> </details> prompts?: \{ listChanged?: boolean }[Permalink](#servercapabilities-prompts) Present if the server offers any prompt templates.   Type Declaration <ul> <li> `Optional`listChanged?: boolean Whether this server supports notifications for changes to the prompt list. </li></ul>  <details> <summary>Example: Prompts — minimum baseline support[Permalink](#servercapabilities-example-prompts-minimum-baseline-support)</summary><pre id="servercapabilities-example-prompts-minimum-baseline-support">`{  "prompts": {}} `Copy</pre> </details> <details> <summary>Example: Prompts — list changed notifications[Permalink](#servercapabilities-example-prompts-list-changed-notifications)</summary><pre id="servercapabilities-example-prompts-list-changed-notifications">`{  "prompts": {    "listChanged": true  }} `Copy</pre> </details> resources?: \{ subscribe?: boolean; listChanged?: boolean }[Permalink](#servercapabilities-resources) Present if the server offers any resources to read.   Type Declaration <ul> <li> `Optional`subscribe?: boolean Whether this server supports subscribing to resource updates. </li> <li> `Optional`listChanged?: boolean Whether this server supports notifications for changes to the resource list. </li></ul>  <details> <summary>Example: Resources — minimum baseline support[Permalink](#servercapabilities-example-resources-minimum-baseline-support)</summary><pre id="servercapabilities-example-resources-minimum-baseline-support">`{  "resources": {}} `Copy</pre> </details> <details> <summary>Example: Resources — subscription to individual resource updates (only)[Permalink](#servercapabilities-example-resources-subscription-to-individual-resource-updates-only)</summary><pre id="servercapabilities-example-resources-subscription-to-individual-resource-updates-only">`{  "resources": {    "subscribe": true  }} `Copy</pre> </details> <details> <summary>Example: Resources — list changed notifications (only)[Permalink](#servercapabilities-example-resources-list-changed-notifications-only)</summary><pre id="servercapabilities-example-resources-list-changed-notifications-only">`{  "resources": {    "listChanged": true  }} `Copy</pre> </details> <details> <summary>Example: Resources — all notifications[Permalink](#servercapabilities-example-resources-all-notifications)</summary><pre id="servercapabilities-example-resources-all-notifications">`{  "resources": {    "subscribe": true,    "listChanged": true  }} `Copy</pre> </details> tools?: \{ listChanged?: boolean }[Permalink](#servercapabilities-tools) Present if the server offers any tools to call.   Type Declaration <ul> <li> `Optional`listChanged?: boolean Whether this server supports notifications for changes to the tool list. </li></ul>  <details> <summary>Example: Tools — minimum baseline support[Permalink](#servercapabilities-example-tools-minimum-baseline-support)</summary><pre id="servercapabilities-example-tools-minimum-baseline-support">`{  "tools": {}} `Copy</pre> </details> <details> <summary>Example: Tools — list changed notifications[Permalink](#servercapabilities-example-tools-list-changed-notifications)</summary><pre id="servercapabilities-example-tools-list-changed-notifications">`{  "tools": {    "listChanged": true  }} `Copy</pre> </details> extensions?: \{ \[key: string]: JSONObject }[Permalink](#servercapabilities-extensions) Optional MCP extensions that the server supports. Keys are extension identifiers
(e.g., "io.modelcontextprotocol/tasks"), and values are per-extension settings
objects. An empty object indicates support with no settings. Keys MUST follow the [`_meta` key naming rules](#metaobject), with a
mandatory prefix.   <details> <summary>Example: Extensions — Tasks extension support[Permalink](#servercapabilities-example-extensions-tasks-extension-support)</summary><pre id="servercapabilities-example-extensions-tasks-extension-support">`{  "extensions": {    "io.modelcontextprotocol/tasks": {}  }} `Copy</pre> </details>

## `subscriptions/listen`

### `SubscriptionsListenRequest`

interface SubscriptionsListenRequest \{\
  [jsonrpc](#subscriptionslistenrequest-jsonrpc): "2.0";\
  [id](#subscriptionslistenrequest-id): [RequestId](#requestid);\
  [method](#subscriptionslistenrequest-method): "subscriptions/listen";\
  [params](#subscriptionslistenrequest-params): [SubscriptionsListenRequestParams](#subscriptionslistenrequestparams);\
}Sent from the client to open a long-lived channel for receiving notifications
outside the context of a specific request. Replaces the previous HTTP GET
endpoint and ensures consistent behavior between HTTP and STDIO.  <details> <summary>Example: Listen for tools and resource list changes[Permalink](#subscriptionslistenrequest-example-listen-for-tools-and-resource-list-changes)</summary><pre id="subscriptionslistenrequest-example-listen-for-tools-and-resource-list-changes">`{  "jsonrpc": "2.0",  "id": "listen-1",  "method": "subscriptions/listen",  "params": {    "_meta": {      "io.modelcontextprotocol/protocolVersion": "2026-07-28",      "io.modelcontextprotocol/clientInfo": {        "name": "ExampleClient",        "version": "1.0.0"      },      "io.modelcontextprotocol/clientCapabilities": {}    },    "notifications": {      "toolsListChanged": true,      "resourceSubscriptions": ["file:///project/config.json"]    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#subscriptionslistenrequest-jsonrpc)  Inherited from [JSONRPCRequest](#jsonrpcrequest).[jsonrpc](#jsonrpcrequest-jsonrpc) id: RequestId[Permalink](#subscriptionslistenrequest-id)  Inherited from [JSONRPCRequest](#jsonrpcrequest).[id](#jsonrpcrequest-id) method: "subscriptions/listen"[Permalink](#subscriptionslistenrequest-method)  Overrides [JSONRPCRequest](#jsonrpcrequest).[method](#jsonrpcrequest-method) params: SubscriptionsListenRequestParams[Permalink](#subscriptionslistenrequest-params)  Overrides [JSONRPCRequest](#jsonrpcrequest).[params](#jsonrpcrequest-params)

### `SubscriptionsListenRequestParams`

interface SubscriptionsListenRequestParams \{\
  [\_meta](#subscriptionslistenrequestparams-_meta): [RequestMetaObject](#requestmetaobject);\
  [notifications](#subscriptionslistenrequestparams-notifications): [SubscriptionFilter](#subscriptionfilter);\
}Parameters for a [subscriptions/listen](#subscriptionslistenrequest) request.  \_meta: RequestMetaObject[Permalink](#subscriptionslistenrequestparams-_meta)  Inherited from [RequestParams](#requestparams).[\_meta](#requestparams-_meta) notifications: SubscriptionFilter[Permalink](#subscriptionslistenrequestparams-notifications) The notifications the client opts in to on this stream. The server **MUST NOT** send notification types the client has not explicitly
requested.&#x20;

### `SubscriptionsListenResultResponse`

interface SubscriptionsListenResultResponse \{\
  [jsonrpc](#subscriptionslistenresultresponse-jsonrpc): "2.0";\
  [id](#subscriptionslistenresultresponse-id): [RequestId](#requestid);\
  [result](#subscriptionslistenresultresponse-result): [SubscriptionsListenResult](#subscriptionslistenresult);\
}A successful response from the server for a [subscriptions/listen](#subscriptionslistenrequest)
request, sent when the server tears the subscription down gracefully.  <details> <summary>Example: Subscription closed gracefully response[Permalink](#subscriptionslistenresultresponse-example-subscription-closed-gracefully-response)</summary><pre id="subscriptionslistenresultresponse-example-subscription-closed-gracefully-response">`{  "jsonrpc": "2.0",  "id": "listen-1",  "result": {    "resultType": "complete",    "_meta": {      "io.modelcontextprotocol/subscriptionId": "listen-1"    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#subscriptionslistenresultresponse-jsonrpc)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[jsonrpc](#jsonrpcresultresponse-jsonrpc) id: RequestId[Permalink](#subscriptionslistenresultresponse-id)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[id](#jsonrpcresultresponse-id) result: SubscriptionsListenResult[Permalink](#subscriptionslistenresultresponse-result)  Overrides [JSONRPCResultResponse](#jsonrpcresultresponse).[result](#jsonrpcresultresponse-result)

### `SubscriptionsListenResult`

interface SubscriptionsListenResult \{\
  [resultType](#subscriptionslistenresult-resulttype): string;\
  [\_meta](#subscriptionslistenresult-_meta): [SubscriptionsListenResultMetaObject](#subscriptionslistenresultmetaobject);\
  \[key: string]: unknown;\
}The response to a [subscriptions/listen](#subscriptionslistenrequest)
request, signalling that the subscription has ended gracefully (for example,
during server shutdown). Because the listen stream is long-lived, this result
is sent only when the server tears the subscription down; an abrupt transport
close carries no response. The result body is otherwise empty.  <details> <summary>Example: Subscription closed gracefully[Permalink](#subscriptionslistenresult-example-subscription-closed-gracefully)</summary><pre id="subscriptionslistenresult-example-subscription-closed-gracefully">`{  "resultType": "complete",  "_meta": {    "io.modelcontextprotocol/subscriptionId": "listen-1"  }} `Copy</pre> </details> resultType: string[Permalink](#subscriptionslistenresult-resulttype) Indicates the type of the result, which allows the client to determine
how to parse the result object. Servers implementing this protocol version MUST include this field.
For backward compatibility, when a client receives a result from a
server implementing an earlier protocol version (which does not include `resultType`), the client MUST treat the absent field as `"complete"`.  Inherited from [Result](#result).[resultType](#result-resulttype) \_meta: SubscriptionsListenResultMetaObject[Permalink](#subscriptionslistenresult-_meta)  Overrides [Result](#result).[\_meta](#result-_meta)

### `SubscriptionFilter`

interface SubscriptionFilter \{\
  [toolsListChanged](#subscriptionfilter-toolslistchanged)?: boolean;\
  [promptsListChanged](#subscriptionfilter-promptslistchanged)?: boolean;\
  [resourcesListChanged](#subscriptionfilter-resourceslistchanged)?: boolean;\
  [resourceSubscriptions](#subscriptionfilter-resourcesubscriptions)?: string\[];\
}The set of notification types a client may opt in to on a [subscriptions/listen](#subscriptionslistenrequest) request. Each notification type is **opt-in**; the server **MUST NOT** send
notification types the client has not explicitly requested here.  toolsListChanged?: boolean[Permalink](#subscriptionfilter-toolslistchanged) If true, receive [notifications/tools/list\_changed](#toollistchangednotification).  promptsListChanged?: boolean[Permalink](#subscriptionfilter-promptslistchanged) If true, receive [notifications/prompts/list\_changed](#promptlistchangednotification).  resourcesListChanged?: boolean[Permalink](#subscriptionfilter-resourceslistchanged) If true, receive [notifications/resources/list\_changed](#resourcelistchangednotification).  resourceSubscriptions?: string\[][Permalink](#subscriptionfilter-resourcesubscriptions) Subscribe to [notifications/resources/updated](#resourceupdatednotification) for these resource URIs.
Replaces the former `resources/subscribe` RPC.&#x20;

### `SubscriptionsListenResultMetaObject`

interface SubscriptionsListenResultMetaObject \{\
  ["io.modelcontextprotocol/serverInfo"](#subscriptionslistenresultmetaobject-iomodelcontextprotocolserverinfo)?: [Implementation](#implementation);\
  ["io.modelcontextprotocol/subscriptionId"](#subscriptionslistenresultmetaobject-iomodelcontextprotocolsubscriptionid): [RequestId](#requestid);\
  \[key: string]: unknown;\
}Extends [ResultMetaObject](#resultmetaobject) with the subscription-stream identifier carried by a [SubscriptionsListenResult](#subscriptionslistenresult). All key naming rules from `MetaObject` apply.   See[MetaObject](#metaobject) for key naming rules and reserved prefixes.  "io.modelcontextprotocol/serverInfo"?: Implementation[Permalink](#subscriptionslistenresultmetaobject-iomodelcontextprotocolserverinfo) Identifies the server software producing the response. Servers SHOULD
include this field on every response unless specifically configured not
to do so. The [Implementation](#implementation) schema requires `name` and `version`; other
fields are optional. The value is self-reported by the server and is not verified by the
protocol. It is intended for display, logging, and debugging. Clients
SHOULD NOT use it to change their behavior, and SHOULD NOT rely on it for
security decisions.  Inherited from [ResultMetaObject](#resultmetaobject).[io.modelcontextprotocol/serverInfo](#resultmetaobject-iomodelcontextprotocolserverinfo) "io.modelcontextprotocol/subscriptionId": RequestId[Permalink](#subscriptionslistenresultmetaobject-iomodelcontextprotocolsubscriptionid) Identifies the subscription stream this response closes, so the client can
correlate it with the originating subscription — mirroring the same key on
the stream's notifications. The value is the JSON-RPC ID of the `subscriptions/listen` request that opened the stream (and equals this
response's `id`).&#x20;

## `tools/call`

### `CallToolRequest`

interface CallToolRequest \{\
  [jsonrpc](#calltoolrequest-jsonrpc): "2.0";\
  [id](#calltoolrequest-id): [RequestId](#requestid);\
  [method](#calltoolrequest-method): "tools/call";\
  [params](#calltoolrequest-params): [CallToolRequestParams](#calltoolrequestparams);\
}Used by the client to invoke a tool provided by the server.  <details> <summary>Example: Call tool request[Permalink](#calltoolrequest-example-call-tool-request)</summary><pre id="calltoolrequest-example-call-tool-request">`{  "jsonrpc": "2.0",  "id": "call-tool-example",  "method": "tools/call",  "params": {    "_meta": {      "io.modelcontextprotocol/protocolVersion": "2026-07-28",      "io.modelcontextprotocol/clientInfo": {        "name": "ExampleClient",        "version": "1.0.0"      },      "io.modelcontextprotocol/clientCapabilities": {}    },    "name": "get_weather",    "arguments": {      "location": "New York"    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#calltoolrequest-jsonrpc)  Inherited from [JSONRPCRequest](#jsonrpcrequest).[jsonrpc](#jsonrpcrequest-jsonrpc) id: RequestId[Permalink](#calltoolrequest-id)  Inherited from [JSONRPCRequest](#jsonrpcrequest).[id](#jsonrpcrequest-id) method: "tools/call"[Permalink](#calltoolrequest-method)  Overrides [JSONRPCRequest](#jsonrpcrequest).[method](#jsonrpcrequest-method) params: CallToolRequestParams[Permalink](#calltoolrequest-params)  Overrides [JSONRPCRequest](#jsonrpcrequest).[params](#jsonrpcrequest-params)

### `CallToolRequestParams`

interface CallToolRequestParams \{\
  [\_meta](#calltoolrequestparams-_meta): [RequestMetaObject](#requestmetaobject);\
  [inputResponses](#calltoolrequestparams-inputresponses)?: [InputResponses](#inputresponses);\
  [requestState](#calltoolrequestparams-requeststate)?: string;\
  [name](#calltoolrequestparams-name): string;\
  [arguments](#calltoolrequestparams-arguments)?: \{ \[key: string]: unknown };\
}Parameters for a `tools/call` request.  <details> <summary>Example: \`get\_weather\` tool call params[Permalink](#calltoolrequestparams-example-get_weather-tool-call-params)</summary><pre id="calltoolrequestparams-example-get_weather-tool-call-params">`{  "_meta": {    "io.modelcontextprotocol/protocolVersion": "2026-07-28",    "io.modelcontextprotocol/clientInfo": {      "name": "ExampleClient",      "version": "1.0.0"    },    "io.modelcontextprotocol/clientCapabilities": {}  },  "name": "get_weather",  "arguments": {    "location": "New York"  }} `Copy</pre> </details> <details> <summary>Example: Tool call params with progress token[Permalink](#calltoolrequestparams-example-tool-call-params-with-progress-token)</summary><pre id="calltoolrequestparams-example-tool-call-params-with-progress-token">`{  "_meta": {    "io.modelcontextprotocol/protocolVersion": "2026-07-28",    "io.modelcontextprotocol/clientInfo": {      "name": "ExampleClient",      "version": "1.0.0"    },    "io.modelcontextprotocol/clientCapabilities": {},    "progressToken": "oivaizmir"  },  "name": "build_simulation",  "arguments": {    "city": "Micropolis"  }} `Copy</pre> </details> \_meta: RequestMetaObject[Permalink](#calltoolrequestparams-_meta)  Inherited from [InputResponseRequestParams](#inputresponserequestparams).[\_meta](#inputresponserequestparams-_meta) inputResponses?: InputResponses[Permalink](#calltoolrequestparams-inputresponses)  Inherited from [InputResponseRequestParams](#inputresponserequestparams).[inputResponses](#inputresponserequestparams-inputresponses) requestState?: string[Permalink](#calltoolrequestparams-requeststate)  Inherited from [InputResponseRequestParams](#inputresponserequestparams).[requestState](#inputresponserequestparams-requeststate) name: string[Permalink](#calltoolrequestparams-name) The name of the tool.  arguments?: \{ \[key: string]: unknown }[Permalink](#calltoolrequestparams-arguments) Arguments to use for the tool call.&#x20;

### `CallToolResultResponse`

interface CallToolResultResponse \{\
  [jsonrpc](#calltoolresultresponse-jsonrpc): "2.0";\
  [id](#calltoolresultresponse-id): [RequestId](#requestid);\
  [result](#calltoolresultresponse-result): [InputRequiredResult](#inputrequiredresult) | [CallToolResult](#calltoolresult);\
}A successful response from the server for a [tools/call](#calltoolrequest) request.  <details> <summary>Example: Call tool result response[Permalink](#calltoolresultresponse-example-call-tool-result-response)</summary><pre id="calltoolresultresponse-example-call-tool-result-response">`{  "jsonrpc": "2.0",  "id": "call-tool-example",  "result": {    "resultType": "complete",    "content": [      {        "type": "text",        "text": "Current weather in New York:\nTemperature: 72°F\nConditions: Partly cloudy"      }    ],    "isError": false  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#calltoolresultresponse-jsonrpc)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[jsonrpc](#jsonrpcresultresponse-jsonrpc) id: RequestId[Permalink](#calltoolresultresponse-id)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[id](#jsonrpcresultresponse-id) result: InputRequiredResult | CallToolResult[Permalink](#calltoolresultresponse-result)  Overrides [JSONRPCResultResponse](#jsonrpcresultresponse).[result](#jsonrpcresultresponse-result)

### `CallToolResult`

interface CallToolResult \{\
  [\_meta](#calltoolresult-_meta)?: [ResultMetaObject](#resultmetaobject);\
  [resultType](#calltoolresult-resulttype): string;\
  [content](#calltoolresult-content): [ContentBlock](#contentblock)\[];\
  [structuredContent](#calltoolresult-structuredcontent)?: unknown;\
  [isError](#calltoolresult-iserror)?: boolean;\
  \[key: string]: unknown;\
}The result returned by the server for a [tools/call](#calltoolrequest) request.  <details> <summary>Example: Result with unstructured text[Permalink](#calltoolresult-example-result-with-unstructured-text)</summary><pre id="calltoolresult-example-result-with-unstructured-text">`{  "resultType": "complete",  "content": [    {      "type": "text",      "text": "Current weather in New York:\nTemperature: 72°F\nConditions: Partly cloudy"    }  ],  "isError": false} `Copy</pre> </details> <details> <summary>Example: Result with structured content[Permalink](#calltoolresult-example-result-with-structured-content)</summary><pre id="calltoolresult-example-result-with-structured-content">`{  "resultType": "complete",  "content": [    {      "type": "text",      "text": "{\"temperature\": 22.5, \"conditions\": \"Partly cloudy\", \"humidity\": 65}"    }  ],  "structuredContent": {    "temperature": 22.5,    "conditions": "Partly cloudy",    "humidity": 65  }} `Copy</pre> </details> <details> <summary>Example: Invalid tool input error[Permalink](#calltoolresult-example-invalid-tool-input-error)</summary><pre id="calltoolresult-example-invalid-tool-input-error">`{  "resultType": "complete",  "content": [    {      "type": "text",      "text": "Invalid departure date: must be in the future. Current date is 08/08/2025."    }  ],  "isError": true} `Copy</pre> </details> \_meta?: ResultMetaObject[Permalink](#calltoolresult-_meta)  Inherited from [Result](#result).[\_meta](#result-_meta) resultType: string[Permalink](#calltoolresult-resulttype) Indicates the type of the result, which allows the client to determine
how to parse the result object. Servers implementing this protocol version MUST include this field.
For backward compatibility, when a client receives a result from a
server implementing an earlier protocol version (which does not include `resultType`), the client MUST treat the absent field as `"complete"`.  Inherited from [Result](#result).[resultType](#result-resulttype) content: ContentBlock\[][Permalink](#calltoolresult-content) A list of content objects that represent the unstructured result of the tool call.  structuredContent?: unknown[Permalink](#calltoolresult-structuredcontent) An optional JSON value that represents the structured result of the tool call. This can be any JSON value (object, array, string, number, boolean, or null)
that conforms to the tool's outputSchema if one is defined.  isError?: boolean[Permalink](#calltoolresult-iserror) Whether the tool call ended in an error. If not set, this is assumed to be false (the call was successful). Any errors that originate from the tool SHOULD be reported inside the result
object, with `isError` set to true, *not* as an MCP protocol-level error
response. Otherwise, the LLM would not be able to see that an error occurred
and self-correct. However, any errors in *finding* the tool, an error indicating that the
server does not support tool calls, or any other exceptional conditions,
should be reported as an MCP error response.&#x20;

## `tools/list`

### `ListToolsRequest`

interface ListToolsRequest \{\
  [jsonrpc](#listtoolsrequest-jsonrpc): "2.0";\
  [id](#listtoolsrequest-id): [RequestId](#requestid);\
  [params](#listtoolsrequest-params): [PaginatedRequestParams](#paginatedrequestparams);\
  [method](#listtoolsrequest-method): "tools/list";\
}Sent from the client to request a list of tools the server has.  <details> <summary>Example: List tools request[Permalink](#listtoolsrequest-example-list-tools-request)</summary><pre id="listtoolsrequest-example-list-tools-request">`{  "jsonrpc": "2.0",  "id": "list-tools-example",  "method": "tools/list",  "params": {    "_meta": {      "io.modelcontextprotocol/protocolVersion": "2026-07-28",      "io.modelcontextprotocol/clientInfo": {        "name": "ExampleClient",        "version": "1.0.0"      },      "io.modelcontextprotocol/clientCapabilities": {}    }  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#listtoolsrequest-jsonrpc)  Inherited from PaginatedRequest.jsonrpc id: RequestId[Permalink](#listtoolsrequest-id)  Inherited from PaginatedRequest.id params: PaginatedRequestParams[Permalink](#listtoolsrequest-params)  Inherited from PaginatedRequest.params method: "tools/list"[Permalink](#listtoolsrequest-method)  Overrides PaginatedRequest.method

### `ListToolsResultResponse`

interface ListToolsResultResponse \{\
  [jsonrpc](#listtoolsresultresponse-jsonrpc): "2.0";\
  [id](#listtoolsresultresponse-id): [RequestId](#requestid);\
  [result](#listtoolsresultresponse-result): [ListToolsResult](#listtoolsresult);\
}A successful response from the server for a [tools/list](#listtoolsrequest) request.  <details> <summary>Example: List tools result response[Permalink](#listtoolsresultresponse-example-list-tools-result-response)</summary><pre id="listtoolsresultresponse-example-list-tools-result-response">`{  "jsonrpc": "2.0",  "id": "list-tools-example",  "result": {    "resultType": "complete",    "tools": [      {        "name": "get_weather",        "title": "Weather Information Provider",        "description": "Get current weather information for a location",        "inputSchema": {          "type": "object",          "properties": {            "location": {              "type": "string",              "description": "City name or zip code"            }          },          "required": ["location"]        },        "icons": [          {            "src": "https://example.com/weather-icon.png",            "mimeType": "image/png",            "sizes": ["48x48"]          }        ]      }    ],    "nextCursor": "next-page-cursor",    "ttlMs": 3600000,    "cacheScope": "public"  }} `Copy</pre> </details> jsonrpc: "2.0"[Permalink](#listtoolsresultresponse-jsonrpc)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[jsonrpc](#jsonrpcresultresponse-jsonrpc) id: RequestId[Permalink](#listtoolsresultresponse-id)  Inherited from [JSONRPCResultResponse](#jsonrpcresultresponse).[id](#jsonrpcresultresponse-id) result: ListToolsResult[Permalink](#listtoolsresultresponse-result)  Overrides [JSONRPCResultResponse](#jsonrpcresultresponse).[result](#jsonrpcresultresponse-result)

### `ListToolsResult`

interface ListToolsResult \{\
  [\_meta](#listtoolsresult-_meta)?: [ResultMetaObject](#resultmetaobject);\
  [resultType](#listtoolsresult-resulttype): string;\
  [nextCursor](#listtoolsresult-nextcursor)?: string;\
  [ttlMs](#listtoolsresult-ttlms): number;\
  [cacheScope](#listtoolsresult-cachescope): "public" | "private";\
  [tools](#listtoolsresult-tools): [Tool](#tool)\[];\
  \[key: string]: unknown;\
}The result returned by the server for a [tools/list](#listtoolsrequest) request.  <details> <summary>Example: Tools list with cursor and TTL[Permalink](#listtoolsresult-example-tools-list-with-cursor-and-ttl)</summary><pre id="listtoolsresult-example-tools-list-with-cursor-and-ttl">`{  "resultType": "complete",  "tools": [    {      "name": "get_weather",      "title": "Weather Information Provider",      "description": "Get current weather information for a location",      "inputSchema": {        "type": "object",        "properties": {          "location": {            "type": "string",            "description": "City name or zip code"          }        },        "required": ["location"]      },      "icons": [        {          "src": "https://example.com/weather-icon.png",          "mimeType": "image/png",          "sizes": ["48x48"]        }      ]    }  ],  "nextCursor": "next-page-cursor",  "ttlMs": 300000,  "cacheScope": "public"} `Copy</pre> </details> \_meta?: ResultMetaObject[Permalink](#listtoolsresult-_meta)  Inherited from PaginatedResult.\_meta resultType: string[Permalink](#listtoolsresult-resulttype) Indicates the type of the result, which allows the client to determine
how to parse the result object. Servers implementing this protocol version MUST include this field.
For backward compatibility, when a client receives a result from a
server implementing an earlier protocol version (which does not include `resultType`), the client MUST treat the absent field as `"complete"`.  Inherited from PaginatedResult.resultType nextCursor?: string[Permalink](#listtoolsresult-nextcursor) An opaque token representing the pagination position after the last returned result.
If present, there may be more results available.  Inherited from PaginatedResult.nextCursor ttlMs: number[Permalink](#listtoolsresult-ttlms) A hint from the server indicating how long (in milliseconds) the
client MAY cache this response before re-fetching. Semantics are
analogous to HTTP Cache-Control max-age. <ul> <li>If 0, The response SHOULD be considered immediately stale,
The client MAY re-fetch every time the result is needed.</li> <li>If positive, the client SHOULD consider the result fresh for this many
milliseconds after receiving the response.</li> </ul>  Inherited from CacheableResult.ttlMs cacheScope: "public" | "private"[Permalink](#listtoolsresult-cachescope) Indicates the intended scope of the cached response, analogous to HTTP `Cache-Control: public` vs `Cache-Control: private`. <ul> <li>`"public"`: The response does not contain user-specific data. Any
client or intermediary (e.g., shared gateway, caching proxy) MAY cache
the response and serve it across authorization contexts.</li> <li>`"private"`: The response MAY be cached and reused only within the
same authorization context. Caches MUST NOT be shared across
authorization contexts (e.g., a different access token requires a
different cache).</li> </ul>  Inherited from CacheableResult.cacheScope tools: Tool\[][Permalink](#listtoolsresult-tools)&#x20;

### `Tool`

interface Tool \{\
  [icons](#tool-icons)?: [Icon](#icon)\[];\
  [name](#tool-name): string;\
  [title](#tool-title)?: string;\
  [description](#tool-description)?: string;\
  [inputSchema](#tool-inputschema): \{ \$schema?: string; type: "object"; \[key: string]: unknown };\
  [outputSchema](#tool-outputschema)?: \{ \$schema?: string; \[key: string]: unknown };\
  [annotations](#tool-annotations)?: [ToolAnnotations](#toolannotations);\
  [\_meta](#tool-_meta)?: [MetaObject](#metaobject);\
}Definition for a tool the client can call.  <details> <summary>Example: With default 2020-12 input schema[Permalink](#tool-example-with-default-2020-12-input-schema)</summary><pre id="tool-example-with-default-2020-12-input-schema">`{  "name": "calculate_sum",  "description": "Add two numbers",  "inputSchema": {    "type": "object",    "properties": {      "a": { "type": "number" },      "b": { "type": "number" }    },    "required": ["a", "b"]  }} `Copy</pre> </details> <details> <summary>Example: With explicit draft-07 input schema[Permalink](#tool-example-with-explicit-draft-07-input-schema)</summary><pre id="tool-example-with-explicit-draft-07-input-schema">`{  "name": "calculate_sum",  "description": "Add two numbers",  "inputSchema": {    "$schema": "http://json-schema.org/draft-07/schema#",    "type": "object",    "properties": {      "a": { "type": "number" },      "b": { "type": "number" }    },    "required": ["a", "b"]  }} `Copy</pre> </details> <details> <summary>Example: With no parameters[Permalink](#tool-example-with-no-parameters)</summary><pre id="tool-example-with-no-parameters">`{  "name": "get_current_time",  "description": "Returns the current server time",  "inputSchema": {    "type": "object",    "additionalProperties": false  }} `Copy</pre> </details> <details> <summary>Example: With output schema for structured content[Permalink](#tool-example-with-output-schema-for-structured-content)</summary><pre id="tool-example-with-output-schema-for-structured-content">`{  "name": "get_weather_data",  "title": "Weather Data Retriever",  "description": "Get current weather data for a location",  "inputSchema": {    "type": "object",    "properties": {      "location": {        "type": "string",        "description": "City name or zip code"      }    },    "required": ["location"]  },  "outputSchema": {    "type": "object",    "properties": {      "temperature": {        "type": "number",        "description": "Temperature in celsius"      },      "conditions": {        "type": "string",        "description": "Weather conditions description"      },      "humidity": {        "type": "number",        "description": "Humidity percentage"      }    },    "required": ["temperature", "conditions", "humidity"]  }} `Copy</pre> </details> icons?: Icon\[][Permalink](#tool-icons) Optional set of sized icons that the client can display in a user interface. Clients that support rendering icons MUST support at least the following MIME types: <ul> <li>`image/png` - PNG images (safe, universal compatibility)</li> <li>`image/jpeg` (and `image/jpg`) - JPEG images (safe, universal compatibility)</li> </ul> Clients that support rendering icons SHOULD also support: <ul> <li>`image/svg+xml` - SVG images (scalable but requires security precautions)</li> <li>`image/webp` - WebP images (modern, efficient format)</li> </ul>  Inherited from Icons.icons name: string[Permalink](#tool-name) Intended for programmatic or logical use, but used as a display name in past specs or fallback (if title isn't present).  Inherited from BaseMetadata.name title?: string[Permalink](#tool-title) Intended for UI and end-user contexts — optimized to be human-readable and easily understood,
even by those unfamiliar with domain-specific terminology. If not provided, the name should be used for display (except for [Tool](#),
where `annotations.title` should be given precedence over using `name`,
if present).  Inherited from BaseMetadata.title description?: string[Permalink](#tool-description) A human-readable description of the tool. This can be used by clients to improve the LLM's understanding of available tools. It can be thought of like a "hint" to the model.  inputSchema: \{ \$schema?: string; type: "object"; \[key: string]: unknown }[Permalink](#tool-inputschema) A JSON Schema object defining the expected parameters for the tool. Tool arguments are always JSON objects, so `type: "object"` is required at the root.
Beyond that, any JSON Schema 2020-12 keyword may appear alongside `type` — including
composition keywords (`oneOf`, `anyOf`, `allOf`, `not`), conditional keywords
(`if`/`then`/`else`), reference keywords (`$ref`, `$defs`, `$anchor`), and any other
standard validation or annotation keywords. Property schemas may carry an `x-mcp-header` annotation to mirror the
argument value into an HTTP header on the Streamable HTTP transport. See
the Streamable HTTP transport specification for the validity and
extraction rules. Defaults to JSON Schema 2020-12 when no explicit `$schema` is provided.  outputSchema?: \{ \$schema?: string; \[key: string]: unknown }[Permalink](#tool-outputschema) An optional JSON Schema object defining the structure of the tool's output returned in
the structuredContent field of a [CallToolResult](#calltoolresult). This can be any valid JSON Schema 2020-12. Defaults to JSON Schema 2020-12 when no explicit `$schema` is provided.  annotations?: ToolAnnotations[Permalink](#tool-annotations) Optional additional tool information. Display name precedence order is: `title`, `annotations.title`, then `name`.  \_meta?: MetaObject[Permalink](#tool-_meta)&#x20;

### `ToolAnnotations`

interface ToolAnnotations \{\
  [title](#toolannotations-title)?: string;\
  [readOnlyHint](#toolannotations-readonlyhint)?: boolean;\
  [destructiveHint](#toolannotations-destructivehint)?: boolean;\
  [idempotentHint](#toolannotations-idempotenthint)?: boolean;\
  [openWorldHint](#toolannotations-openworldhint)?: boolean;\
}Additional properties describing a [Tool](#tool) to clients. NOTE: all properties in `ToolAnnotations` are **hints**.
They are not guaranteed to provide a faithful description of
tool behavior (including descriptive properties like `title`). Clients should never make tool use decisions based on `ToolAnnotations`
received from untrusted servers.  title?: string[Permalink](#toolannotations-title) A human-readable title for the tool.  readOnlyHint?: boolean[Permalink](#toolannotations-readonlyhint) If true, the tool does not modify its environment. Default: false  destructiveHint?: boolean[Permalink](#toolannotations-destructivehint) If true, the tool may perform destructive updates to its environment.
If false, the tool performs only additive updates. (This property is meaningful only when `readOnlyHint == false`) Default: true  idempotentHint?: boolean[Permalink](#toolannotations-idempotenthint) If true, calling the tool repeatedly with the same arguments
will have no additional effect on its environment. (This property is meaningful only when `readOnlyHint == false`) Default: false  openWorldHint?: boolean[Permalink](#toolannotations-openworldhint) If true, this tool may interact with an "open world" of external
entities. If false, the tool's domain of interaction is closed.
For example, the world of a web search tool is open, whereas that
of a memory tool is not. Default: true&#x20;
