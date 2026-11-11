# TinyMCP 2026-07-28 协议支持实施计划

## 1. 目标与范围

在 TinyMCP 现有 2024-11-05 实现之上，增加 2026-07-28 协议版本支持，做到：

- 同一进程、同一套 SDK 同时支持 2024-11-05 与 2026-07-28。
- 按 2026-07-28 规范实现核心能力：无会话请求、`server/discover`、`_meta`、`resultType`、MRTR、`subscriptions/listen`、缓存字段、错误码、工具/资源/提示词新字段。
- 按 2026-07-28 核心规范及 MCP Apps 扩展实现 MCP Apps 的**服务端能力**（仅限 2026-07-28；2024-11-05 明确不支持 MCP Apps）。
- 严格按 `Doc/modelcontextprotocol/specification/2026-07-28` 与 `Doc/modelcontextprotocol/schema/2026-07-28` 实施。
- 不引入任何外部依赖，继续使用 jsoncpp、cpp-httplib 与现有基础设施。
- 建立零依赖的自动化测试体系（单元测试 + 集成测试），并按“每完成一个单元/一个特性”强制门禁执行。
- 版本演进可扩展：新增更老或更新协议版本时，只需增加“版本 profile（时代 + 能力/方法开关 + 错误映射）”，不修改时代识别、请求分发与序列化主干。

### 本期范围与不做项

| 项 | 决策 |
|---|---|
| Streamable HTTP | 本期不实现。2026-07-28 仅在 stdio 传输上提供；规范允许服务器选择传输 binding，但这不是完整的 transport 覆盖，README 必须明确说明。若后续要求完整传输支持，另立计划实现 Streamable HTTP。 |
| Tasks 扩展（`io.modelcontextprotocol/tasks`） | 不实现。 |
| OAuth / 授权 | 不涉及。 |
| MCP Apps 的 View↔Host `postMessage` 协议 | 不实现。该协议运行在宿主与 iframe 之间，不是 MCP Server SDK 的职责。 |
| 客户端 SDK | 不实现，项目仍为 Server-only。 |

### 实施修订（最终落地口径）

后续分层与封装重构对本文若干设计点做了收敛，落地时以本节为准：

1. 版本模型类型（`ProtocolEra/ProtocolFeature/ProtocolProfile/ProtocolContext/CProtocolRegistry`）最终位于 `Source/Protocol/Internal/`，不出现在公共头；对外仅通过 `CMCPServer` 暴露能力。
2. Step 4 的 `SetModernProtocolSupported(bool)` 与 `m_bModernSupported` 已删除；现代版本改为通过 `CMCPServer::SetSupportedProtocolVersions` 显式声明，未声明时仅服务 2024-11-05（现代支持 opt-in）。
3. `SetProtocolRegistry` 不再对外公开；测试通过内部 `ProtocolRegistryAccess` 注入假版本注册表。
4. 已声明但不在会话注册表中的现代版本返回 `-32022`（带 `supported`/`requested`）；会话型传输（HTTP+SSE）上的现代请求返回 `-32600`。
5. 2026-07-28 不提供 `ping`；现代 `ping` 返回 `-32601`。
6. 显式注册扩展后，legacy `initialize` 也会在 `capabilities.extensions` 中透传；未注册时不出现。

## 2. 规范依据（实施时逐条对照）

- 总体：`Doc/modelcontextprotocol/specification/2026-07-28/index.md`
- 变更清单：`.../changelog.md`
- 基础协议、`_meta`、`resultType`、错误码：`.../basic/index.md`
- 版本协商：`.../basic/versioning.md`
- stdio：`.../basic/transports/stdio.md`
- MRTR：`.../basic/patterns/mrtr.md`
- 订阅：`.../basic/patterns/subscriptions.md`
- 取消/进度：`.../basic/patterns/cancellation.md`、`progress.md`
- 发现：`.../server/discover.md`
- 工具：`.../server/tools.md`
- 资源：`.../server/resources.md`
- 提示词：`.../server/prompts.md`
- 缓存：`.../server/utilities/caching.md`
- 日志：`.../server/utilities/logging.md`
- 补全：`.../server/utilities/completion.md`
- 已弃用：`.../deprecated.md`
- Schema：`Doc/modelcontextprotocol/schema/2026-07-28/schema.json`（以下 `$defs` 为实现依据）
  - `RequestMetaObject`、`NotificationMetaObject`、`ResultMetaObject`
  - `ResultType`、`Result`、`CacheableResult`、`InputRequiredResult`
  - `ServerCapabilities`、`ClientCapabilities`、`Implementation`、`Icon`
  - `DiscoverRequest`、`DiscoverResult`
  - `SubscriptionsListenRequest`、`SubscriptionsListenRequestParams`、`SubscriptionsListenResult`、`SubscriptionsAcknowledgedNotificationParams`
  - `InputRequests`、`InputResponses`、`InputResponseRequestParams`
  - `Tool`、`ToolAnnotations`、`CallToolRequestParams`、`CallToolResult`
  - `Resource`、`ResourceTemplate`、`ResourceContents`、`ReadResourceResult`
  - `Prompt`、`GetPromptResult`
  - `ListToolsResult`、`ListResourcesResult`、`ListResourceTemplatesResult`、`ListPromptsResult`
  - `ContentBlock`、`AudioContent`、`ResourceLink`、`ToolUseContent`、`ToolResultContent`
  - `UnsupportedProtocolVersionError`、`MissingRequiredClientCapabilityError`、`HeaderMismatchError`

### 扩展依据（已入库）

MCP 扩展文档已下载到 `Doc/modelcontextprotocol/extensions/`，本计划以其为实施依据：

- 扩展机制与协商：`Doc/modelcontextprotocol/extensions/overview.md`
- MCP Apps 概览：`Doc/modelcontextprotocol/extensions/apps/overview.md`
- MCP Apps 构建指南：`Doc/modelcontextprotocol/extensions/apps/build.md`
- 扩展支持矩阵：`Doc/modelcontextprotocol/extensions/client-matrix.md`
- Tasks / Skills / Auth 概览：仅作背景，不在本期范围

关键规则（来自 `extensions/overview.md`）：

1. 扩展标识格式为 `{vendor-prefix}/{extension-name}`；MCP Apps 为 `io.modelcontextprotocol/ui`。
2. 现代版本（2026-07-28 及以后）：
   - 客户端在 `params._meta["io.modelcontextprotocol/clientCapabilities"].extensions` 中声明；
   - 服务端在 `server/discover` 的 `capabilities.extensions` 中声明。
3. 旧版本（2025-11-25 及更早，含本项目 2024-11-05）：通过 `initialize` 的 `params.capabilities.extensions` / `result.capabilities.extensions` 透传。该字段不在旧版 schema 中，**不是一致性要求**，但规范鼓励 SDK 透传以保持跨版本一致。
4. 服务端扩展设置对象：无设置时使用空对象 `{}`；`mimeTypes` 是客户端声明，不放在服务端设置里。
5. 扩展默认关闭，必须由开发者显式 opt-in。
6. 优雅降级：支持 UI 的工具在客户端不支持扩展时仍应返回有意义的文本内容。

> 注意：完整的 Apps 规范（`specification/*/apps.mdx`）未入库；本期只实现 `apps/overview.md` 与 `apps/build.md` 明确描述的服务端子集，不实现 View↔Host 的 `postMessage` 协议。

## 3. 现状分析

当前代码结构：

| 层 | 位置 | 现状 |
|---|---|---|
| 协议常量/枚举 | `Source/Protocol/Public/PublicDef.h` | 只有 2024-11-05 常量；`MessageType` 无现代消息；错误码只有 `-32001/-32002` 等旧值 |
| 消息模型 | `Source/Protocol/Message/BasicMessage.h/.cpp` | 无 `_meta`、无 `resultType`、无 `inputRequests/inputResponses`、无 `extensions`、Tool/Resource/Prompt 缺少 `title/icons/_meta/annotations`，Tool 缺少 `outputSchema` |
| 请求 | `Request.h/.cpp` | 无 `server/discover`、`subscriptions/listen`；无请求级 `_meta`、`inputResponses/requestState` |
| 响应 | `Response.h/.cpp` | 无 `resultType`、无 `inputRequests/requestState`、无 `ttlMs/cacheScope`、无结果级 `serverInfo` |
| 通知 | `Notification.h/.cpp` | 无 `subscriptions/acknowledged`；通知无 `subscriptionId` 元数据 |
| 会话 | `Session.h/.cpp` | 单例 + 状态机；`initialize` 握手；`Run()` 单读线程；未知 method 不返回错误；异步任务线程由 `notifications/initialized` 启动 |
| 任务 | `Task.h`、`BasicTask.h/.cpp` | 每类请求一个任务；分页错误响应漏设 `requestId`；资源不存在固定 `-32002` |
| 注册 API | `Entity/Server.h` | 只有 2024-11-05 能力/任务注册，无现代能力、扩展、发现信息注册 |
| 传输 | `Transport.h/.cpp`、`SSETransport.*` | stdio 行分隔 JSON；旧 HTTP+SSE；本期不改 |
| 示例 | `Example/MCPServer/Source/*` | Echo 示例；无 MRTR、无订阅、无 Apps |

### 2026-07-28 关键差异（必须落实）

1. **无会话**：现代请求不再使用 `initialize`/`notifications/initialized`；每个请求在 `params._meta` 中携带：
   - `io.modelcontextprotocol/protocolVersion`（必填）
   - `io.modelcontextprotocol/clientCapabilities`（必填，可为空对象）
   - `io.modelcontextprotocol/clientInfo`（可选）
   - `io.modelcontextprotocol/logLevel`（可选，请求级）
   - `progressToken`（可选，沿用旧键）
   - 必填字段缺失必须返回 `-32602`，**不得补默认值**。
2. **`server/discover` 必须实现**：返回 `supportedVersions`、`capabilities`、`instructions?`、`ttlMs`、`cacheScope`、`resultType`；`serverInfo` 位于 `result._meta["io.modelcontextprotocol/serverInfo"]`，不是 result 顶层。
3. **所有结果必须有 `resultType`**：`"complete"` 或 `"input_required"`。
4. **MRTR**：服务端返回 `resultType: "input_required"` + `inputRequests` + 可选 `requestState`；客户端重试原请求并带 `inputResponses`/`requestState`；MRTR 结果不得带缓存提示。
5. **订阅**：`resources/subscribe`/`resources/unsubscribe`/HTTP GET 被 `subscriptions/listen` 取代；过滤器位于 `params.notifications`；订阅通知带 `_meta["io.modelcontextprotocol/subscriptionId"]`；同时补齐 `notifications/tools/list_changed`、`notifications/prompts/list_changed`、`notifications/resources/list_changed` 的发送能力。
6. **移除**：`ping`、`logging/setLevel`、`notifications/roots/list_changed`（现代模式下）。
7. **缓存**：仅 `server/discover`、`tools/list`、`prompts/list`、`resources/list`、`resources/read`、`resources/templates/list` 结果必须带 `ttlMs`、`cacheScope`；`tools/call`、`prompts/get` 不带。
8. **日志**：现代模式使用请求级 `_meta.logLevel`；没有该字段的请求禁止发送 `notifications/message`；不得写入会话级 `m_loggingLevel`。
9. **错误码**：新增 `-32020 HeaderMismatch`、`-32021 MissingRequiredClientCapability`、`-32022 UnsupportedProtocolVersion`；legacy 的 `-32001` 常量保留原值；现代资源不存在用 `-32602`，legacy 继续 `-32002`。
10. **能力**：`ServerCapabilities`/`ClientCapabilities` 新增 `extensions`；现代 discover 用 `completions`，legacy `initialize` 仍输出 `experimental.completion`（不互相替代）；`ClientCapabilities.elicitation` 区分 `form`/`url`。
11. **内容类型**：新增 `AudioContent`、`ResourceLink`、`ToolUseContent`、`ToolResultContent`；`Tool` 新增 `title/icons/outputSchema/annotations/_meta`；所有 ContentBlock 补 `annotations` 与 `_meta`。
12. **传输头**：`Mcp-Method`/`Mcp-Name`/`x-mcp-header` 只适用于 Streamable HTTP，本期不实现（stdio 无此要求）；保留错误码常量但不在 stdio 使用。

## 4. 核心设计决策

### 4.0 复用与 2024-11-05 兼容硬约束（全局，优先于后续所有设计）

本节是强制约束：与后续章节冲突时，以本节为准；任何无法满足的条目必须停下来评审，不得自行放宽。

**A. 复用已有实现，禁止重复造轮子/基建**

1. 传输：复用 `CMCPTransport`、`CStdioTransport`、`CSSETransport`；不新增传输抽象层，不新建 Streamable HTTP 实现。
2. 会话与分发：现代能力作为 `CMCPSession` 内的分支（`ResolveProtocolContext` + `ProcessModernRequest`/`ProcessModernNotification`）；不新建第二个 Session、Server、消息总线或分发器。
3. 任务：复用 `CMCPTask` 原型 + `Clone()` + `CommitAsyncTask`/`CancelAsyncTask`；不新建并行任务框架。`NotifyInputRequired`/`NotifyMissingClientCapability` 放在现有任务基类，且必须是非纯虚/带默认实现，确保已有派生任务类重新编译即可通过。
4. 序列化：复用 `DoSerialize`/`DoDeserialize`、jsoncpp、`MSG_KEY_*` 常量；`MetaObject` 只是 `Request` 的字段，结果 `_meta` 用 `Response::jResultMeta` 承载，二者都不构成第二套消息体系；不引入任何 JSON 库。
5. 注册：复用 `CMCPServer<T>` 现有 `Register*` 模板与 `CMCPSession` 的服务端注册表；新 API 只做追加。UI 资源复用 `RegisterServerResources`，不新建资源注册体系。`CProtocolRegistry` 是只读的“协议版本配置表”，不是第二套服务端能力注册体系，也不替代 `Register*`。
6. 错误响应：复用 `ErrorResponse` 与 `ERRNO_*`；只新增缺失常量，不重命名/不改旧值、不新建错误类型。
7. 分页/异步/取消/进度：全部复用现有机制；`NotifyResult`/`NotifyError`/`NotifyProgress` 不改签名。
8. 测试与构建：复用现有 `Tests/` 目录、CTest、`.sln`/`.vcxproj`/CMake 结构；新增单头文件 harness 仅为满足“零外部依赖”，不引入 gtest/catch2。
9. 禁止新增平行的 `modern/` 目录或 `Modern*` 类族；新类型必须是现有类族的增量（如 `DiscoverRequest : Request`、`DiscoverResult : Response`）。

**B. 2024-11-05 重新编译即无缝，功能零变化**

1. 总开关默认关闭：`ctxProtocol` 默认 `2024-11-05 / Era_Handshake`，所有现代 feature 为 false；所有新增字段默认“未设置”。旧代码直接构造 `Request`/`Response`/`Tool` 等对象并序列化，结果必须与升级前一致。`ProtocolContext` 默认构造只做注册表查询得到指向 `2024-11-05` profile 的常量指针，**不修改任何全局状态，不影响 legacy 输出与既有调用链**。
2. 新字段一律“有值才写”：2024-11-05 schema 中不存在的字段，在 legacy 下一律不输出。包括但不限于：
   - `resultType`、`inputRequests`、`inputResponses`、`requestState`、`ttlMs`、`cacheScope`、`structuredContent`；
   - `Tool`/`Resource`/`ResourceTemplate`/`Prompt` 的 `title`、`icons`、`annotations`、对象级 `_meta`，以及 Tool 独有的 `outputSchema`（请求级 `progressToken` 见第 3 条）；
   - `Implementation` 的 `websiteUrl`、`icons`；
   - `ContentBlock` 的 `annotations`、`_meta` 及 `audio`/`resource_link`/`tool_use`/`tool_result` 新类型；
   - `ServerCapabilities.completions`、`extensions`（唯一例外：应用显式注册扩展时，legacy `initialize` 结果按扩展规范透传 `capabilities.extensions`；旧代码不调用即无变化）。
