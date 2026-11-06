# TinyMCP 🧩

## 📖 What is TinyMCP?
TinyMCP is a lightweight C++ SDK for implementing the MCP Server.

## 🤔 What is MCP?
MCP is an open protocol that standardizes how applications provide context to LLMs. Think of MCP like a USB-C port for AI applications. Just as USB-C provides a standardized way to connect your devices to various peripherals and accessories, MCP provides a standardized way to connect AI models to different data sources and tools.
> 🔗 [Reference for details](https://modelcontextprotocol.io/introduction)

## 💻 TinyMCP supported system platforms
* Windows
* Linux
* MacOS
* Android
* Other platforms that support standart c++...

## 🚀 Usage Guide
Please check the [wiki](https://github.com/Qihoo360/TinyMCP/wiki) for more information.

## 📦 Dependencies
* [jsoncpp 1.9.5](https://github.com/open-source-parsers/jsoncpp/tree/1.9.5)
* [cpp-httplib v0.44.0](https://github.com/yhirose/cpp-httplib/tree/v0.44.0)

## 📌 MCP protocol version support

| Version | Transports | Session model |
| --- | --- | --- |
| [2024-11-05](https://modelcontextprotocol.io/specification/2024-11-05/index) | stdio, HTTP+SSE | Handshake with a session state machine |
| [2026-07-28](https://modelcontextprotocol.io/specification/2026-07-28/index) | stdio only | Stateless, per-request `_meta` |

`2026-07-28` is served over **stdio only**; this release does not implement the streamable HTTP transport. The legacy HTTP+SSE transport keeps working unchanged for 2024-11-05 clients. Existing 2024-11-05 clients see no behavioral change; a server that used to disable the stateless era now pins `GetHandshakeOnlyRegistry()` instead (see below).

### Selecting protocol versions
The active `CProtocolRegistry` is the single source of truth: it decides which versions are discovered, negotiated and served. The default global registry serves every version in the table above, so no extra call is needed.

```cpp
// Serve both shipped versions (default).
MCP::CMCPSession::GetInstance().SetProtocolRegistry(nullptr);

// Serve only the handshake era (used by the HTTP+SSE entry point).
MCP::CMCPSession::GetInstance().SetProtocolRegistry(
    &MCP::CProtocolRegistry::GetHandshakeOnlyRegistry());

// Serve a custom set. Copy profiles from the global registry and add or drop
// entries; the list order picks the default handshake / latest stateless profile.
static MCP::CProtocolRegistry custom(std::vector<MCP::ProtocolProfile>{
    *MCP::CProtocolRegistry::GetGlobalRegistry().Find(MCP::PROTOCOL_VER_2026_07_28) });
MCP::CMCPSession::GetInstance().SetProtocolRegistry(&custom);
```

The registry must outlive the session (static, global, or a long-lived member). A request whose `_meta` names a version the active registry does not serve is rejected with `-32022`, and the error data lists the supported versions. The example pins the `sse` branch to `GetHandshakeOnlyRegistry()` and leaves the `stdio` branch on the global registry.

### Per-request `_meta`
Stateless requests carry their version and client information in `params._meta`. `protocolVersion` and `clientCapabilities` are required; if any `io.modelcontextprotocol/*` key is present but a required one is missing, the server returns `-32602` and does **not** fill in defaults.

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "tools/list",
  "params": {
    "_meta": {
      "io.modelcontextprotocol/protocolVersion": "2026-07-28",
      "io.modelcontextprotocol/clientCapabilities": {},
      "io.modelcontextprotocol/clientInfo": { "name": "demo-client", "version": "1.0.0" },
      "io.modelcontextprotocol/logLevel": "info"
    }
  }
}
```

Requests without any modern `_meta` key keep the legacy 2024-11-05 semantics. `initialize` is always handled by the handshake path, even when it carries modern metadata.

### Discovery
`server/discover` reports the registered versions and capabilities. `serverInfo` lives in the result `_meta`, not at the result top level.

```json
{
  "jsonrpc": "2.0",
  "id": 2,
  "method": "server/discover",
  "params": { "_meta": { "io.modelcontextprotocol/protocolVersion": "2026-07-28", "io.modelcontextprotocol/clientCapabilities": {} } }
}
```

```json
{
  "jsonrpc": "2.0",
  "id": 2,
  "result": {
    "resultType": "complete",
    "supportedVersions": ["2026-07-28", "2024-11-05"],
    "capabilities": { "tools": { "listChanged": false }, "resources": { "listChanged": false } },
    "instructions": "Echo server",
    "ttlMs": 0,
    "cacheScope": "private",
    "_meta": { "io.modelcontextprotocol/serverInfo": { "name": "TinyMCP", "version": "1.0.0" } }
  }
}
```

### Multi-round tool requests (MRTR)
A tool can answer with `resultType: "input_required"` and ask the client for elicitation, sampling, or roots. The client retries the same request with `inputResponses` and the opaque `requestState`.

```json
{
  "jsonrpc": "2.0",
  "id": 7,
  "result": {
    "resultType": "input_required",
    "inputRequests": {
      "confirm": {
        "method": "elicitation/create",
        "params": { "mode": "form", "message": "Confirm the echo request.", "requestedSchema": { "type": "object", "properties": { "confirm": { "type": "boolean" } } } }
      }
    },
    "requestState": "v1:hello:0a1b2c3d4e5f6071"
  }
}
```

Applications call `NotifyInputRequired(inputRequests, requestState)`. If the client did not declare a capability the request cannot live without, call `NotifyMissingClientCapability(capabilities)`; the SDK answers `-32021` with `error.data.requiredCapabilities`. `requestState` is opaque to the SDK: applications that use it for authorization must protect it themselves (HMAC/AEAD). See `CEchoConfirmTask` in the example. `input_required` results and MRTR retries never carry cache hints.

### Subscriptions
`subscriptions/listen` replaces `resources/subscribe`, `resources/unsubscribe`, and HTTP GET. Filters live in `params.notifications`; the server first acknowledges with a numeric or string `subscriptionId` that keeps the request id type, then pushes matching events with `_meta["io.modelcontextprotocol/subscriptionId"]`.

```json
{ "jsonrpc": "2.0", "id": 9, "method": "subscriptions/listen", "params": { "notifications": { "toolsListChanged": true, "resourceSubscriptions": ["ui://echo/app.html"] } } }
```

The server only grants filters it can actually publish, keeps matching resource URIs exact, and stops sending after `notifications/cancelled` or connection close. The example pushes a `notifications/resources/updated` event every five seconds through `SendSubscriptionNotification()`; a server without listeners sends nothing.

### Caching
Only these six `resultType: "complete"` results must carry `ttlMs` (>= 0) and `cacheScope` (`"public"` or `"private"`):

`server/discover`, `tools/list`, `prompts/list`, `resources/list`, `resources/templates/list`, `resources/read`.

Defaults are `ttlMs = 0` and `cacheScope = "private"`; override them with `SetServerCacheOptions(ttlMs, cacheScope)`. `tools/call`, `prompts/get`, `completion/complete`, `subscriptions/listen`, `input_required` results, and MRTR retries never carry cache fields.

### Errors
* `-32602`: malformed parameters, including a missing required `_meta` field, an unknown tool/resource, or an invalid cursor on modern requests. No defaults are filled in.
* `-32022`: unsupported protocol version; `error.data.supported` lists `CProtocolRegistry::GetSupportedVersions()`.
* `-32601`: unknown method, including modern-only methods on a profile that does not support them.
* `-32021`: missing required client capability; `error.data.requiredCapabilities` is present.
* `-32001`: legacy header mismatch constant, kept unchanged for compatibility; stdio never emits it.
* `-32020`: modern header mismatch constant, reserved for header-carrying transports; it is not emitted over stdio in this release.

Modern `resources/read` reports a missing resource as `-32602`, while legacy keeps `-32002`.

### Extensions and MCP Apps
The only extension implemented today is MCP Apps (`io.modelcontextprotocol/ui`). Extensions are off by default: call `RegisterServerAppsCapabilities()` (or `RegisterServerExtensionsCapabilities()`) to advertise them. Without that call neither `server/discover` nor legacy `initialize` mentions the extension.

MCP Apps covers the server side only: `Tool._meta.ui.resourceUri` links a tool to a `ui://` resource, and `Resource._meta.ui` carries `csp` and `permissions` verbatim. Tools must still return meaningful text content so that hosts without Apps support degrade gracefully. The View↔Host `postMessage` protocol (`ui/initialize`, `ui/message`, `ui/open-link`, `ui/update-model-context`) is **not** part of this SDK.

### Deprecated features
`logging`, `roots`, and `sampling` are deprecated in 2026-07-28. This release still implements them as specified: request-level `_meta.logLevel` for logging (requests without it receive no `notifications/message`), and MRTR for roots/sampling. New implementations should prefer stderr or OpenTelemetry, tool parameters or resource URIs, and direct LLM API calls. Legacy `logging/setLevel` remains available only on 2024-11-05 sessions. SSE connections do not accept modern requests.

## 🗂️ Code structure brief
| directory | brief |
| --- | --- |
| Public | SDK public directory, used to store global type definitions, global constant resources, helper classes, etc. |
| Message | Definition and implementation of the basic data structures of the MCP protocol, mainly including the implementation of protocol data such as standard requests, responses, and notifications that follow the JSON-RPC 2.0 protocol. |
| Session | Definition and implementation of MCP protocol interaction sessions, mainly including the implementation of stateful sessions based on the C/S architecture and the maintenance of the complete MCP session lifecycle (initialization phase, running phase, and closing phase). |
| Transport | Definition of data transmission channels between various endpoints based on the C/S architecture, including an implementation scheme for the default standard stdio transmission channel and an optional SEE transmission channel. |
| Task | Definition and implementation of various MCP tasks, such as the three - way handshake and capability negotiation task during the initialization phase, and the processing tasks for major requests like Prompts, Resources, Tools, etc. Time - consuming tasks are executed asynchronously, while ordinary tasks are executed synchronously. |
| Entity | Definition of various protocol entity classes for MCP, mainly including the base class of the Server following the Singleton pattern. |

## ✅ Feature support brief
> 🎉 All features defined by the [MCP 2024-11-05 specification](https://modelcontextprotocol.io/specification/2024-11-05/index) are fully implemented.

| Feature | Remarks | Essential Features | Supported or not |
| --- | --- | --- | --- |
| Base Protocol | Core JSON-RPC message types | Yes | Yes |
| Lifecycle Management | Connection initialization, capability negotiation, and session control | Yes | Yes |
| Transports | stdio | Yes | Yes |
| Cancellation | Cancellation of in-progress requests through notification messages. | Yes | Yes |
| Progress | Progress tracking for long-running operations through notification messages. | Yes | Yes |
| Tools | Tools enable models to interact with external systems, such as querying databases, calling APIs, or performing computations. | Yes | Yes |
| Resources | Resources allow servers to share data that provides context to language models, such as files, database schemas, or application-specific information. | Yes | Yes |
| Transports | HTTP with Server-Sent Events (SSE) | No | Yes |
| Pagination | Pagination allows servers to yield results in smaller chunks rather than all at once. | No | Yes |
| Ping | Ping mechanism that allows either party to verify that their counterpart is still responsive and the connection is alive. | No | Yes |
| Custom Notifications | JSON-RPC 2.0 notifications that are sent by an MCP Server or Client using custom method names, intended for transmitting arbitrary business data. | No | Yes |
| Prompts | Prompts allow servers to provide structured messages and instructions for interacting with language models. | No | Yes |
| Completion | A standardized way for servers to offer argument autocompletion suggestions for prompts and resource URIs. | No | Yes |
| Logging | A standardized way for servers to send structured log messages to clients. | No | Yes |

## 📄 LICENSE
TinyMCP is licensed under the MIT License - see the LICENSE file for details.