3. `_meta` 逐字节兼容：legacy 只在 `progressToken` 存在时输出 `params._meta.progressToken`，与当前 `Request::DoSerialize` 行为一致；不得输出空 `_meta`、不得注入 `serverInfo`/`protocolVersion`/`clientCapabilities`。
4. 既有解析保留：`CallToolRequest` 从 `params._meta.toolCallId` 读取的现有行为保持不变；`toolCallId` 仍由 `CallToolRequest` 现有成员承载，`params._meta` 的组装/解析不得丢失它。`progressToken` 同理仍由 `Request::progressToken` 承载，不迁入 `MetaObject`。
5. 错误码不变：`-32001`/`-32002` 等旧值不变；legacy 资源不存在仍 `-32002`；legacy 成功路径的响应内容与字段顺序不变。
6. API 不变：现有公开类型、方法签名、默认参数、行为不变；新增 API 只做追加；不删除、不重命名、不改变现有成员语义。
7. 校验不变：`IsValid()` 的 legacy 语义不得因新增字段改变；现代必填项（`resultType`、`protocolVersion` 等）只在对应 `ctxProtocol.Supports(...)` 为真时校验，legacy 不因新增字段变为无效。
8. 状态不影响：legacy `initialize`/`notifications/initialized`/异步任务线程/取消/SSE 订阅保持现状；现代请求不得读写 `m_loggingLevel`、不得污染 `m_hashMessage`、不得改变 `SessionState` 迁移。
9. 零 legacy 行为变化：除本计划明确列为“新增探测”的 `server/discover` 外，2024-11-05 路径的任何现有 method、错误码、静默行为、状态迁移均不得改变。`server/discover` 是老代码从未发送的新 method，legacy 模式下对它返回 `-32601` 不构成既有功能变化，且是版本回退探测的必要前提。
10. 门禁：7.2 的 golden 用例必须覆盖所有现有方法（initialize、tools/list、tools/call、resources/list、resources/read、resources/templates/list、resources/subscribe、resources/unsubscribe、prompts/list、prompts/get、completion/complete、logging/setLevel、ping、通知）。任一 legacy 输出差异直接判定失败，禁止“接受新输出”。

### 4.1 版本注册表、时代识别与演进（Version Registry）

**目标**：本期只实现 `2024-11-05` 与 `2026-07-28`，但识别、分发、序列化不得写死“只有两个版本/二选一”。新增更老或更新版本时，优先只增加版本 profile 数据，不改主干。注册表只描述“版本与能力开关”，**不定义任何规范之外的协议字段**；所有字段仍以 `Doc/modelcontextprotocol/specification/2026-07-28` 与 `schema/2026-07-28` 为准。

**版本模型**（最终位于 `Source/Protocol/Internal/Protocol.h/.cpp`，公共头不暴露）：

- `enum ProtocolEra { Era_Handshake, Era_Stateless };`
  - `Era_Handshake`：`initialize` + 会话状态机（2024-11-05）。
  - `Era_Stateless`：请求级 `_meta`、无会话（2026-07-28；未来版本通常沿用此模型）。
  - 时代表示“交互模型”，不是版本序号；不得用“新/旧”布尔值代替。
  - **唯一的架构级扩展点**：只有出现第三种交互模型（既非 Handshake 也非 Stateless）时才需新增 `ProtocolEra` 枚举值与一处时代分支；仅新增版本号不需要改本枚举。这一边界必须在评审时显式确认。
- `CProtocolRegistry`/`ProtocolProfile`/`ProtocolContext` 是轻量配置类型，位于内部目录且不对外公开；仅使用标准库 `std::bitset`/`std::set`，不引入外部依赖。
- `enum ProtocolFeature`：按能力/字段组划分，例如 `Feature_ResultMeta`、`Feature_EntityMeta`、`Feature_ImplementationExtras`、`Feature_ResultType`、`Feature_Discover`、`Feature_Caching`、`Feature_Mrtr`、`Feature_ListenSubscriptions`、`Feature_Extensions`、`Feature_CompletionsCapability`、`Feature_ToolTitle`、`Feature_ToolAnnotations`、`Feature_StructuredContent`、`Feature_ContentAudio`、`Feature_ContentResourceLink`、`Feature_ContentToolUse`、`Feature_ContentAnnotations`、`Feature_LogLevelInMeta`。新增版本时按需增加枚举值。请求侧 `_meta` 的解析由 `Era_Stateless` 直接驱动，不设 feature 位。
- `struct ProtocolProfile`
  - `std::string strVersion`
  - `ProtocolEra eEra`
  - `std::bitset<Feature_Count> bitsFeatures`（或 `uint32_t` 位掩码）
  - `std::set<std::string> setSupportedMethods`
  - `int iResourceNotFoundCode`（旧版本 `-32002`，现代 `-32602`）
  - 其他版本级差异（如错误码映射）放同一 profile
- `class CProtocolRegistry`（只读版本表：提供默认全局实例，同时允许以 profile 列表构造测试实例；不是可修改的业务单例）
  - `const ProtocolProfile* Find(const std::string& strVersion) const`
  - `const ProtocolProfile* GetDefaultHandshakeProfile() const`（最高 Handshake 版本）
  - `const ProtocolProfile* GetLatestStatelessProfile() const`
  - `std::vector<std::string> GetSupportedVersions() const`
  - `std::vector<std::string> GetSupportedVersions(ProtocolEra) const`
  - 版本优先级由注册表的显式声明顺序决定（默认实例按“现代在前、握手在后”声明），不依赖字符串比较。
  - 构造可接受 profile 列表：默认全局实例由内置 profile 构造；测试可注入自定义实例（假版本 + 一个 Handshake profile），用于验证“只加 profile 即可工作”。`CMCPSession` 持有 `const CProtocolRegistry* m_pRegistry`，默认指向全局实例；注入实例的生命周期必须覆盖 Session（`ProtocolContext::pProfile` 指向其内部 profile）。
  - 注入实例必须至少包含一个 `Era_Handshake` profile；`ProtocolContext` 默认构造始终使用内置全局注册表的 `2024-11-05` profile，不随注入实例变化，保证任何未显式设置版本的对象仍为有效 legacy。Handshake 请求由 Session 用其注册表的 `GetDefaultHandshakeProfile()` 设置 `ctxProtocol.strVersion`，因此新增更老 Handshake 版本时 `initialize` 结果自动使用该版本。
- `struct ProtocolContext`
  - `std::string strVersion`
  - `const ProtocolProfile* pProfile`（指向注册表静态对象，生命周期稳定）
  - `bool IsValid() const`
  - `bool IsStateless() const`（仅表示交互模型，**不用于字段门控**）
  - `bool Supports(ProtocolFeature eFeature) const`
  - 默认构造 = `2024-11-05` / `Era_Handshake`，`pProfile` 指向**内置全局注册表实例**的 `GetDefaultHandshakeProfile()`（不读取 Session 注入的实例），保证旧代码直接构造对象时仍为 legacy 且 `IsValid()` 为真。
  - 另提供 `static ProtocolContext Invalid(const std::string& strRequested)`（`pProfile == nullptr`，`IsValid()` 为 false），专用于“版本未知/解析失败”；**不得用默认构造表示失败**。
  - 注册表全局实例采用函数内静态初始化（C++14 线程安全），避免静态初始化顺序问题；`pProfile` 指向的 profile 在整个进程生命周期内稳定。

**本期版本表**：

| 版本 | 时代 | 关键 feature / 方法差异 |
|---|---|---|
| `2024-11-05` | Handshake | 无现代 feature；`iResourceNotFoundCode = -32002`；方法集为现有 legacy 方法 |
| `2026-07-28` | Stateless | `ResultMeta`、`EntityMeta`、`ImplementationExtras`、`ResultType`、`Discover`、`Caching`、`Mrtr`、`ListenSubscriptions`、`Extensions`、`CompletionsCapability`、`LogLevelInMeta`、`ToolTitle`/`ToolAnnotations`/`ContentAnnotations` 及各类 Content 新字段；`iResourceNotFoundCode = -32602` |

- `supportedVersions`、`-32022.data.supported`、`initialize` 回退版本、默认版本全部从注册表生成，**禁止手写版本列表**。
- 本期只注册两个 profile；未知版本一律 `-32022`，不会误落 legacy。

**识别**（`ResolveProtocolContext`）：

1. `params._meta` 含 `io.modelcontextprotocol/protocolVersion` 或任意 `io.modelcontextprotocol/*` 键 → 按 `Era_Stateless` 候选处理。
2. 取 `protocolVersion` → `CProtocolRegistry::Find()`：
   - 找到且为 `Era_Stateless` → 使用该 profile；
   - 找到且为 `Era_Handshake` → 该版本不定义请求级 `_meta`，按 Handshake 流程处理，不进入现代分支；
   - 未找到 → `-32022`（`supported` 来自注册表）；
   - 缺 `protocolVersion`/`clientCapabilities` → `-32602`，不补默认值。
3. 无现代 `_meta` 键且 `method == "initialize"` → `Era_Handshake`。
4. 无现代 `_meta` 键且已处于 legacy 状态 → `Era_Handshake`。
5. 无现代 `_meta` 键且未初始化 → 维持现有 legacy 行为（不改）。
6. `initialize` 的 `protocolVersion` 若属于 `Era_Stateless`（如 `2026-07-28`），仍按 Handshake 协商规则返回 `GetDefaultHandshakeProfile()` 的版本，由客户端决定是否继续；不当作现代请求。

- 只有 `progressToken`/`toolCallId`/OpenTelemetry 等非现代键的 `_meta` 仍按 Handshake 处理。
- `Era_Stateless` 分支不依赖 `SessionState`，也不读写会话级 `m_loggingLevel`。
- 双模型可在同一进程/同一 stdio 并存：逐个请求解析 `ProtocolContext`；`initialize` 只影响 Handshake 状态机。

**演进操作手册**（作为计划/README 的一部分）：

- 新增“更新版本”：在注册表加 profile（时代、feature、方法表、错误映射）。若只是能力增减，`Session`/`Message`/序列化主干零修改；仅新增字段时在对应 `DoSerializeVersionExtras` 内按 feature 输出。
- 新增“更老 Handshake 版本”：加 profile 与方法表；握手分支从 profile 读取版本与能力，不新增版本专用的 `if`。
- 新增 method：才需要按现有模式加 `MessageType` + `Request/Response` + 任务；同时在 profile 的 `setSupportedMethods` 登记。
- 新增交互模型（era）：才需要加 `ProtocolEra` 枚举值与对应分支；这是唯一会触及分发主干的情况，必须在计划评审时单独批准。
- 每次新增版本必须补 `unit.version_registry` 与 `integration.version_extensibility`（用测试专用假版本验证“只加 profile 即可工作”）。

### 4.2 序列化隔离

在 `MCP::Message`（被 `Request`/`Response`/`Notification` 继承）新增 `ProtocolContext ctxProtocol`，默认 `2024-11-05 / Era_Handshake`：

- 由 `ParseMessage` 解析后设置到请求/通知对象；任务构建结果时从请求复制到结果；`Clone()`/`SetRequest()` 必须一并复制，避免异步任务丢失版本上下文。
- 本次对 `Message`/`CMCPSession` 只**追加成员**（`ctxProtocol`、注册表快照等）与追加方法，不改变任何现有成员语义、方法签名与默认参数；约束目标是“旧代码重新编译即可、功能不变”，不承诺跨编译器版本的二进制 ABI 兼容。
- 字段门控一律使用 `ctxProtocol.Supports(Feature_X)`，**禁止使用 `IsStateless()` 或版本字符串比较来门控字段**。这样未来版本增加/移除字段只需调整 profile 的 feature 位。
- 现代专属字段只有在对应 feature 为真时序列化，包括：
  - `Feature_ResultType` → `resultType`；`Feature_Mrtr` → `inputRequests`/`requestState`；`Feature_StructuredContent` → `structuredContent`；`_meta.serverInfo` 由 `Feature_ResultMeta` 控制；
  - `Feature_ToolTitle` → Tool/Resource/ResourceTemplate/Prompt 的 `title`/`icons`，以及 Tool 的 `outputSchema`；`Feature_ToolAnnotations` → Tool 的 `annotations`/`ToolAnnotations`（Resource/ResourceTemplate/Prompt 的 `annotations` 自 2024-11-05 即存在，不门控）；`Feature_ImplementationExtras` → `Implementation` 的 `title`/`description`/`icons`/`websiteUrl`；
  - `Feature_EntityMeta` → Tool/Resource/ResourceTemplate/Prompt/Content/`ResourceContents` 的对象级 `_meta`（与 title/annotations 解耦，未来只开扩展不开标题时仍能输出 `_meta`）；
  - `Feature_ContentAudio`/`Feature_ContentResourceLink`/`Feature_ContentToolUse` → 对应新增 Content 类型；所有 ContentBlock 的 `annotations` 自 2024-11-05 即存在，不门控；`Feature_AnnotationLastModified` → `annotations.lastModified`（2026-07-28 新增）；
  - `Feature_Extensions` → `capabilities.extensions`（唯一例外：显式注册扩展时 Handshake 的 `initialize` 也透传；这是扩展规范要求，不属于版本 feature）；`Feature_CompletionsCapability` → `capabilities.completions`（legacy 仍用 `experimental.completion`）。
- 新增可选字段一律“有值才写”：即使 feature 开启，未赋值的可选字段也不输出空对象/空数组，避免无意义 diff。
- 数据对象（`Tool`/`Resource`/`ResourceTemplate`/`Prompt`/`Implementation`/各 `Content` 子类）不持有版本上下文（它们由服务端原型共享，异步线程序列化时加字段会竞态）。统一新增 `void DoSerializeVersionExtras(Json::Value& jObj, const ProtocolContext& ctx) const`，只写该对象在 `ctx` 下允许的新增字段；**现有 `DoSerialize` 签名与行为不变**。旧调用链不传 `ctx`、不调用 extras；现代调用链在基础 `DoSerialize` 之后调用。旧代码直接 `tool.DoSerialize(jObj)` 的输出因此不受影响。
- 旧代码直接构造 `Request`/`Response`/`Tool` 并序列化时，`ctxProtocol` 默认为 `2024-11-05 / Era_Handshake`，所有 feature 为 false，输出必须与升级前逐字节一致。
- `ttlMs`/`cacheScope`：`server/discover` 由 `Feature_Discover` 必出；其余 5 类（tools/list、prompts/list、resources/list、resources/templates/list、resources/read）由 `Feature_Caching` 控制，且只在 `resultType == "complete"` 且 `!bMrtrRetry` 时序列化；`tools/call`、`prompts/get` 不写。
- **结果字段写入顺序**：现有响应子类先构建 `jResult`，再 `jMsg["result"] = jResult`，而 `Response::DoSerialize()` 只写 `id`。因此版本扩展字段不能放在 `Response::DoSerialize()` 里写，否则会被子类覆盖。统一改为在 `Response` 增加非虚辅助方法 `void WriteVersionResultFields(Json::Value& jResult, const ProtocolContext& ctx, const Json::Value& jServerInfo) const`（feature 全关时 no-op；`jServerInfo` 为空时不写 `serverInfo`，避免 `Response` 反向依赖 `CMCPSession`），由每个结果子类在赋值 `jMsg["result"]` 之前调用；新增的 `DiscoverResult`/`SubscriptionsListenResult` 同样调用。`Response` 增加 `Json::Value jServerInfo` 与 `bool bMrtrRetry{false}`；`CMCPSession::PrepareResult(const std::shared_ptr<Request>&, const std::shared_ptr<Response>&)` 在序列化前统一复制 `ctxProtocol`、`jServerInfo`（当前 `ServerInfo` 序列化结果）与 `bMrtrRetry`（请求携带 `inputResponses` 或 `requestState`），各任务在 `Serialize()` 前调用，避免逐个 `NotifyResult` 重复。
- **通知字段写入顺序**：同理，`Notification` 增加 `void WriteVersionParamsMeta(Json::Value& jParams, const ProtocolContext& ctx) const`，由各通知子类在构建完 `params` 后、写入消息前调用，确保 `subscriptionId` 与业务字段合并而非覆盖。`Notification` 同时增加 `MCP::RequestId subscriptionId` 与 `bool bHasSubscriptionId{false}`，由 `SendSubscriptionNotification` 在发送前设置；`WriteVersionParamsMeta` 通过 `RequestId::WriteValue()` 写入 `io.modelcontextprotocol/subscriptionId`，并保持原始 string/integer 类型。
- **新增字段的演进方式**：加 `ProtocolFeature` 枚举值 → 在 `2026-07-28` 及后续 profile 置位 → 在对应 `DoSerializeVersionExtras`/`WriteVersionResultFields` 中按 feature 输出；不修改旧字段的序列化代码，也不在主干里写版本号判断。

### 4.3 `_meta` 模型

新增 `MetaObject`：

- 固定字段：`protocolVersion`、`clientInfo`、`clientCapabilities`、`logLevel`。序列化映射：它们写为 `io.modelcontextprotocol/protocolVersion`、`io.modelcontextprotocol/clientInfo`、`io.modelcontextprotocol/clientCapabilities`、`io.modelcontextprotocol/logLevel`。
- `progressToken` **继续使用现有 `Request::progressToken` 成员**，不放进 `MetaObject`，避免同一字段两处存储；`toolCallId` 继续保留在 `CallToolRequest` 现有成员中。`logLevel` **只保存在 `MetaObject` 中**，`Request` 不另存副本。
- 保留 `Json::Value jExtra` 透传未知键（例如 OpenTelemetry 键）；序列化时先写 `jExtra`，再写已知字段。
- **必须改造现有 `Request::DoSerialize/DoDeserialize`**：`params._meta` 只在组装后非空时创建；组装顺序为 `jExtra` → `Request::progressToken`（若有效）→ 现代命名空间键，禁止出现“两处各写一次、后者覆盖前者”。
- 请求序列化时写入 `params._meta`，但**仅在 `_meta` 非空时创建该对象**：legacy 下只有 `progressToken`（或应用显式设置的旧键）会输出，保持逐字节一致；现代下写入命名空间键。legacy 不得输出空 `_meta`。
- 结果序列化时通过 4.2 的 `WriteVersionResultFields(jResult, ctxProtocol, jServerInfo)` 写入 `result._meta`，并附加 `io.modelcontextprotocol/serverInfo`。非协议保留键以应用值为准；协议保留键（`io.modelcontextprotocol/serverInfo`、`io.modelcontextprotocol/subscriptionId`）由 SDK 写入并覆盖同名应用值，避免应用自定义 `_meta` 被整体覆盖的同时保证关联/身份键不可伪造；`jServerInfo` 为空时不写该键。
- legacy `initialize` 的 `clientInfo` 继续使用现有 `ClientInfo`/`Implementation` 路径，不被 `MetaObject` 替换，避免握手输出变化。
- `clientCapabilities` 保存原始 JSON；提供 `SupportsCapability("elicitation"/"sampling"/"roots")` 与 `SupportsExtension(id)` 辅助方法。
- 必填校验：`protocolVersion`、`clientCapabilities` 缺失 → `-32602`；不补默认值。
- `logLevel` 只保存在 `Request::meta` 中，供该请求及其异步任务使用，不写回 `CMCPSession::m_loggingLevel`。

### 4.4 MRTR

- 仅 `tools/call`、`resources/read`、`prompts/get` 三类请求允许返回 `input_required`；其他请求禁止返回该结果类型。
- `params.inputResponses`、`params.requestState` 按 schema 可出现在任意客户端请求上；`Request` 基类统一承载 `MCP::InputResponses inputResponses` 与 `std::string strRequestState`，非三类请求只透传、不消费。
- `Response` 新增：`std::string strResultType{"complete"}`、`MCP::InputRequests inputRequests`、`std::string strRequestState`。
- `InputRequiredResult` 必须至少包含 `inputRequests` 或 `requestState` 之一；两者都没有时不得返回 `input_required`。`NotifyInputRequired(...)` 在两者皆空时返回 `ERRNO_INVALID_PARAMS`，不写出结果。
- 任务基类提供 `NotifyInputRequired(...)`：写入结果并结束当前任务；由应用在 `Execute()` 中调用。
- 发送前校验客户端能力：`elicitation/create` → `clientCapabilities.elicitation`，并按 `mode` 细分（`form` 需 `elicitation.form`，`url` 需 `elicitation.url`）；`sampling/createMessage` → `sampling`；`roots/list` → `roots`。客户端未声明对应能力时：
  - `NotifyInputRequired(...)` **拒绝**把该请求放进 `inputRequests`，返回 `ERRNO_MISSING_REQUIRED_CLIENT_CAPABILITY` 给应用，不写结果；
  - 若应用确认该请求无法降级、必须依赖该能力才能完成，可调用 `NotifyMissingClientCapability(caps)`，由 SDK 返回 `-32021`，`error.data.requiredCapabilities` 填 `ClientCapabilities` 形状的最小对象（如 `{"elicitation":{}}`）。
- `inputResponses` 与 `requestState` 相互独立，均可单独出现：`InputRequiredResult` 只有 `inputRequests` 时，客户端重试可以只带 `inputResponses`；只有 `requestState` 时（如 load shedding），重试只带 `requestState`。SDK 不强制二者成对，也不补空值。
- `requestState` 对 SDK 不透明，只负责透传。应用若让 `requestState` 影响授权、资源访问或业务逻辑，必须按规范自行做完整性保护（HMAC/AEAD）并校验失败即拒绝；还应考虑绑定主体、短 TTL 与请求标识以防重放。SDK 不代做，代码注释必须写明这一点。
- `input_required` 结果不得写 `ttlMs`/`cacheScope`；携带 `inputResponses` 或 `requestState` 的重试结果也不得写缓存提示（见 4.6）。
- `roots/list` 与 `sampling/createMessage` 在 2026-07-28 已标记 deprecated（最早 2027-07-28 移除）；本期仍按 MRTR 支持，README 注明迁移建议（工具参数/资源 URI、直接调用 LLM API），不得因 deprecated 而拒绝实现或改变 legacy 行为。

### 4.5 订阅

- 新增请求 `subscriptions/listen`，参数按 `SubscriptionsListenRequestParams`，过滤器嵌套在 `params.notifications` 下：
  - `notifications.toolsListChanged`、`notifications.promptsListChanged`、`notifications.resourcesListChanged`（bool；缺省即未订阅）
  - `notifications.resourceSubscriptions`（URI 数组）
- 请求由读线程同步处理，顺序固定：
  1. 读取并校验 `params.notifications`；
  2. 建立订阅记录，状态先标记为“未激活”，确保此时不投递任何通知；
  3. 发送 `notifications/subscriptions/acknowledged`，`params._meta["io.modelcontextprotocol/subscriptionId"]` = listen 请求 id，`params.notifications` 只回执实际授予的子集；
  4. 写出发送成功后，才把订阅标记为“已激活”并开始投递。
  - ack 必须是该订阅的第一条消息；stdio 上其他订阅的消息可以插入。
- 订阅通知的 `params._meta` 写入 `io.modelcontextprotocol/subscriptionId`，值必须保留 listen 请求 id 的 JSON-RPC 类型（字符串或整数），不能统一转成字符串。
- 订阅表 key 使用 `RequestId` 的联合键（数据类型 + 值），提供 `MakeRequestIdKey()` 生成稳定字符串键；禁止用会丢失类型的 `asString()` 作 key。
- 收到 `notifications/cancelled` 且 `requestId` 指向 listen 请求时，先移除订阅再处理取消；传输结束/`Terminate` 时清空订阅表。
- 优雅关闭：`Terminate` 时对仍存活的订阅发送 `subscriptions/listen` 的 `complete` 结果（`resultType:"complete"`），`result._meta["io.modelcontextprotocol/subscriptionId"]` 必填；发送前用监听请求调用 `PrepareResult`（或显式设置 `ctxProtocol`/`jServerInfo`），保证 `resultType`/`serverInfo` 正确；随后清空订阅表。
- 现代模式下，只有被订阅的类型才发送通知；legacy 模式保持现有 `resources/subscribe` 行为（不带 subscriptionId）。
- 通知发送 API：`CMCPSession::SendSubscriptionNotification(method, params, uri)`，按 method 选择订阅（list_changed 类看对应布尔位，`resources/updated` 看 `resourceSubscriptions` 是否包含该 URI），复制订阅快照后逐个写入；订阅表加互斥锁，允许应用工作线程调用。
- `resources/updated` 只做**订阅 URI 精确匹配**，删除自创的“前缀匹配”。若应用需要通知子资源变化，调用方用“已订阅 URI”参与匹配，`params.uri` 传实际变化的子资源 URI（schema 允许 `uri` 是子资源）。

### 4.6 缓存

- 仅以下 6 类 `resultType == "complete"` 的结果必须带 `ttlMs`（int，>= 0）与 `cacheScope`（`"public"`/`"private"`）：
  `server/discover`、`tools/list`、`prompts/list`、`resources/list`、`resources/templates/list`、`resources/read`。
- `tools/call`、`prompts/get`、`completion/complete`、`subscriptions/listen` 以及所有 `input_required` 结果不写缓存字段；携带 `inputResponses` 或 `requestState` 的重试结果同样不写。
- 服务端级默认值：`ttlMs = 0`、`cacheScope = "private"`（0 表示立即可过期，安全优先）。
- 新增注册 API `SetServerCacheOptions(int ttlMs, const std::string& strCacheScope)`，应用可覆盖；同一列表的所有分页必须使用相同 `cacheScope`。
- `server/discover` 的这两个字段由 `Feature_Discover` 保证必出（schema 必填，不受 `Feature_Caching` 影响）；其余 5 类由 `Feature_Caching` 控制；legacy 完全不输出。

### 4.7 MCP Apps（服务端子集）

以已入库的 `Doc/modelcontextprotocol/extensions/apps/apps.mdx` 为准实施：

- 版本边界：MCP Apps 仅随 2026-07-28 提供；2024-11-05 明确不支持 MCP Apps，既不声明该扩展，也不输出任何 UI 元数据（`Tool._meta.ui`、`Resource._meta.ui`、`text/html;profile=mcp-app` 内容）。
- 扩展标识：`io.modelcontextprotocol/ui`；MIME：`text/html;profile=mcp-app`；URI 方案：`ui://`（保留给 MCP Apps）。
- 默认关闭：只有调用 `RegisterServerAppsCapabilities()` 后才声明扩展；未注册时 `server/discover` 不出现该扩展；legacy `initialize` 即使已注册也不声明。
- 服务端能力声明：`server/discover` 的 `capabilities.extensions["io.modelcontextprotocol/ui"] = {}`；设置对象为空 `{}`，`mimeTypes` 是客户端声明，不放在服务端设置里。
- 客户端能力识别：通过 `ClientCapabilities::FindExtensionSettings()`/`SupportsMimeType()` 读取现代 `_meta["io.modelcontextprotocol/clientCapabilities"].extensions["io.modelcontextprotocol/ui"].mimeTypes`，供应用按请求判断宿主是否支持 UI；注册期无法按连接注册工具变体。
- 工具关联：`Tool._meta.ui.resourceUri` 指向 `ui://...` 资源，`Tool._meta.ui.visibility` 取值仅 `model`/`app`；提供 `SetUiResourceUri()`/`SetUiVisibility()` setter，不输出已废弃的扁平 `_meta["ui/resourceUri"]`。
- 资源元数据：通过 `UiResourceMeta` 建模 `csp`（`connectDomains`/`resourceDomains`/`frameDomains`/`baseUriDomains`）、`permissions`（`camera`/`microphone`/`geolocation`/`clipboardWrite`）、`domain`、`prefersBorder`；未知字段用 `jExtra` 透传。`Resource`/`TextResourceContents`/`BlobResourceContents` 通过 `SetUiMeta()` 写入，调用者不手写 `jMeta`。
- `resources/read` 返回 `mimeType = text/html;profile=mcp-app` 的 text 或 blob 内容，并带 `_meta.ui`；SDK 不做 HTML5 合法性与运行时 MIME 强制。
- 配置校验：`Start()` 在注册了 Apps 时校验 `ui://` 资源 MIME、工具 `resourceUri` 的 scheme 与引用完整性、`visibility` 取值，失败返回 `-32602`；`RegisterServerUiResources()` 为空的 `ui://` 资源补默认 MIME。
- 优雅降级：带 UI 的工具仍必须返回有意义的文本 `content`，确保不支持 Apps 的客户端可正常使用。
- 不实现 `ui/initialize`、`ui/message`、`ui/open-link`、`ui/update-model-context` 等 View↔Host 方法；在 README/注释中明确 SDK 边界。

### 4.8 日志

- Legacy：保持 `logging/setLevel` + 会话级 `m_loggingLevel`。
- Modern：从请求 `_meta.logLevel` 取级别（仅 `Feature_LogLevelInMeta` 为真时解析）；没有该字段时任务层不得发送 `notifications/message`。`SendLogMessage` 的会话级过滤只服务 legacy；现代路径必须走 `ProcessRequest::NotifyLogMessage` 的请求级过滤，并复用不读取 `m_loggingLevel` 的底层写入辅助（如新增私有 `WriteLogNotification`），禁止让 `SendLogMessage` 在 modern 下回落到会话级别。
- Modern 请求不得读写 `CMCPSession::m_loggingLevel`；`logLevel` 只存在于 `Request::meta` 中，异步任务从其持有者读取，避免读线程状态被异步线程读取产生竞态。
- 规范将 `logging` 标记为 deprecated（保留期由规范定义）；本期仍按规范实现请求级 `logLevel`，README 必须注明 deprecated 状态并建议新实现使用 stderr/OpenTelemetry，不得借此恢复会话级用法。Modern `server/discover` 在启用日志时仍应声明 `logging` 能力（规范要求发送日志必须声明），但 `logging/setLevel` 方法不登记。

### 4.9 错误处理修正（与服务端正确性直接相关）

1. **未知 method**：
   - 现代路径：返回 `-32601`（带原 `id`）。
   - Legacy 路径：仅对 `server/discover`（老代码不会发送的新 method）返回 `-32601`，用于版本回退探测；其他未知 method 保持现有行为不变，避免任何 2024-11-05 行为变化。
2. **分页非法 cursor**：
   - 现代路径的四类 list 任务必须写入 `requestId` 并返回 `-32602`。
   - Legacy 路径保持现状（不纳入本期行为修复），确保老版本功能零变化。
3. **资源不存在**：错误码取 `ctxProtocol.pProfile->iResourceNotFoundCode`（现代 `-32602`，legacy `-32002`）；现代不得发出 `-32002`。未来版本改错误码只改 profile，不在任务内加分支。
4. **现代必填 `_meta` 缺失**：请求带任一 `io.modelcontextprotocol/*` 键但缺 `protocolVersion`/`clientCapabilities` 时返回 `-32602`，不补默认值；仅当请求没有任何现代 `_meta` 键时按 legacy 语义处理（`initialize` 握手本身也走 legacy 路径），不要把“缺 `_meta`”误判为 `-32600`。
5. **`-32021`**：当处理请求确实需要客户端未声明的能力、且无法降级时返回，`data.requiredCapabilities` 必填；SDK 不把未声明能力的请求塞进 `inputRequests`（那是规范禁止的行为）。
6. **`initialize` 收到现代 `_meta`**：`initialize` 始终按 Handshake 协商处理（versioning.md：`initialize` selects legacy semantics），即使携带现代 `_meta` 也返回 `GetDefaultHandshakeProfile()` 的版本，不返回 `-32600`；`-32600` 只用于“未启用现代协议的传输上收到现代请求”的边界。
7. **未知协议版本**：返回 `-32022`，`data.supported` 来自 `CProtocolRegistry::GetSupportedVersions()`，`data.requested` 为客户端原值；不得回落到 legacy 静默处理。

## 5. 实施步骤（按顺序）

### 测试门禁（强制，适用于所有步骤）

1. **单元测试门禁**：每完成一个独立编码单元（Step 1–Step 8 中的每个文件组/子项），必须构建并运行对应单元测试；全部通过后才能进入下一个单元。
2. **集成测试门禁**：每完成一个相对独立的功能/特性（发现与版本协商、现代核心请求、MRTR、订阅、缓存、MCP Apps、双版本共存、legacy 回归），必须运行对应集成测试；全部通过后才能标记该特性完成。
3. **失败即停**：测试失败时不得继续后续功能开发，不得跳过、注释或弱化断言；先修复再继续。
4. **回归**：任何后续改动都必须重新运行受影响的单元/集成测试，禁止破坏已通过的特性。
5. **提交门禁**：全量 `ctest --output-on-failure` 必须通过后才允许提交或合并。
6. **测试环境**：不访问真实网络、不依赖外部服务、不使用随机数据；时间相关断言使用固定值；集成用例必须设置超时。

#### 单元/集成分工与门禁映射

| 编码单元 | 完成后必须运行的单元测试 | 完成后必须运行的集成测试 |
|---|---|---|
| Step 1 常量与枚举 | `unit.constants`、`unit.version_registry` | — |
| Step 2 消息模型 | `unit.message`、`unit.content_blocks`、`unit.meta` | — |
| Step 3 请求/响应/通知 | `unit.request`、`unit.response`、`unit.notification`、`unit.legacy_compat` | — |
| Step 4 会话双时代分发 | `unit.session` | — |
| Step 5 任务层 | `unit.tasks`、`unit.mrtr`、`unit.subscriptions`、`unit.caching` | `integration.discover`、`integration.version_fallback`、`integration.version_extensibility`、`integration.modern_core`、`integration.mrtr`、`integration.subscriptions` |
| Step 6 Server 注册 API | `unit.capabilities`、`unit.apps` | `integration.apps` |
| Step 7 示例 | — | `integration.legacy`、`integration.dual_era` |
| Step 8 构建与测试接入 | 全量 `unit.*` | 全量 `integration.*` |

#### 特性完成门禁

| 特性 | 必须通过的集成测试 |
|---|---|
| 发现、版本协商与回退 | `integration.discover`、`integration.version_fallback` |
| 版本演进可扩展性（假版本只加 profile） | `integration.version_extensibility` |
| 2024-11-05 回归 | `integration.legacy` |
| 2026-07-28 核心请求（list/call/read/prompt/completion） | `integration.modern_core` |
| MRTR | `integration.mrtr` |
| 订阅 | `integration.subscriptions` |
| 缓存字段 | 包含在 `integration.discover`、`integration.modern_core` 的断言中 |
| MCP Apps | `integration.apps` |
| 双版本共存 | `integration.dual_era` |

### 编码风格一致性（强制）

所有新增/修改代码必须与 `Source/Protocol`、`Example/MCPServer` 现有风格一致。每个编码单元完成时先做风格自查，再运行单元测试；不符合则先修正。

#### 语言与依赖

- 固定 C++14；禁止 `std::optional`、`std::variant`、`std::string_view` 等 C++17 特性。
- 不引入第三方库、格式化工具或新范式；测试代码同样遵守。

#### 命名约定

| 类别 | 约定 | 现有示例 |
|---|---|---|
| 数据模型/struct | 大驼峰，无前缀 | `Message`、`Tool`、`CallToolResult` |
| 行为类 | `C` 前缀 + 大驼峰 | `CMCPTask`、`CStdioTransport`、`CEchoTask` |
| 成员变量 | `m_` + 类型前缀 + 大驼峰 | `m_spRequest`、`m_strMessage`、`m_iCode`、`m_bFinished`、`m_vecAsyncTasksCache` |
| 局部变量 | 类型前缀 + 大驼峰 | `iErrCode`、`strResponse`、`spTask`、`jMsg`、`ullTimestamp` |
| 方法 | 大驼峰；访问器用 `Get/Set` | `SetServerInfo`、`GetTransport`、`DoSerialize` |
| 常量 | 全大写 + 下划线 | `TOOL_NAME`、`ERRNO_OK` |
| 枚举值 | `类型名_值` | `MessageType_Unknown` |
| 命名空间 | `MCP`；示例实现用 `Implementation` | — |

类型前缀沿用现有用法：`b`=bool、`i`=int、`ul`/`ull`=无符号长整型、`str`=string、`sp`=shared_ptr、`up`=unique_ptr、`vec`=vector、`deq`=deque、`hash`=unordered_map、`map`=map、`j`=Json::Value、`e`=枚举、`p`=指针。

#### 格式约定

- 文件头使用 `#pragma once`；保持 UTF-8 编码（现有文件多为带 BOM，新文件与同目录邻居保持一致）。
- Tab 缩进（显示宽度 4）；大括号另起一行（Allman）；构造函数初始化列表每行一项、逗号前置。
- 单条语句的 `if` 可不加大括号，与现有代码一致；不要为“更规范”而改写既有写法。
- 不重排现有 include、不改无关行；**禁止整文件格式化**，diff 只包含必要改动。
- 新头文件参照同目录邻居的 include 顺序与相对路径写法。

#### 实现范式

- 错误码统一用 `int` 返回 + `ERRNO_*`，以 `ERRNO_OK != iErrCode` 判断；不引入异常。
- JSON 统一用 jsoncpp：`Json::Value jX(Json::objectValue)`、`isMember`、`asString`；序列化/反序列化放在 `DoSerialize/DoDeserialize`。
- 任务沿用“注册原型 + `Clone()` + `SetRequest()`”模式；异步任务用现有的 `NotifyResult/NotifyError/NotifyProgress`。
- 智能指针用 `std::shared_ptr` / `std::unique_ptr` + `std::make_shared`；不新增自定义 RAII 包装。
- 新常量放 `PublicDef.h`，新请求处理类放 `Source/Protocol/Task/BasicTask.h/.cpp`；不新建平行目录或重复基建。
- 虚函数标 `override`；不修改成员的 getter 标 `const`。

#### 注释与测试代码

- `Source/Protocol` 内注释用英文，与现有文件语气一致；只在非显而易见处注释。
- 测试代码同样使用 Tab、Allman、`m_` 前缀；文件命名 `TestXxx.cpp`，用例名 `unit.*` / `integration.*`。
- 测试不得为通过而复制实现、放宽断言或改动生产代码的语义。

### Step 0：确认扩展文档（已完成）

- `Doc/modelcontextprotocol/extensions/apps/apps.mdx`（SEP-1865，已入库）

实施时只读这些文件，不修改；`apps.mdx` 是 MCP Apps 的唯一口径。

### Step 0.5：测试基建（必须先于功能编码完成）

目标：建立零第三方依赖的单元测试与集成测试框架，后续每个编码单元/特性都复用它。

新增目录与文件：

- `Tests/TestHarness.h`：单头文件测试框架，提供 `TEST_CASE`、`CHECK`、`CHECK_EQ`、`CHECK_JSON_EQ`；仅依赖标准库与 jsoncpp。
- `Tests/TestMain.cpp`：
  - `--unit`：运行全部单元用例；`--unit <module>`：只运行指定模块（对应门禁表中的 `unit.<module>`）。
  - `--case <name>`：运行单个集成场景；一个进程只跑一个场景，规避 `CMCPSession` 单例状态串扰。
  - 失败返回非 0，并输出用例名、文件、行号、期望值与实际值。
- `Tests/UnitTests/*.cpp`：按模块拆分：`TestConstants.cpp`、`TestVersionRegistry.cpp`、`TestMessage.cpp`、`TestContentBlocks.cpp`、`TestRequest.cpp`、`TestResponse.cpp`、`TestNotification.cpp`、`TestMeta.cpp`、`TestSession.cpp`、`TestTasks.cpp`、`TestMrtr.cpp`、`TestSubscriptions.cpp`、`TestCaching.cpp`、`TestCapabilities.cpp`、`TestApps.cpp`、`TestLegacyCompat.cpp`。
- `Tests/IntegrationTests/MemoryTransport.h/.cpp`：实现 `MCP::CMCPTransport` 的内存传输；`Read()` 从线程安全队列取一行（无数据时阻塞），`Write()` 收集输出；支持主动关闭使 `Run()` 返回。
- `Tests/IntegrationTests/TestServer.h/.cpp`：基于 `MCP::CMCPServer<CTestServer>` 的测试服务，注册工具/资源/prompt、MRTR、订阅与缓存配置；MCP Apps/扩展由场景显式开启（默认不注册），避免 legacy golden 与 discover 用例混入扩展能力。
- `Tests/IntegrationTests/Cases/*.cpp`：每个特性一个场景，对应门禁表中的 `integration.*`。
- `Tests/CMakeLists.txt`：`add_executable(TinyMCPTests ...)`，编译 `Source/Protocol` 源文件与 `Source/External/jsoncpp/src/lib_json/*.cpp`，包含 `Source/External/jsoncpp/include`；因 `cmake -S Tests` 以 `Tests` 为源目录，仓库内路径统一用 `${CMAKE_CURRENT_SOURCE_DIR}/../Source/...`（或先设置 `ROOT_DIR`），并设置 `CMAKE_CXX_STANDARD 14`、`CMAKE_CXX_STANDARD_REQUIRED ON`、链接 `Threads::Threads`；`enable_testing()`；每个集成场景对应一个 `add_test(NAME integration.<name> COMMAND TinyMCPTests --case <name>)`，并设置 `TIMEOUT 30`；每个单元模块对应 `add_test(NAME unit.<module> COMMAND TinyMCPTests --unit <module>)`。**显式排除 `Source/Protocol/Transport/SSETransport.cpp`**（它依赖 vendored cpp-httplib 与 socket 库，测试用 `MemoryTransport`，不需要它）；若使用 glob，需 `list(FILTER ... EXCLUDE REGEX "SSETransport\\.cpp$")`，避免测试目标引入额外链接依赖。
- Windows：新增 `Tests/TinyMCPTests.vcxproj` 并加入现有解决方案（或独立 `Tests.sln`）。

约束：

- 测试代码直接 include `Source/Protocol`，不得复制协议实现。
- 不引入 gtest/catch2 等外部库。
- 单元测试总时长目标 < 5s；单个集成场景 < 30s。
- 集成测试通过内存传输验证协议层；真实 stdio 另设冒烟脚本（见 7.0），不属于日常门禁。

### Step 1：`PublicDef.h` 常量与枚举

文件：`Source/Protocol/Public/PublicDef.h`

新增：

- 版本常量：`PROTOCOL_VER_2024_11_05`、`PROTOCOL_VER_2026_07_28`（`PROTOCOL_VER` 继续表示默认 Handshake 版本，值不变）。
- 版本演进类型（放在 `PublicDef.h/.cpp`，不新建目录）：
  - `enum ProtocolEra { Era_Handshake, Era_Stateless }`
  - `enum ProtocolFeature { Feature_ResultMeta, Feature_EntityMeta, Feature_ImplementationExtras, Feature_ResultType, Feature_Discover, Feature_Caching, Feature_Mrtr, Feature_ListenSubscriptions, Feature_Extensions, Feature_CompletionsCapability, Feature_ToolTitle, Feature_ToolAnnotations, Feature_StructuredContent, Feature_ContentAudio, Feature_ContentResourceLink, Feature_ContentToolUse, Feature_ContentAnnotations, Feature_LogLevelInMeta, Feature_Count }`（请求侧 `_meta` 不设 feature，按 `Era_Stateless` 解析）
  - `struct ProtocolProfile`（`strVersion`、`eEra`、feature 位、`setSupportedMethods`、`iResourceNotFoundCode`）
  - `struct ProtocolContext`（`strVersion`、`pProfile`、`IsValid()`、`IsStateless()`、`Supports(ProtocolFeature)`；默认 2024-11-05 / Handshake）
  - `class CProtocolRegistry`（`Find`、`GetDefaultHandshakeProfile`、`GetLatestStatelessProfile`、`GetSupportedVersions`、接受 profile 列表的构造）
  - 注册表初始只放 `2024-11-05` 与 `2026-07-28` 两个 profile；后续版本只加数据行。默认全局实例由内置 profile 构造，测试可注入自定义实例（假版本 + 一个 Handshake profile）验证演进能力。
- 禁止再新增 `PROTOCOL_VERSIONS_SUPPORTED` 之类的硬编码数组；所有版本列表从 `CProtocolRegistry` 读取。
- 现代 `_meta` 键：
  - `io.modelcontextprotocol/protocolVersion`
  - `io.modelcontextprotocol/clientCapabilities`
  - `io.modelcontextprotocol/clientInfo`
  - `io.modelcontextprotocol/logLevel`
  - `io.modelcontextprotocol/serverInfo`
  - `io.modelcontextprotocol/subscriptionId`
- 方法名：`server/discover`、`subscriptions/listen`、`subscriptions/acknowledged`（完整名 `notifications/subscriptions/acknowledged`）、`notifications/tools/list_changed`（现有缺失，必须补）。
- 结果/参数键：`resultType`、`inputRequests`、`inputResponses`、`requestState`、`ttlMs`、`cacheScope`、`supportedVersions`、`instructions`、`notifications`、`toolsListChanged`、`promptsListChanged`、`resourcesListChanged`、`resourceSubscriptions`、`extensions`、`completions`、`logLevel`、`title`、`icons`、`websiteUrl`、`outputSchema`、`structuredContent`、`annotations`、`audio`、`resource_link`、`tool_use`、`tool_result`、`toolUseId`、`_meta`、`ui`、`csp`、`permissions`、`resourceUri`。
- 常量：`RESULT_TYPE_COMPLETE = "complete"`、`RESULT_TYPE_INPUT_REQUIRED = "input_required"`、`CACHE_SCOPE_PUBLIC/PRIVATE`、`EXTENSION_UI = "io.modelcontextprotocol/ui"`、`MIME_TYPE_MCP_APP = "text/html;profile=mcp-app"`。
- 错误码：**保留** `ERRNO_HEADER_MISMATCH = -32001`（legacy 常量与取值都不变；本项目 stdio 不使用，仅保留给外部引用）；**新增** `ERRNO_HEADER_MISMATCH_MODERN = -32020`、`ERRNO_MISSING_REQUIRED_CLIENT_CAPABILITY = -32021`、`ERRNO_UNSUPPORTED_PROTOCOL_VERSION = -32022`；保留 `ERRNO_RESOURCE_NOT_FOUND = -32002` 供 legacy。
- `MessageType` 追加：`MessageType_DiscoverRequest/Result`、`MessageType_SubscriptionsListenRequest/Result`、`MessageType_SubscriptionsAcknowledgedNotification`、`MessageType_MetaObject`、`MessageType_Icon`、`MessageType_AudioContent`、`MessageType_ResourceLink`、`MessageType_ToolUseContent`、`MessageType_ToolResultContent`。

> 说明：不修改 `ERRNO_HEADER_MISMATCH` 的既有取值，避免破坏外部应用与 legacy 语义；现代头不匹配使用新常量 `ERRNO_HEADER_MISMATCH_MODERN`。

### Step 2：消息模型扩展

文件：`Source/Protocol/Message/BasicMessage.h/.cpp`

新增/修改：

1. `MetaObject`：现代 `_meta` 的强类型模型（`protocolVersion`、`clientInfo`、`clientCapabilities`、`logLevel`）+ 未识别键的 `Json::Value` 透传；`progressToken`/`toolCallId` 继续使用现有成员，不迁入 `MetaObject`。
2. `Implementation`：补 `title`、`description`、`icons`、`websiteUrl`。
3. `Icon`：`src`、`mimeType?`、`sizes?`、`theme?`。
4. `Annotations`：`audience?`、`priority?`、`lastModified?`。
5. `Tool`：补 `title`、`icons`、`outputSchema`、`annotations`、`_meta`；`title`/`icons`/`outputSchema` 由 `Feature_ToolTitle` 门控，`annotations` 由 `Feature_ToolAnnotations` 门控，`_meta` 由 `Feature_EntityMeta` 门控；提供 `SetUiResourceUri()`（写入 `_meta.ui.resourceUri`），不提供 `SetUiVisibility`。
6. `ToolAnnotations`：补 `title?`、`readOnlyHint?`、`destructiveHint?`、`idempotentHint?`、`openWorldHint?`（均为 hint，按 schema 原样序列化）。
7. `Resource` / `ResourceTemplate` / `Prompt`：补 `title`、`icons`、`_meta`（Resource/ResourceTemplate 另补 `annotations`）。
8. `ResourceContents`（`TextResourceContents`/`BlobResourceContents`）：补 `_meta`（`Feature_EntityMeta` 门控）；所有 `ContentBlock`（text/image/audio/resource_link）补 `annotations`（2024-11-05 起即存在，不门控，仅 `lastModified` 由 `Feature_AnnotationLastModified` 门控）与 `_meta`（`Feature_EntityMeta` 门控）。
9. `AudioContent`、`ResourceLink`、`ToolUseContent`、`ToolResultContent`：按 schema 实现序列化/反序列化；`ToolResultContent.structuredContent` 随 `Feature_ContentToolUse` 输出。
10. `ServerCapabilities`：补 `completions`（存在标志）与 `extensions`（`std::map<std::string, Json::Value>`）。现代 discover 与 legacy `initialize` 结果共用同一 `extensions` 容器；legacy 的 `experimental` 原样保留；`extensions` 为空 map 时**不得输出**该键。新增 `void DoSerializeVersionExtras(Json::Value& jObj, const ProtocolContext& ctx) const`：`completions` 仅 `Feature_CompletionsCapability` 为真时输出；`extensions` 在 `Feature_Extensions` 为真或显式启用扩展（legacy 透传标记 `bExtensionPassthrough`）时输出；`DiscoverResult`/`InitializeResult` 在基础序列化后调用该方法。
11. `ClientCapabilities`：新增客户端能力容器（现代 `elicitation.form/url`、`roots`、`sampling`、`extensions`），并支持 legacy `initialize.params.capabilities.extensions` 的读取与 `SupportsExtension(id)` 查询；不需要完整反序列化所有子字段。
12. 资源 UI 元数据：不新增强类型类，直接使用 `_meta` 的 `Json::Value` 透传 `ui.csp` / `ui.permissions`；避免为本地文档未定义的子域建模。
13. `InputRequests`/`InputResponses`：以 `Json::Value`（对象 map）承载，提供 `IsValid()` 与键值访问，避免过度建模。

实现方式：沿用现有 `DoSerialize/DoDeserialize` 风格；所有新字段“有值才写”。`Implementation`/`Tool`/`Resource`/`ResourceTemplate`/`Prompt`/各 `Content` 子类的新增字段一律通过新增的 `DoSerializeVersionExtras(Json::Value&, const ProtocolContext&) const` 输出，现有 `DoSerialize` 继续只输出旧字段，从而保证 legacy 输出不变且不引入共享原型上的版本标志。

`ServerCapabilities` 例外说明：legacy `initialize` 中 `extensions` 仅在显式注册扩展时输出（注册时置 `bExtensionPassthrough`）；`completions` 仅 `Feature_CompletionsCapability` 为真时输出（本期为 `2026-07-28`），不使用手写版本判断。

### Step 3：请求/响应/通知

文件：`Request.h/.cpp`、`Response.h/.cpp`、`Notification.h/.cpp`

请求：

- `Request` 基类新增 `MetaObject meta`；`ctxProtocol` 统一由基类 `Message` 持有，**不在 `Request` 重复声明**；`logLevel` 只存在于 `meta`，不另设成员。
- `Request` 基类统一承载 `MCP::InputResponses inputResponses`、`std::string strRequestState`（按 schema 可出现在任意客户端请求上；非 MRTR 三类只透传、不消费；`CallToolRequest`/`ReadResourceRequest`/`GetPromptRequest` 直接读取基类字段）。
- 新增 `DiscoverRequest`（`params._meta` 必填）。
- 新增 `SubscriptionsListenRequest`（过滤器嵌套在 `params.notifications`：`toolsListChanged`/`promptsListChanged`/`resourcesListChanged`/`resourceSubscriptions`）。
- 所有 `Era_Stateless` 请求的反序列化：读取 `params._meta`，解析 `protocolVersion`、`clientCapabilities`、`clientInfo`；`logLevel` 仅当所在 profile 开启 `Feature_LogLevelInMeta` 时解析。缺失必填项时保留“缺失”事实，交由 Session 返回 `-32602`，不得在此处补默认值。
- `Era_Handshake` 请求继续忽略不存在的新字段。

响应：

- `Response` 基类新增 `strResultType`、`MCP::InputRequests inputRequests`、`strRequestState`、`ttlMs`、`cacheScope`、`Json::Value jResultMeta`、`Json::Value jServerInfo`、`bool bMrtrRetry{false}`；`ctxProtocol` 统一由基类 `Message` 持有，**不在 `Response` 重复声明**。`jResultMeta` 只承载结果 `_meta` 的应用/业务键，`serverInfo` 由版本辅助方法合并写入；`bMrtrRetry` 由 `PrepareResult` 从请求复制，用于抑制缓存字段。
- **不在 `Response::DoSerialize()` 中写结果字段**（会被子类覆盖）。新增 `void WriteVersionResultFields(Json::Value& jResult, const ProtocolContext& ctx, const Json::Value& jServerInfo) const`，按 `ctx.Supports(...)` 写入：
  - `Feature_ResultType` → `resultType`
  - `Feature_Mrtr` 且非空时写 `inputRequests`/`requestState`
  - `Feature_Caching` 且 4.6 列出的 5 类非 discover 结果、`resultType == "complete"`、且 `!bMrtrRetry` 时写 `ttlMs`/`cacheScope`；`server/discover` 由 `Feature_Discover` 必出（`DiscoverResult` 自行写这两个必填字段，不经 `Feature_Caching`）
  - `Feature_ResultMeta` → `_meta["io.modelcontextprotocol/serverInfo"]`（`serverInfo` 由调用方以 `jServerInfo` 传入，`Response` 不反向依赖 `CMCPSession`；与结果已有 `_meta` 合并，非协议保留键不覆盖应用字段，协议保留键以 SDK 值为准）
- 所有现有结果子类（`InitializeResult`、`ListToolsResult`、`CallToolResult`、`ListResourcesResult`、`ReadResourceResult`、`ListResourceTemplatesResult`、`ListPromptsResult`、`GetPromptResult`、`CompleteResult` 等）在 `jMsg[MSG_KEY_RESULT] = jResult` 之前调用该辅助方法；手写旧代码直接构造结果时 `ctxProtocol` 为 legacy，feature 全关，输出不变。`InitializeResult`/`DiscoverResult` 还需对 `capabilities` 调用 `DoSerializeVersionExtras(jResult["capabilities"], ctxProtocol)`，保证 `completions` 与 `extensions` 按 profile/显式扩展正确输出。
- 新增 `DiscoverResult`：`supportedVersions`、`capabilities`、`instructions?` 必填/可选按 schema；`resultType`、`ttlMs`、`cacheScope` 必须输出；`serverInfo` 只在 `_meta` 中；`supportedVersions` 来自 `CProtocolRegistry`。
- 新增 `SubscriptionsListenResult`：仅服务端优雅关闭时发送，`_meta["io.modelcontextprotocol/subscriptionId"]` 必填且保留请求 id 类型。
- `CallToolResult`/`GetPromptResult`/`ReadResourceResult`/四类 `List*Result` 继承上述能力。
- `CallToolResult` 补 `MCP::StructuredContent structuredContent` 与 `_meta`；`structuredContent` 支持任意 JSON 值（object/array/string/number/boolean/null），仅在 `Feature_StructuredContent` 为真且 `HasValue()` 时输出；`_meta` 的应用键有值才写，`serverInfo` 由辅助方法按 `Feature_ResultMeta` 写入，两者不互相替代。

通知：

- 新增 `SubscriptionsAcknowledgedNotification`（`notifications/subscriptions/acknowledged`）：`MCP::RequestId subscriptionId` + `MCP::SubscriptionFilters filters`（授予子集）。
- `Notification` 基类补 `params._meta` 写入辅助 `WriteVersionParamsMeta(Json::Value& jParams, const ProtocolContext& ctx)`：由子类在构建完 `params` 后调用，与已有 `_meta` 合并，非协议保留键不覆盖应用自定义键；feature 关闭时 no-op。
- 现有资源/工具/提示词变更通知保持 Handshake 行为；`Era_Stateless` 发送前由订阅表过滤并加 `subscriptionId`。
- 新增 `notifications/tools/list_changed` 的发送路径（当前常量与发送方法都缺失）。

### Step 4：会话双时代分发

文件：`Session.h/.cpp`

新增成员与函数：

- 版本类型统一引用 4.1 的 `ProtocolEra`/`ProtocolFeature`/`ProtocolProfile`/`ProtocolContext`/`CProtocolRegistry`，**不得在本文件内重新定义 `enum ProtocolEra` 或任何“新/旧”布尔开关**。
- `void SetSupportedProtocolVersions(const std::vector<std::string>&)`：声明本服务支持的协议版本；省略时仅服务默认 Handshake 版本（2024-11-05）。显式声明非注册表版本返回 `ERRNO_INVALID_PARAMS`。
- 现代版本是 opt-in：stdio 入口显式声明两个版本；SSE 入口只声明 2024-11-05，收到现代请求时回 `-32600`。unknown/未声明的现代版本回 `-32022`。
- `ProtocolContext ResolveProtocolContext(const std::shared_ptr<MCP::Message>&) const`：按 4.1 的识别规则，用 `CProtocolRegistry::Find()` 解析版本并返回上下文；不做 `2026-07-28` 专属字符串比较，也不硬编码版本列表。无法识别时返回 `ProtocolContext::Invalid(version)`（`IsValid() == false`），由调用方决定错误码。
- 现代逻辑全部在现有 `CMCPSession` 内实现，不新建 Session/Server/分发器；legacy 的 `ProcessMessage` 调用链保持原样。
- `Clone()`/`SetRequest()` 复制 `ctxProtocol`、`meta`（含 `logLevel`）；`ctxProtocol` 是异步任务保持现代语义的唯一来源。
- `void PrepareResult(const std::shared_ptr<MCP::Request>&, const std::shared_ptr<MCP::Response>&)`：序列化前统一复制 `ctxProtocol`，并按请求 ctx 序列化当前 `ServerInfo`（含 `Feature_ImplementationExtras` 的 `title`/`description`/`icons`/`websiteUrl`）填入 `jServerInfo`，同时复制 `bMrtrRetry`（请求携带 `inputResponses` 或 `requestState`）；所有任务在调用结果 `Serialize()` 前执行，避免逐处重复。
- `void WriteLogNotification(const MCP::LoggingLevel&, const std::string&, const Json::Value&)`：从现有 `SendLogMessage` 中抽出“实际写 `notifications/message`”的步骤，不读取 `m_loggingLevel`；现代请求的日志过滤在 `ProcessRequest::NotifyLogMessage` 内按 `meta.logLevel` 完成后调用它。
- `int ValidateProtocolContext(const ProtocolContext&, const std::shared_ptr<MCP::Request>&)`：校验版本可识别与必填字段；**不补默认值**，缺失 `protocolVersion`/`clientCapabilities` 返回 `-32602`，未知版本返回 `-32022`。
- `int ProcessModernRequest(const ProtocolContext&, const std::shared_ptr<MCP::Request>&)`：`ProtocolContext` 仅在函数入口解析一次，向下透传给结果序列化，不在分支内部再次读版本字符串。
- `int ProcessModernNotification(const ProtocolContext&, const std::shared_ptr<MCP::Notification>&)`。
- `int EnsureAsyncTaskThread()`：现代请求首次需要异步任务时启动 worker（legacy 仍由 `notifications/initialized` 启动）。
- 订阅表：`std::unordered_map<std::string, SubscriptionState>`（key 由 `MakeRequestIdKey(RequestId)` 生成，保留 string/integer 类型）+ `std::mutex`。
- `int SendSubscriptionNotification(const MCP::ResourceUpdatedNotification&, const std::string& strMatchUri = std::string())`：`resources/updated` 按 `strMatchUri` 精确匹配并附加 `subscriptionId`；list_changed 继续走 `SendToolListChanged`/`SendPromptListChanged`/`SendResourceListChanged`（内部按订阅过滤并附加 `subscriptionId`）。
- 列表变更发送：`int SendToolListChanged()`、`int SendPromptListChanged()`、`int SendResourceListChanged()`（legacy 走现有行为，现代按订阅表过滤并附加 `subscriptionId`）。
- 清理：`ResetForNewClient()`、`Terminate()` 增加清空订阅表、现代请求缓存、logLevel；`Terminate` 关闭前对存活订阅发送优雅关闭结果。

`Run()`/`ParseMessage()` 流程：

1. `ParseRequest` 增加 `server/discover`、`subscriptions/listen` 的显式 method→类型映射；`subscriptions/acknowledged` 为服务端发送方向，不需要入站映射。
2. 反序列化得到 `Request`/`Notification`。
   - `ParseRequest` 对未知 method：若通用 `Request` 携带现代 `_meta`，返回该通用对象（modern 分支回 `-32601`）；否则维持现有 `ERRNO_INTERNAL_ERROR` 静默行为，避免 legacy 行为变化。
   - `server/discover` 在无现代 `_meta` 时按通用 `Request` 返回（legacy 分支据此回 `-32601`）；有现代 `_meta` 时按 `DiscoverRequest` 解析。
3. 解析 `ProtocolContext` 并设置到消息对象（`ctxProtocol`）；解析结果只依赖注册表 profile，不依赖版本号分支。`method == "initialize"` 先行判定为 `Era_Handshake`（即使带现代 `_meta`），取会话注册表的 `GetDefaultHandshakeProfile()`，使新增更老/更新 Handshake 版本时 `initialize` 结果自动使用注册表版本；其余 Handshake 请求同样取该 profile。
4. `Era_Stateless`：
   - 会话注册表不含该版本 → 写 `ErrorResponse(-32022)`（带 `supported`/`requested`）；传输不支持无状态（如 SSE）→ 写 `ErrorResponse(-32600)`。
   - 版本校验失败 → 直接写 `ErrorResponse(-32022)`，带上 `id`；`data.supported` 来自 `CProtocolRegistry::GetSupportedVersions()`。
   - 必填 `_meta` 缺失 → 写 `ErrorResponse(-32602)`，带上 `id`，不补默认值。
   - 请求 → `ProcessModernRequest`；通知 → `ProcessModernNotification`。
5. `Era_Handshake`：走现有 `ProcessMessage` 逻辑，不做任何行为变更；唯一新增例外是 `server/discover` 直接回 `-32601`（用于版本回退探测），其余未知 method 保持原静默行为。
6. **通知不强制按 `_meta` 分流**：`notifications/cancelled` 不携带现代 `_meta` 时仍走现有取消逻辑（若 `requestId` 命中订阅则先移除订阅）；其余无 `_meta` 通知维持 legacy 行为。通知的“现代”判定不得依赖客户端额外补 `_meta`。
7. 现代请求**不写入** `m_hashMessage`（或仅在明确上限内保留），避免长期运行无限增长；legacy 维持现状。
8. **传输边界**：2026-07-28 本期只声明 stdio。stdio 入口显式声明 `2024-11-05` 与 `2026-07-28`；SSE 入口只声明 `2024-11-05`；因此 SSE 连接上收到携带现代 `_meta` 的请求会回 `-32600`，不得静默按现代语义执行；legacy 请求在 SSE 上完全保持现状。

`ProcessModernRequest` 分发：按当前 `ProtocolContext` 的 profile 判定，**禁止写 `if (strVersion == "...")`**。

| method | 处理 |
|---|---|
| `server/discover` | 新增 `ProcessDiscoverRequest`，输出 `resultType`/`ttlMs`/`cacheScope`/`_meta.serverInfo`；仅 `Feature_Discover` 为真的 profile 可达（其余返回 `-32601`） |
| `tools/list`、`resources/list`、`resources/templates/list`、`prompts/list` | 复用现有 list 任务，`Feature_Caching`/`Feature_ResultType` 决定是否补充缓存字段/resultType |
| `tools/call` | 复用 `ProcessCallToolRequest`，`Feature_Mrtr`/`Feature_StructuredContent`/`Feature_ResultType` 决定扩展；不写缓存字段 |
| `resources/read` | 复用 `ProcessReadResourceRequest`，补充缓存/`_meta`/MRTR；错误码取 profile 的 `iResourceNotFoundCode` |
| `prompts/get` | 复用 `ProcessGetPromptRequest`，补充 `_meta`/MRTR/resultType；不写缓存字段 |
| `completion/complete` | 复用 `ProcessCompleteRequest`，补 resultType；能力名取 profile |
| `subscriptions/listen` | 新增 `CMCPSession::ProcessSubscriptionsListenRequest`（**同步处理，不进入异步队列**）：登记 → ack（含 `subscriptionId` 与授予子集）→ 激活，不立即回结果；仅 `Feature_ListenSubscriptions` 为真时可达 |
| `initialize` | 始终按 Handshake 协商（即使携带现代 `_meta`），返回注册表的默认 Handshake 版本；不进入 Stateless 分支 |
| `ping`、`logging/setLevel`、`resources/subscribe`、`resources/unsubscribe` | 当前 profile `setSupportedMethods` 未登记，返回 `-32601` |
| 其他 | `-32601`（带原 `id`） |

- 方法可用性以 `ProtocolProfile::setSupportedMethods` 为准；本期只把方法登记进 `2026-07-28` profile，未来版本增删方法只改 profile 数据，不改本分发表结构。

现代通知：

- `notifications/cancelled` → 复用取消逻辑；若 `requestId` 对应 listen 请求，先移除订阅再取消。
- 其他通知 → 忽略（现代没有对应通知入口）。

### Step 5：任务层

文件：`Task.h`、`BasicTask.h/.cpp`

新增任务类：

- `ProcessDiscoverRequest`：组装 `DiscoverResult`。
  - `capabilities` **复用现有 `ServerCapabilities` 组装逻辑**，禁止另写一套能力模型；**复制**当前能力后按 profile 裁剪，不得修改 Session 共享的 legacy 能力对象：
    - 不输出 `experimental`（仅 legacy 使用 `experimental.completion`）；
    - 不输出 `resources.subscribe`（现代用 `subscriptions/listen` 取代；legacy `initialize` 保持原值）；
    - `completions` 由 `Feature_CompletionsCapability` 且已注册补全能力时输出；`extensions` 由 `Feature_Extensions` 且显式注册时输出；
    - 启用日志时保留 `logging` 能力（规范要求发送日志必须声明），但 `logging/setLevel` 方法不登记。
  - `serverInfo` 复用现有 `ServerInfo`/`Implementation`（含 `title`/`description`/`icons`/`websiteUrl`），只写入 `result._meta`；结果序列化前调用 `PrepareResult`，保证 `ctxProtocol`/`jServerInfo` 就位。
  - `capabilities` 即使为空也输出 `{}`（schema 必填），不得省略 `capabilities` 键。
  - `supportedVersions` 来自 `CProtocolRegistry`，`instructions` 来自 `RegisterServerInstructions`。
- `ProcessSubscriptionsListenRequest`：实现为 `CMCPSession` 的同步方法（不是异步 Task）；需与读线程解耦的清理逻辑仍走现有 Session 方法，避免把长连接订阅混入任务队列。
- `ProcessInputRequired` 辅助：`ProcessCallToolRequest`/`ProcessReadResourceRequest`/`ProcessGetPromptRequest` 增加 `NotifyInputRequired`。

现有任务修改：

0. `ProcessInitializeRequest`：`protocolVersion` 改为取 `ctxProtocol.strVersion`（由 Session 按注册表 Handshake profile 设置），不再直接引用 `PROTOCOL_VER`；legacy 默认仍为 `2024-11-05`，golden 不变。`capabilities` 的新字段由 `InitializeResult::DoSerialize` 调用 `DoSerializeVersionExtras` 输出（见 Step 3），任务不重复调用；legacy 仅显式扩展时输出 `extensions`，不输出 `completions`。
1. `ProcessListToolsRequest` / `ProcessListResourcesRequest` / `ProcessListResourceTemplatesRequest` / `ProcessListPromptsRequest`：
   - 结果补 `ttlMs`、`cacheScope`、`resultType`：是否输出由 `ctxProtocol.Supports(Feature_Caching)`/`Supports(Feature_ResultType)` 决定（现代，且仅这四类 list 可缓存）；序列化前调用 `PrepareResult`。
   - 现代序列化时对每个 `Tool`/`Resource`/`ResourceTemplate`/`Prompt` 调用 `DoSerializeVersionExtras(jObj, ctxProtocol)`；legacy 不调用，输出与现状一致。
   - 修正非法 cursor 的 `ErrorResponse`（仅现代路径）：设置 `requestId` 与 `ctxProtocol`，返回 `-32602`；legacy 路径保持现状。
   - 保持现有“一页一条”的 cursor 语义（cursor 对客户端不透明）；同一列表所有分页使用同一 `cacheScope`。
2. `ProcessCallToolRequest`：
   - 结果补 `resultType`、`_meta`、`structuredContent`：分别由 `Feature_ResultType`、`Feature_ResultMeta`、`Feature_StructuredContent` 门控；**不写** `ttlMs`/`cacheScope`；序列化前调用 `PrepareResult`。
   - 传递 `inputResponses`、`strRequestState` 给应用任务（`InputResponses` 数据模型）。
   - 提供 `NotifyInputRequired`（校验至少一项存在、校验客户端能力）与 `NotifyMissingClientCapability`（`-32021`），两者仅为 `Feature_Mrtr` 为真的上下文生效。
   - 未知工具：现代 `-32602`，legacy 保持原行为。
3. `ProcessReadResourceRequest`：
   - 资源不存在错误码取 `ctxProtocol.pProfile->iResourceNotFoundCode`（现代 `-32602`，legacy `-32002`），不在任务内写死版本判断。
   - 结果补 `ttlMs`、`cacheScope`、`resultType`、`_meta`；缓存字段仅 `Feature_Caching` 为真、`complete` 且 `!bMrtrRetry` 时写；序列化前调用 `PrepareResult`。
   - 内容支持 `_meta` 与 `text/html;profile=mcp-app`。
   - 支持 `inputResponses/requestState` 与 `NotifyInputRequired`（`Feature_Mrtr` 门控）。
4. `ProcessGetPromptRequest`：
   - 结果补 `resultType`、`_meta`；提示词不存在在现代返回 `-32602`，legacy 保持原行为；序列化前调用 `PrepareResult`。
   - 不写缓存字段；支持 MRTR。
5. `ProcessCompleteRequest`：结果补 `resultType`；现代能力名 `completions`；不写缓存字段；序列化前调用 `PrepareResult`。
6. `ProcessErrorRequest`：确保所有错误响应带 `requestId`；JSON-RPC error 响应不带 `result`/`resultType`；`-32021` 的 `error.data.requiredCapabilities` 必填。
7. `ProcessPingRequest`、`ProcessSetLevelRequest`、`ProcessSubscribeResourceRequest`、`ProcessUnsubscribeResourceRequest`：仅 legacy 可达，保持原实现。
8. 订阅相关：list_changed 通知走 `SendToolListChanged`/`SendPromptListChanged`/`SendResourceListChanged`；资源更新走类型化 `SendSubscriptionNotification(const MCP::ResourceUpdatedNotification&, uri)`（精确匹配、附加 `subscriptionId`）。现有 `Send*Changed` 需在构建 `params` 后调用 `WriteVersionParamsMeta`，并由订阅表为每条匹配订阅设置 `MCP::RequestId subscriptionId`；legacy 不设置，输出保持原样。
9. 日志：`ProcessRequest::NotifyLogMessage` 在 `Feature_LogLevelInMeta` 为真时按 `meta.logLevel` 过滤（缺失即丢弃），过滤通过后调用 `WriteLogNotification`；legacy 仍调用 `SendLogMessage` 的会话级逻辑。

### Step 6：Server 注册 API

文件：`Source/Protocol/Entity/Server.h`

新增：

- `RegisterServerCompletionsCapability()`：现代 `completions`（legacy 仍用 `RegisterServerExperimentalCapabilities`）。现代 discover 只输出 `completions`、不输出 `experimental`；legacy `initialize` 输出保持原样，新 API 不改变老代码的 legacy 能力声明。该能力字段由 `Feature_CompletionsCapability` 门控。
- `RegisterServerExtensionsCapabilities(const std::map<std::string, Json::Value>&)`：通用扩展注册；现代 discover 与 legacy initialize 共用。注册非空扩展时置 `ServerCapabilities::bExtensionPassthrough = true`，供 legacy `initialize` 透传；注册 API 本身不写 `_meta`。
- `RegisterServerAppsCapabilities()`：显式启用 MCP Apps；向 `capabilities.extensions` 写入 `"io.modelcontextprotocol/ui": {}`，**仅在 2026-07-28 的 `server/discover` 中输出**；legacy `initialize` 不得输出（2024-11-05 不支持 MCP Apps）。需与 `RegisterServerExtensionsCapabilities()` 的通用透传区分：Apps 属于仅现代扩展，legacy 序列化时必须过滤。默认不启用。
- `RegisterServerInstructions(const std::string&)`：`server/discover` 的 `instructions`。
- `SetServerCacheOptions(int ttlMs, const std::string& cacheScope)`：仅影响 4.6 的 6 类可缓存结果。
- `RegisterServerUiResources(...)`：可选的便捷方法，内部复用 `RegisterServerResources`；不新造注册体系。UI 资源的 `_meta.ui` 以 `Json::Value` 原样传入。
- list_changed 能力沿用现有 `Tools/Resources/Prompts` 的 `bListChanged`；不新增重复能力字段。发送 API 在 Session（见 Step 4）。
- 注册 API 只登记“服务端具备什么”，不登记“属于哪个版本”。能力是否出现在某版本响应中由该版本 profile 的 feature/能力映射决定；新增协议版本时不得修改这些注册方法签名。

说明：所有新 API 只把数据交给 `CMCPSession` 保存，实际序列化在响应层完成；应用自定义 `_meta` 的非保留键不得被 SDK 覆盖，协议保留键（`serverInfo`、`subscriptionId`）由 SDK 写入。

### Step 7：示例

文件：`Example/MCPServer/Source/EchoServer.*`、`EchoTask.*`、`MCPServer.cpp`

- 保持现有 2024-11-05 工具/资源/prompt 不变。
- 新增现代示例：
  - `echo_confirm` 工具：首次调用返回 `inputRequests`（`elicitation/create`），带 `requestState`；重试带 `inputResponses` 后返回 `complete`。示例用一个“不透明字符串 + 简单校验位”演示对 `requestState` 的完整性校验（不实现加密），并在注释中写明“生产环境必须使用 AEAD/HMAC 保护”；不得为此引入第三方加密库。
  - `ui://echo/app.html` 资源：返回 `text/html;profile=mcp-app`，通过 `UiResourceMeta`/`SetUiMeta()` 声明 `csp`（四子域）与 `permissions`（四键），`resources/read` 用 `BuildUiResourceContents()` 保持一致。
  - `echo_app` 工具：`_meta.ui.resourceUri = "ui://echo/app.html"`，`visibility = ["model","app"]`，同时返回文本 `content`（优雅降级）。
  - 注册 `completions`，并通过 `RegisterServerAppsCapabilities()` 显式启用 `io.modelcontextprotocol/ui`（默认关闭）。
  - 现代订阅示例：listen 请求用数字 `id`，过滤器写在 `params.notifications`；服务端先回 ack（带数字 `subscriptionId`），再由定时器构造 `MCP::ResourceUpdatedNotification` 调用类型化 `SendSubscriptionNotification(updated, "echo://...")` 推送；未订阅时不推送。
- `MCPServer.cpp` 的参数解析保持 `--transport stdio|sse`；不新增传输。`--transport stdio` 分支显式声明 `{2024-11-05, 2026-07-28}`，`sse` 分支只声明 `{2024-11-05}`。

### Step 8：构建文件

- `Example/MCPServer/Build/Windows/MCPServer.vcxproj` 与 `.filters`：把新增 `.h/.cpp` 加入编译与筛选器。
- `Example/MCPServer/Build/Linux/CMakeLists.txt`、`Example/MCPServer/Build/MacOS/CMakeLists.txt` 使用 glob，无需修改；实施时确认新文件位于 glob 路径内。
- 新增 `Tests/TinyMCPTests.vcxproj` 并加入 `Example/MCPServer/Build/Windows/MCPServer.sln`；CMake 构建通过 `Tests/CMakeLists.txt` 注册。
- `Tests/CMakeLists.txt` 必须：
  - `add_executable(TinyMCPTests ...)` 同时编译 `Source/Protocol` 源文件、`Source/External/jsoncpp/src/lib_json/*.cpp`；因 `cmake -S Tests` 以 `Tests` 为源目录，所有仓库内路径必须写成 `${CMAKE_CURRENT_SOURCE_DIR}/../Source/...`（或先统一设置 `ROOT_DIR`），禁止写相对 `Source/...`；同时设置 `CMAKE_CXX_STANDARD 14`、`CMAKE_CXX_STANDARD_REQUIRED ON`，链接 `Threads::Threads`。
  - 排除 `Source/Protocol/Transport/SSETransport.cpp`（见 Step 0.5），避免测试目标依赖 cpp-httplib/socket 库。
  - `enable_testing()`；每个集成场景一个 `add_test(NAME integration.<name> COMMAND TinyMCPTests --case <name>)`，设置 `TIMEOUT 30`。
  - 单元测试注册为 `unit.all`（`--unit` 全部）；同时为每个单元模块注册 `add_test(NAME unit.<module> COMMAND TinyMCPTests --unit <module>)`，便于按门禁表执行单个单元。
- 提供统一入口：`ctest --test-dir <build> --output-on-failure` 可一次运行全部 `unit.*` 与 `integration.*`。
- 保持 C++14；禁止使用 `std::optional`、`std::variant`、`string_view` 等 C++17 特性。

### Step 9：文档

- `Readme.md` 增加“协议版本支持”一节：
  - 2026-07-28 仅 stdio；2024-11-05 支持 stdio + HTTP+SSE。
  - **启用方式**：现代协议默认关闭，库使用者必须在 stdio 入口通过 `SetSupportedProtocolVersions` 显式声明 `2026-07-28`；SSE 入口只声明 `2024-11-05`（现代请求 `-32600`）。示例已按此配置。
  - 现代请求 `_meta` 示例、`server/discover` 示例、MRTR 示例、订阅示例。
  - 可缓存结果清单（6 类）与“MRTR 重试结果不缓存”。
  - 必填 `_meta` 缺失返回 `-32602`、不补默认值；`-32001` 与 `-32020` 的区别。
  - 列出 SDK 支持的扩展（当前仅 MCP Apps），说明默认关闭、显式启用与优雅降级行为。
  - 明确 MCP Apps 仅随 2026-07-28 提供；2024-11-05 不支持 MCP Apps，不声明扩展、不输出 UI 元数据。
  - 明确 MCP Apps 为服务端能力子集，不含 View↔Host `postMessage` 协议。
  - 说明 `logging`、`roots`、`sampling` 在 2026-07-28 已 deprecated：本期仍按规范支持（logging 请求级 `logLevel`、roots/sampling 走 MRTR），但建议新实现使用 stderr/OpenTelemetry、工具参数/资源 URI、直接调用 LLM API；SSE 连接不支持现代请求。
- 不修改已入库的规范/schema 原文。

## 6. 假设与决策

| 编号 | 决策 | 理由 |
|---|---|---|
| D1 | 2026-07-28 本期只在 stdio 上提供 | 规范允许服务器选择 transport binding；这不是完整的 transport 覆盖，README 必须明确说明。Streamable HTTP 若需要完整支持，另立计划 |
| D2 | 旧 HTTP+SSE 完全保留 | 保证 2024-11-05 功能不回退 |
| D3 | 现代请求逐请求识别，`initialize` 选 legacy | 符合 versioning.md 的 dual-era 允许并发服务 |
| D4 | 新字段只在对应 `ProtocolFeature` 开启时序列化，默认 profile 全部关闭 | 保证 legacy 输出兼容；字段与版本号解耦，未来版本只改 profile 的 feature 位 |
| D5 | `ttlMs=0`、`cacheScope="private"` 为默认，且只用于 6 类可缓存结果 | 规范要求可缓存结果必填；0 表示立即过期，安全 |
| D6 | MRTR 仅三类请求 | 规范明确限制 |
| D7 | Apps 只做服务端资源/工具元数据/能力声明 | View↔Host 协议不属于 Server SDK |
| D8 | 不引入依赖 | 约束 |
| D9 | 继续用 numeric cursor | 现有实现可用且 cursor 对客户端不透明；不改分页协议 |
| D10 | 现代不支持 `ping`/`logging/setLevel`/`resources/subscribe` | changelog 明确移除/取代 |
| D11 | 扩展默认关闭，必须显式 `RegisterServerAppsCapabilities()` | 遵循 `extensions/overview.md` 的 opt-in 规则 |
| D12 | legacy `initialize` 透传 `capabilities.extensions` | 旧版 schema 无该字段，但扩展规范鼓励透传；MCP Apps 例外，按 D29 处理 |
| D13 | 测试框架使用仓库内单头文件实现 + CTest | 满足“零外部依赖”，与现有轻量定位一致 |
| D14 | 集成测试使用内存传输，一进程一场景 | 规避 `CMCPSession` 单例状态串扰；真实 stdio 用冒烟脚本补充 |
| D15 | 单元/集成门禁为强制，失败即停 | 保证每个单元与特性在进入下一步前可验证 |
| D16 | 编码风格沿用现有仓库约定，不引入格式化工具 | 避免风格漂移与无关 diff，保持轻量 |
| D17 | 必填 `_meta` 缺失返回 `-32602`，不补默认值 | 规范明确“malformed request MUST reject with -32602”，补默认会掩盖客户端错误 |
| D18 | `subscriptionId` 保留 `RequestId` 的 string/integer 类型 | schema 规定其值就是 `RequestId`；统一转字符串会导致客户端无法关联 |
| D19 | 缓存只覆盖 6 类结果，MRTR 结果与重试结果不缓存 | 规范明确可缓存清单与 MRTR 不缓存规则 |
| D20 | 保留 `ERRNO_HEADER_MISMATCH = -32001`，新增 `ERRNO_HEADER_MISMATCH_MODERN = -32020` | 不破坏外部引用与 legacy 语义；现代错误用新常量 |
| D21 | Apps 通过 setter 建模 `_meta.ui.resourceUri`/`visibility` 与 `UiResourceMeta`；未知字段只透传，不解释、不校验 | 避免魔法键与多处手写不一致；字段集以 `apps.mdx` 为准 |
| D22 | `resources/updated` 只做精确 URI 匹配 | 原型规范未定义前缀匹配；子资源语义由应用显式传入匹配 URI 处理 |
| D23 | 现代字段一律由 `ctxProtocol.Supports(Feature_X)` 门控，`ctxProtocol` 默认 2024-11-05/Handshake | 旧代码直接构造对象序列化时保持 legacy 输出；7.2 golden 逐字节验证；字段由 feature 位而非版本号控制，未来版本只改 profile |
| D24 | Legacy 仅新增 `server/discover` 探测响应；其他既有错误/静默行为不变 | 老代码不会发送新 method，因此不改变任何既有功能；同时满足版本回退探测需要 |
| D25 | 不新建平行现代目录/类族/Session/任务框架 | 新类型继承现有基类，逻辑作为 `CMCPSession` 分支；满足“复用优先”约束 |
| D26 | 版本差异全部收敛到 `CProtocolRegistry` 的 profile 数据（时代、feature 位、方法集、错误码映射） | 新增更老/更新版本只加 profile，不新增版本专用 `if`；`supportedVersions`/`-32022.data.supported` 均由注册表生成 |
| D27 | 禁止在分发、序列化、任务代码中出现版本号字符串比较或“新/旧”布尔开关 | 版本号比较会把每个新版本变成主干改动；用 `Supports()` 与 profile 数据表达差异，保证演进可扩展 |
| D28 | 新增协议版本的固定操作：加 profile → 按需加 feature 枚举值 → 补 `unit.version_registry` 与 `integration.version_extensibility` | 把“如何扩展”固化为可执行清单，避免后续实施者另起炉灶 |
| D29 | MCP Apps 仅随 2026-07-28 提供，2024-11-05 明确不支持 | 统一口径：legacy 不声明 Apps 扩展、不输出 `_meta.ui` 与 mcp-app 内容，避免“声明了却交付不了”的误导；Apps 视为仅现代扩展，legacy 序列化时过滤 |

## 7. 验证计划

> 以自动化测试为主：单元测试覆盖消息/模型/任务，集成测试覆盖完整协议链路；手工 JSON-RPC 仅用于探索性排查。测试框架为仓库内单头文件实现，不引入第三方依赖。

### 7.0 自动化测试入口（强制）

构建与运行：

```powershell
cmake -S Tests -B build-tests -DCMAKE_BUILD_TYPE=Debug
cmake --build build-tests --config Debug
ctest --test-dir build-tests --output-on-failure
```

- `TinyMCPTests --unit`：运行全部单元测试（CTest 名 `unit.all`）。
- `TinyMCPTests --unit <module>`：运行单个单元模块（CTest 名 `unit.<module>`）。
- `TinyMCPTests --case <name>`：运行单个集成场景（CTest 名 `integration.<name>`）。
- **Legacy golden 基线（Step 1 编码前完成）**：在 Step 0.5 只用现有 API 建立 `CTestServer` 的 legacy 子集（工具/资源/prompt/legacy 补全），用它跑一遍 7.2 的 legacy 请求序列，把成功响应 JSON 存为 `Tests/IntegrationTests/Golden/*.json` 并入库；后续 Step 1–8 可以在 `CTestServer` 追加现代工具/资源，但**不得修改 legacy 子集的注册方式与 golden 文件**，`integration.legacy` 只读取比对。没有基线就无法证明“仅重新编译、功能不变”。
- **测试宿主能力配置**：`CTestServer` 的 legacy 子集只调用现有 API（含 `RegisterServerExperimentalCapabilities` 的 `experimental.completion`）；Step 1 之后追加 `RegisterServerCompletionsCapability()`（现代 `completions`）以及工具/资源/prompt/MRTR/订阅/缓存配置，但不得改动 legacy 子集。MCP Apps/扩展默认**不注册**。`integration.legacy`、`integration.discover`、`integration.modern_core` 使用“未注册扩展”的配置，`integration.apps` 使用“启用 Apps”的配置；一进程一场景，避免能力串扰。
- 门禁：每个编码单元完成后先运行对应 `unit.*`；每个特性完成后运行对应 `integration.*`；全部完成后运行全量 CTest。
- 失败处理：任何用例失败即停止后续开发，修复后从失败用例开始重跑，再跑全量回归。
- 真实 stdio 冒烟（可选，不属于日常门禁）：`Tests/scripts/stdio_smoke.ps1` 启动 `MCPServer.exe`，依次写入 legacy 与 modern 请求并断言关键字段；SSE 冒烟同属脚本，不注册为 CTest。
- 测试宿主默认模拟 stdio：`CTestServer`/`MemoryTransport` 场景显式声明两个版本；仅“非 stdio 拒绝现代请求”用例使用不支持无状态的 MemoryTransport 子类。

### 7.1 构建

- Windows：MSBuild 构建 `Example/MCPServer/Build/Windows/MCPServer.sln`，Release x64；同时构建 `Tests/TinyMCPTests.vcxproj`。
- Linux/macOS（如可用）：CMake 构建；`cmake -S Tests -B build-tests` 生成测试目标。
- 要求：0 error；新增文件全部参与编译；测试目标可独立构建并通过 `ctest` 运行。

### 7.2 Legacy 回归（2024-11-05，`integration.legacy`）

1. stdio 全链路：`initialize` → `notifications/initialized` → `tools/list` → `tools/call`（echo）→ `resources/list` → `resources/read` → `resources/templates/list` → `resources/subscribe`/`unsubscribe` → `prompts/list` → `prompts/get` → `completion/complete` → `logging/setLevel` → `ping`。
   - 断言 `protocolVersion == "2024-11-05"`。
   - 断言结果中**没有** `resultType`、`ttlMs`、`cacheScope`、`extensions`（未注册扩展时）、`_meta.serverInfo`、`structuredContent`、`title`/`icons`/`outputSchema`/`annotations`、`audio`/`resource_link`/`tool_use`/`tool_result`。
   - 对所有成功响应做 golden JSON 对比（以升级前输出为基准），字段集合、字段顺序、值均不得变化。
2. `params._meta` 兼容：
   - 带 `progressToken` 的 legacy 请求输出仅 `_meta.progressToken`，与升级前一致。
   - 不带 `progressToken` 的 legacy 请求不得出现 `_meta` 空对象。
   - `params._meta.toolCallId` 仍能被 `CallToolRequest` 解析（通过现有示例或单元测试验证）。
3. 直接构造对象：不经过 `ParseMessage`，直接构造 `Request`/`Response`/`Tool`/`Resource`/`Prompt` 并序列化，`ctxProtocol` 为默认的 `2024-11-05 / Era_Handshake`（所有 feature 关闭），输出与升级前 golden 一致。
4. SSE（可选冒烟脚本，不注册为 CTest）：`initialize` + `notifications/initialized` + `resources/subscribe` + 资源更新通知。
   - 断言通知无 `subscriptionId`。
   - 本期不修改 SSE 实现；SSE 冒烟需要 `SSETransport.cpp` 与 cpp-httplib，而 `TinyMCPTests` 明确排除该源文件，因此只能由 `Tests/scripts/sse_smoke.ps1` 启动示例进程完成；若测试环境无法绑定本地端口，则降级为人工回归项并记录。
5. 版本探测：legacy 模式下发送 `server/discover`（无现代 `_meta`）→ `-32601`；这是唯一新增的 legacy 响应，老代码不会触发。
6. legacy 非法 cursor、未知 method 等既有错误/静默行为保持现状，不纳入本期修复；legacy 资源不存在仍返回 `-32002`；`-32001` 常量值不变。

### 7.3 发现、版本协商与现代核心（`integration.discover` / `integration.version_fallback` / `integration.modern_core`）

对每个请求构造 `params._meta`：

```json
{
  "io.modelcontextprotocol/protocolVersion": "2026-07-28",
  "io.modelcontextprotocol/clientCapabilities": {}
}
```

**`integration.discover`**

1. `server/discover`：
   - `supportedVersions` 由 `CProtocolRegistry::GetSupportedVersions()` 生成，本期恰为 `2026-07-28` 与 `2024-11-05`（顺序由注册表声明顺序决定）。
   - 有 `resultType:"complete"`、`ttlMs`、`cacheScope`；`serverInfo` 位于 `result._meta`。
   - 注册补全能力时 `capabilities.completions` 输出；启用日志时 `capabilities.logging` 保留；不出现 `experimental` 与 `resources.subscribe`。
   - 断言代码不得写死版本字符串比较，而是与注册表返回值比对。

**`integration.version_fallback`**

2. 版本不支持：`protocolVersion = "2025-06-18"` → `-32022`，`data.supported` 与 `CProtocolRegistry::GetSupportedVersions()` 完全一致、`data.requested` 为原值。
3. 缺 `protocolVersion`/`clientCapabilities` → `-32602`；先用 legacy `initialize` 完成握手后，再发只带 `progressToken` 的 legacy 请求仍按 2024-11-05 处理。
4. 现代客户端对 legacy 服务端的回退路径（模拟）：`CTestServer` 提供“legacy-only”配置（只声明 `2024-11-05` 且不注册现代方法），携带现代 `_meta` 的 `server/discover` 返回 `-32022`（版本未声明）；客户端改发 `initialize` 后可继续工作。断言回退判定不依赖具体错误码。

**`integration.modern_core`**

5. `tools/list`、`resources/list`、`resources/templates/list`、`prompts/list`：
   - 均有 `resultType`、`ttlMs`、`cacheScope`，顺序稳定。
6. `tools/call`：`resultType:"complete"`，`content` 支持 text/image/audio/resource_link/tool_use/tool_result；**不得**出现 `ttlMs`/`cacheScope`。
7. `resources/read`：含 `ttlMs`、`cacheScope`；不存在资源 → `-32602`（不得 `-32002`）。
8. `completion/complete`：`resultType` 正确，且无缓存字段。
9. 移除项：`ping`、`logging/setLevel`、`resources/subscribe` → `-32601`；若启用日志，`server/discover` 仍声明 `logging` 能力（与移除方法不矛盾）。
10. 未知 method → `-32601` 且带原 `id`。
11. 缺必填 `_meta`（无 `protocolVersion` 或 `clientCapabilities`）→ `-32602`，且不得补默认值；未初始化但完全没有现代 `_meta` 键时按 legacy 语义处理。
12. 带现代 `_meta` 的 `initialize` 仍按 Handshake 协商，返回注册表默认 Handshake 版本（不返回 `-32600`），证明双时代回退兼容。

**`integration.version_extensibility`**（验证“未来版本只加 profile”）

13. 测试内置一个假版本 profile（如 `2099-01-01`，`Era_Stateless`，开启 `Feature_ResultType`、`Feature_ResultMeta`、`Feature_Discover`、`Feature_Caching`，方法集只含 `server/discover` 与 `tools/list`），通过内部 `ProtocolRegistryAccess` 注入自定义 `CProtocolRegistry` 实例（同一实例保留 `2024-11-05` Handshake profile，保证默认 `ProtocolContext` 与 legacy 输出仍有效）：
    - `server/discover` 的 `supportedVersions` 自动包含该假版本；`-32022.data.supported` 同步包含。
    - 用假版本发 `tools/list` → 成功，结果含 `resultType`、`ttlMs`、`cacheScope`（与 schema 对可缓存结果的要求一致）。
    - 用假版本发 `tools/call` → `-32601`（未在方法集登记）。
14. 另建一个仅验证门控的合成 profile（只开 `Feature_ResultType` 或只开 `Feature_Caching`），单独验证 `resultType`/缓存字段由 feature 位控制，而非版本号分支；该合成 profile 不声称符合任何正式 schema，只用于单测门控。具体做法：在仅验证 `tools/list` 的独立假 profile 中关闭 `Feature_ResultType` 后，`tools/list` 不再输出 `resultType`，其余输出不变（该用例不调用 `server/discover`，避免构造不符合 schema 的假结果）。
15. 单独注入另一个注册表，加入 `2098-01-01` 作为 `Era_Handshake` profile（声明在 `2024-11-05` 之前，成为该注册表最高 Handshake 版本；与第 13 条的 `2099-01-01` Stateless profile 不共存）：`initialize` 返回 `2098-01-01` 且后续请求按 4.1 规则处理，证明新增更老/更新版本同样只依赖注册表声明顺序与 profile，而不是主干分支。
16. 上述用例全程未修改 `Session`/`Message`/任务的分发结构；若需要改主干才能通过，则判定本节失败。

### 7.4 MRTR（`integration.mrtr`）

1. 调用 `echo_confirm`（无 `inputResponses`）→ `resultType:"input_required"`，至少含 `inputRequests` 或 `requestState`，且**不含** `ttlMs`/`cacheScope`。
2. 携带 `inputResponses` + `requestState` 重试 → `resultType:"complete"`；该结果同样不含缓存字段。
3. 客户端未声明 `elicitation` 能力时：`NotifyInputRequired` 返回 `ERRNO_MISSING_REQUIRED_CLIENT_CAPABILITY`；应用调用 `NotifyMissingClientCapability` 时客户端收到 `-32021`，且 `error.data.requiredCapabilities` 含 `elicitation`。
4. 仅有 `inputRequests`（无 `requestState`）的 `input_required`，重试只带 `inputResponses` 也能成功；未声明能力时 `NotifyInputRequired` 返回 `ERRNO_MISSING_REQUIRED_CLIENT_CAPABILITY`，不产生非法 `inputRequests`。
5. 应用调用 `NotifyInputRequired` 但 `inputRequests`/`requestState` 都为空 → SDK 返回 `ERRNO_INVALID_PARAMS` 且不写结果；应用未产出其它结果时回 `-32602`。
6. `requestState` 被篡改时示例任务拒绝执行（验证应用侧完整性保护路径）。
7. 仅 `requestState`（无 `inputRequests`）的 load shedding 结果，重试只带 `requestState` 合法。
8. 对可缓存的 `resources/read` 做 MRTR 重试（请求携带 `inputResponses`/`requestState`，最终 `resultType:"complete"`）：结果**不得**包含 `ttlMs`/`cacheScope`，验证 `bMrtrRetry` 抑制而非仅靠 `tools/call` 天然不可缓存。

### 7.5 订阅（`integration.subscriptions`）

1. `subscriptions/listen` 的过滤器写在 `params.notifications`：`{"resourcesListChanged":true,"resourceSubscriptions":["echo://..."]}`：
   - 第一条订阅消息必须是 `notifications/subscriptions/acknowledged`，`_meta.subscriptionId` 等于 listen 请求 id，且只含被授予项。
2. listen 请求使用数字 `id` 时，ack 与后续通知的 `subscriptionId` 必须仍是数字；使用字符串 `id` 时保持字符串。
3. 触发资源更新：
   - 收到 `notifications/resources/updated`，`_meta["io.modelcontextprotocol/subscriptionId"]` 类型与值都等于 listen 请求 id。
4. 未订阅类型：不产生通知；子资源 URI 不因“前缀相同”而被投递。
5. 取消 listen 请求：订阅移除，后续不再推送；取消后再触发更新无通知。
6. 优雅关闭：`Terminate` 时收到 `subscriptions/listen` 的 `complete` 结果，`result._meta.subscriptionId` 存在且类型正确。

### 7.6 MCP Apps（`integration.apps`）

1. `server/discover` → `capabilities.extensions["io.modelcontextprotocol/ui"] == {}`。
2. legacy `initialize` → `result.capabilities.extensions` 不包含 `io.modelcontextprotocol/ui`（即使启用了 Apps）；legacy `tools/list`/`resources/read` 不出现任何 UI 元数据。
3. `tools/list` → `echo_app` 的 `_meta.ui.resourceUri == "ui://echo/app.html"`。
4. `resources/read ui://echo/app.html` → `mimeType == "text/html;profile=mcp-app"`，`_meta.ui.csp`/`_meta.ui.permissions` 原样透传；当前只透传这两个子域，不输出 SDK 未建模的字段。
5. `echo_app` 仍返回文本 `content`（优雅降级）。
6. 客户端 `_meta.clientCapabilities.extensions["io.modelcontextprotocol/ui"].mimeTypes` 可被读取，且不影响服务端返回 UI 资源。
7. 未调用 `RegisterServerAppsCapabilities()` 时，`server/discover` 不包含该扩展；未启用时任何版本都不输出 UI 元数据。
8. 应用自定义的非保留 `_meta` 键在结果中保留；协议保留键（`serverInfo`、`subscriptionId`）由 SDK 写入，应用同名值不覆盖。

### 7.7 双版本共存（`integration.dual_era`）

- 同一 stdio 进程先发 `initialize`（legacy）再发带 `_meta` 的现代请求：两者都应成功，互不污染状态。
- legacy `logging/setLevel` 设置会话级别后，现代请求缺 `logLevel` 时不得复用该级别发送 `notifications/message`。
- 现代请求带 `_meta.logLevel` 时可发送 `notifications/message`，且级别与请求值一致；该能力不读写会话级 `m_loggingLevel`。
- 现代请求不写入 `m_hashMessage`（或受上限约束），长时间连续现代请求后内存不随请求数线性增长。
- 重启进程后仅发现代请求：无需 `initialize` 即可工作。

### 7.8 单元测试覆盖清单

| CTest 名称 | 覆盖内容 |
|---|---|
| `unit.constants` | 两个协议版本字符串、method 名、错误码（`-32001` 保留 + `-32020`~`-32022`）、`notifications/tools/list_changed` |
| `unit.version_registry` | 注册表 `Find`/`GetSupportedVersions`/默认 Handshake 与最新 Stateless profile；`ProtocolContext` 默认值；`Supports()` feature 位；新增假 profile 后版本列表与 feature 自动生效；未知版本返回不可用上下文 |
| `unit.message` | `Message` 基础字段、时间戳/runtimeId、序列化稳定性、可选字段 |
| `unit.content_blocks` | text/image/audio/resource_link/tool_use/tool_result 的序列化与校验；`annotations`（两版均输出，`lastModified` 仅现代）、内容 `_meta`（`Feature_EntityMeta`）；空文本/空 blob 边界 |
| `unit.request` | 现代 `_meta` 解析、版本读取、`inputResponses`/`requestState`、`subscriptions/listen` 的 `params.notifications` 嵌套 |
| `unit.response` | `resultType`、`InputRequests inputRequests`、缓存字段仅在 6 类结果输出、`bMrtrRetry` 抑制缓存、`jServerInfo` 合并、`_meta` 保留应用非保留键、`RequestId subscriptionId` 类型保留 |
| `unit.notification` | ack 结构（`SubscriptionFilters`）、订阅通知、取消通知、日志通知结构；`RequestId subscriptionId` 类型保留；legacy 不出现 `subscriptionId` |
| `unit.meta` | `_meta` 命名空间 key、未知键透传、`logLevel`/客户端能力读取、`serverInfo` 注入、空 `_meta` 不输出 |
| `unit.session` | legacy/modern 判定（仅 `progressToken`/`toolCallId` 仍为 legacy）、版本协商回退、Handshake 版本取自注册表、带现代 `_meta` 的 `initialize` 仍走 Handshake、`-32022`、现代未知 method `-32601`、legacy `server/discover` 探测、缺 `_meta` `-32602`、现代不入 `m_hashMessage`、非 stdio 入口现代请求 `-32600` |
| `unit.tasks` | discover 必填字段、现代分页非法 cursor 带 `id`、资源不存在错误码（现代 `-32602` / legacy `-32002`）、缓存配置、`PrepareResult` 上下文复制 |
| `unit.mrtr` | `input_required` 至少一项、`inputResponses`/`requestState` 可独立出现、未声明能力时拒绝构造、`NotifyMissingClientCapability` 的 `-32021` data、不写缓存 |
| `unit.subscriptions` | 过滤器嵌套、ack-first、`subscriptionId` 类型保留、精确 URI 匹配、取消/优雅关闭清理 |
| `unit.caching` | 仅 6 类结果、`complete` 才输出、MRTR 不输出、默认值与显式覆盖、仅 `Feature_Caching` 开启时输出 |
| `unit.capabilities` | `completions` 由 `Feature_CompletionsCapability` 门控、`extensions` 显式透传、Apps `{}`、legacy initialize 透传、list_changed 能力、`logging` 能力保留 |
| `unit.apps` | `_meta.ui.resourceUri`/`visibility`、`UiResourceMeta` 全字段（csp 四域、permissions 四键、domain、prefersBorder、jExtra）、`ui://`、MIME 默认值、`Start()` 四个负例、`StructuredContent`、`JsonSchema`、降级内容 |
| `unit.legacy_compat` | 2024-11-05 输出不含现代字段；initialize 版本与能力不变；`-32001` 取值不变 |

## 8. 风险与规避

| 风险 | 规避 |
|---|---|
| 现代字段污染 legacy 输出 | 所有新字段由 `ctxProtocol.Supports(...)` 门控；7.2 回归用例逐项断言 |
| 异步 worker 未启动导致现代慢请求挂起 | `EnsureAsyncTaskThread()` 在现代分发入口调用 |
| 订阅表跨线程读写 | 互斥锁 + 只通过 `SendSubscriptionNotification` 访问 |
| listen 请求占用异步队列 | listen 在读线程同步登记，不进入 `m_deqAsyncTasks` |
| 分页 cursor 语义变化 | 保持 numeric cursor，不改客户端可见协议 |
| 构建遗漏新文件 | Windows vcxproj 显式加入；Linux/macOS 用 glob 并验证 |
| MRTR 状态跨请求丢失 | `requestState` 由应用自包含；SDK 只透传，不存会话状态 |
| 本地缺少完整 Apps 规范 | 只实现 `extensions/apps/overview.md` 与 `build.md` 明确的服务端子集；View↔Host 不实现，README 说明边界 |
| 旧错误码 `-32001` 被外部引用 | 保留常量名与取值不变；现代头不匹配使用新常量 `-32020`，README 注明两者区别 |
| 订阅 ack 前误发通知 | 订阅先以“未激活”登记，ack 写入成功后才激活；`integration.subscriptions` 断言首条消息为 ack |
| `subscriptionId` 丢失 JSON-RPC 类型 | 订阅表使用 `MakeRequestIdKey(RequestId)`；ack/通知/优雅关闭都用原始 `RequestId` 序列化；单测覆盖数字与字符串 id |
| 缓存字段无法判断是否为 MRTR 重试 | `Response::bMrtrRetry` 由 `PrepareResult` 从请求的 `inputResponses`/`requestState` 复制；`unit.response`/`unit.caching` 覆盖 |
| 现代日志回落到会话级 `m_loggingLevel` | 现代路径只走 `ProcessRequest::NotifyLogMessage` + `WriteLogNotification`，不调用会话级过滤；`integration.dual_era` 验证 |
| `m_hashMessage` 随现代请求无限增长 | 现代请求不入该表或设置上限；`integration.dual_era` 验证长时间请求后不线性增长 |
| `-32001` 与 `-32020` 混用 | legacy 只用 `-32001` 常量；现代代码只用 `ERRNO_HEADER_MISMATCH_MODERN`；单测锁定两值 |
| stdio-only 被误认为完整 2026 传输支持 | README 明确“2026-07-28 仅 stdio，非完整 transport 覆盖”；完成标准单列该条 |
| `_meta` 被 SDK 整体覆盖 | 统一 merge：先保留应用自定义的非保留键，再写协议保留键（`serverInfo`、`subscriptionId`）；单测/集成验证自定义键不丢、保留键不可伪造 |
| 集成测试与 `CMCPSession` 单例状态串扰 | 一个进程只跑一个 `--case` 场景；场景内使用独立的 `CTestServer` 配置 |
| 测试不稳定或超时 | 集成测试走内存传输，不使用真实网络/随机数据；CTest 统一 `TIMEOUT 30` |
| 新增字段缺少测试覆盖 | 按 5 节门禁表逐模块补 `unit.*`，特性级补 `integration.*`；全量 CTest 作为提交门禁 |
| 测试代码复制协议实现 | 测试目标直接编译 `Source/Protocol` 与仓库内 jsoncpp 源文件，不复制实现 |
| 编码风格漂移或整文件格式化 | 按 5 节风格约定逐项自查；只改必要行，禁止批量格式化；评审 diff 中的无关改动 |
| legacy 输出因新增可选字段漂移 | 所有新字段“有值才写”且由 `ctxProtocol.Supports(...)` 门控；7.2 对全部现有方法做 golden 逐字节对比，差异即失败 |
| `Response::DoSerialize` 写入被结果子类覆盖 | 采用 `WriteVersionResultFields(jResult, ctxProtocol, jServerInfo)` 由子类最后调用；单测直接断言序列化 JSON 含 `resultType` 且 legacy 无新增键 |
| 现有公开 API/类型被无意破坏 | 只允许追加；编译现有示例与测试作为回归门禁；评审检查签名 diff |
| `progressToken`/`toolCallId` 解析被 `MetaObject` 合并破坏 | 单测覆盖 legacy `_meta` 两种键；7.2 断言 `_meta` 仅含 `progressToken`、`toolCallId` 可读 |
| 新增第二套会话/任务/序列化基建 | 代码评审检查禁止项：不得新增 `ModernSession`/`ModernTask`/新 JSON 库/新传输抽象；复用清单写入 PR 模板 |
| 版本列表硬编码漂移 | `supportedVersions`/`-32022.data.supported`/`initialize` 回退版本只允许来自 `CProtocolRegistry`；`unit.version_registry` 与 `integration.version_extensibility` 锁定 |
| feature 位误用或漏设 | 新增字段必须同时补 `ProtocolFeature` 枚举值、profile 置位与门控代码；单测覆盖“开启有字段、关闭无字段”两种 profile |
| 未来版本改动侵入主干 | 演进用例要求“仅加 profile 即可通过”；评审检查分发/序列化中是否出现版本号字符串比较或“新/旧”布尔开关 |

## 9. 完成标准

- 上述 7.0–7.8 全部通过。
- `unit.*` 全绿；每个编码单元完成后均已执行对应单元测试。
- `integration.*` 全绿；每个特性完成后均已执行对应集成测试。
- 全量 `ctest --test-dir build-tests --output-on-failure` 0 失败后才允许提交/合并。
- 2024-11-05 的 stdio 与 HTTP+SSE 行为无回归；7.2 全部 golden 用例逐字节一致，legacy 结果不含现代字段，`-32001`/`-32002` 取值不变；legacy 既有错误/静默行为不变，仅新增 `server/discover` 探测响应。
- 旧代码无需修改源码即可重新编译：现有公开类型/方法签名不变，新增 API 均为追加；直接构造消息对象序列化时 `ctxProtocol` 为默认 `2024-11-05 / Era_Handshake`，输出与升级前一致。
- legacy `params._meta` 行为保持：仅 `progressToken` 时输出，不产生空 `_meta`；`toolCallId` 仍可解析。
- 复用约束已落实：未新增平行传输/会话/任务/序列化/注册体系，未引入第三方库；新类型均继承现有 `Request`/`Response`/`Notification`。
- 版本演进可扩展：`supportedVersions`、`-32022.data.supported`、`initialize` 回退版本全部来自 `CProtocolRegistry`，代码中无硬编码版本列表；`unit.version_registry` 与 `integration.version_extensibility` 通过，证明新增假版本只需增加 profile、无需改动检测/分发/序列化主干；分发与序列化代码中不存在版本号字符串比较或“新/旧”布尔开关。
- 2026-07-28 在 stdio 上完成 discover、双版本协商/回退、核心请求、MRTR、订阅、缓存、错误码、Apps 元数据。
- MCP Apps 仅在 2026-07-28 生效；2024-11-05 不声明该扩展、不输出任何 UI 元数据。
- README 明确写出：2026-07-28 仅提供 stdio binding，不是完整 transport 覆盖；Streamable HTTP 未实现。
- 必填 `_meta` 缺失时返回 `-32602` 且不补默认值；`-32021` 带 `data.requiredCapabilities`。
- 缓存字段只出现在 6 类可缓存 `complete` 结果上；`tools/call`、`prompts/get`、MRTR 结果不带。
- 订阅过滤器位于 `params.notifications`，ack 先于任何通知，`subscriptionId` 保留 string/integer 类型。
- Apps 只输出本地文档定义的 `_meta.ui.resourceUri`/`csp`/`permissions`，无 `domain`/`prefersBorder` 等自创字段。
- 现代请求不写会话级 `m_loggingLevel`，且不使 `m_hashMessage` 无限增长。
- 无新增第三方依赖；新增代码使用现有 `MCP::` 命名、`DoSerialize/DoDeserialize`、任务原型 + Clone 模式。
- 新增/修改代码与现有风格一致（命名、缩进、大括号、错误码、jsoncpp 用法），diff 中无无关格式化改动。
