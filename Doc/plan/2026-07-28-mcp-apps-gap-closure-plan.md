# TinyMCP MCP Apps 服务端能力补全实施计划

## 1. 背景与口径

本计划针对已入库的完整规范 `Doc/modelcontextprotocol/extensions/apps/apps.mdx`（SEP-1865，Stable 2026-01-26），补全 TinyMCP 作为 **MCP Server 侧 SDK** 在 MCP Apps 上的缺口。

固定口径：

1. MCP Apps 仅随 `2026-07-28` 提供；`2024-11-05` 明确不支持 MCP Apps。
2. legacy 会话既不声明 `io.modelcontextprotocol/ui`，也不输出任何 UI 元数据。
3. 只实现服务端职责；宿主/View 侧的 `postMessage`、sandbox、CSP 强制执行、display mode、theming、`tool-input/tool-result` 通知、visibility 过滤等全部不在范围内。
4. 不引入第三方依赖，沿用 jsoncpp、现有消息/任务/注册体系与 C++14 风格。
5. 任何改动不得改变 `2024-11-05` 既有输出；legacy golden 必须逐字节一致。

## 2. 缺口来源

| 类别 | 缺口 | 级别 |
|---|---|---|
| 版本口径 | `RegisterServerAppsCapabilities()` 仍让 legacy `initialize` 声明该扩展 | 项目口径（违反已确认决策） |
| URI 约束 | `ui://` 前缀无任何校验 | 规范 MUST（SDK 无保障） |
| MIME 约束 | UI 资源的 `mimeType` 无默认值/校验，`resources/read` 内容 MIME 无保障 | 规范 MUST |
| 引用完整性 | `Tool._meta.ui.resourceUri` 指向的资源是否存在、是否 `ui://` 无校验 | 规范 MUST |
| 内容形态 | 仅支持 text/blob 模型，不校验 HTML5 | 规范 MUST（校验缺失） |
| 客户端能力 | 无读取扩展设置与 `mimeTypes` 的 helper | 规范 SHOULD + 便利性 |
| 工具可见性 | 无 `visibility` 常量与 setter | 可选字段 + 便利性 |
| 资源元数据 | 无 `domain`/`prefersBorder`/CSP 子域/权限键常量，无 setter，示例/测试需手写 `jMeta`；文档与计划仍按旧口径 | 便利性 + 文档过时 |
| 常量卫生 | `EXTENSION_UI` 与 `EXTENSION_MCP_APPS_UI` 同值双定义 | 代码卫生 |
| 示例 | 示例 `resources/read` 的 contents 缺少 `_meta.ui` | 示例覆盖 |
| 测试 | 缺少负例与新增字段覆盖，`unit.apps` 位置不符约定 | 测试覆盖 |
| 废弃键 | 未明确禁止输出扁平 `_meta["ui/resourceUri"]` | 规范已废弃，只应支持嵌套 `_meta.ui.resourceUri` |
| 前缀保留 | 未写明 `ui://` 仅供 Apps 使用 | 规范保留前缀 |
| 结构化内容 | `CallToolResult.jStructuredContent` 直接暴露 `Json::Value`，调用者绕过数据模型 | 数据模型封装缺失（一致性） |
| 输入请求/响应 | `Request.jInputResponses`、`Response.jInputRequests` 未使用已有的 `InputResponses`/`InputRequests` | 数据模型封装缺失（一致性） |
| 订阅 ID | `Notification`/`SubscriptionsListenResult` 用裸 `Json::Value` 保存 id | 未复用 `RequestId`，类型约束丢失 |
| ACK 过滤器 | `SubscriptionsAcknowledgedNotification.jNotifications` 为裸 `Json::Value` | 固定结构未建模 |
| 通知发送 API | `SendSubscriptionNotification` 让调用者手写 `Json::Value` 参数 | 公共 API 封装缺失（破坏性调整） |
| JSON Schema | `Tool.jInputSchema`/`jOutputSchema` 为裸 `Json::Value` | 命名明确的字段未建模 |

参考位置：

- 能力声明：`Source/Protocol/Entity/Server.cpp` 的 `RegisterServerAppsCapabilities()`
- 实体 `_meta` 门控：`Source/Protocol/Message/BasicMessage.cpp` 的 `Tool/Resource/TextResourceContents::DoSerializeVersionExtras()`
- 结果级 `_meta`：`Source/Protocol/Message/Response.cpp` 的 `WriteVersionResultFields()`
- 常量：`Source/Protocol/Public/PublicDef.h`
- 示例：`Example/MCPServer/Source/EchoTask.cpp`、`EchoServer.cpp`
- 计划旧口径：`Doc/plan/2026-07-28-protocol-support-plan.md` 的 4.7、D21、Step 0

## 3. 设计决策

| 编号 | 决策 | 理由 |
|---|---|---|
| A1 | Apps 实现为“仅现代扩展”：注册时标记为 modern-only，legacy 能力序列化时过滤 | 与已确认口径一致，且保留其它扩展的 legacy 透传能力 |
| A2 | 校验放在 `Start()` 的配置期，而不是发送路径 | `Start()` 能拿到全部已注册实体，且返回 `int`；发送路径强制校验会改变既有语义并可能导致请求无响应 |
| A3 | 不校验 HTML5 合法性 | 无法可靠判定，误判成本高；由应用保证，文档写明 |
| A4 | MIME 默认值只作用于 `RegisterServerUiResources()` 的 `ui://` 资源 | 不改变通用 `RegisterServerResources()` 的行为，避免影响非 Apps 使用者 |
| A5 | 新增字段/常量只做追加，不修改现有公开方法签名 | 满足旧代码重新编译即可的约束 |
| A6 | 常量统一为 `EXTENSION_UI`，删除本分支新增的重复常量 `EXTENSION_MCP_APPS_UI` 并更新引用 | 该常量仅本分支引入，无历史包袱 |
| A7 | 不做 UI-only 资源过滤、不做按连接注册工具变体、不做 text-only 变体注册；`ui://` 前缀保留给 MCP Apps，非 Apps 资源不得使用 | 规范为 MAY/SHOULD，且与当前“Start 前全局注册”的架构冲突；用优雅降级与文档说明替代 |
| A8 | 不在发送路径强制 `CallToolResult::IsValid()` | 失败时没有既有的错误回写路径，改动风险高于收益；由应用保证并补测试 |
| A9 | UI 元数据必须通过 setter 建模：`Resource`/`TextResourceContents` 使用 `UiResourceMeta`，`Tool` 使用 `SetUiResourceUri`/`SetUiVisibility`；调用者不直接写 `jMeta` | 避免魔法键与多处手写不一致；内部仍序列化为 `_meta.ui`，未知字段用 `jExtra` 透传 |
| A10 | 迁移仓库内所有手写 `jMeta["ui"]` 的示例与测试到 setter | 统一口径，避免示例示范错误用法 |
| A11 | `structuredContent` 用 `StructuredContent` 数据模型封装，不在结果类型上直接暴露 `Json::Value` | 与 `TextContent`/`InputRequests` 的建模方式一致；调用者通过 `Set/Get` 使用，序列化/校验集中在类型内部 |
| A12 | 复用已有封装类型：`InputRequests`/`InputResponses`/`RequestId` 已存在但未被使用，直接接入，不新建平行类型 | 消除“类型已实现却仍用裸 JSON”的不一致 |
| A13 | 公共发送 API 改为类型化：`SendSubscriptionNotification` 只接受 `ResourceUpdatedNotification`，旧签名不保留 | 调用者不再手写通知 params；list_changed 已有专用发送方法 |
| A14 | `Tool.inputSchema`/`outputSchema` 用 `JsonSchema` 包装；`ToolResultContent.jContent` 维持裸 JSON | JSON Schema 字段含义明确，包装成本低；`jContent` 是多态内容数组，类型化成本高，本次不做 |

## 4. 实施步骤

### Phase 0：口径对齐（P0）

目标：legacy 不再声明 Apps 扩展。

改动：

1. `Source/Protocol/Message/BasicMessage.h`
   - `ServerCapabilities` 新增 `std::set<std::string> setModernOnlyExtensions;`（如未包含 `<set>` 需补充）。
   - 保持 `mapExtensions` 与 `bExtensionPassthrough` 语义不变。
2. `Source/Protocol/Message/BasicMessage.cpp`
   - `ServerCapabilities::DoSerializeVersionExtras()` 中，仅在 `!ctxProtocol.Supports(Feature_Extensions)` 且走 `bExtensionPassthrough` 分支时，跳过 `setModernOnlyExtensions` 中的键。
   - `DoDeserialize()` 同步读取该集合（可选：legacy 侧不需要，现代侧可直接由注册产生）。
3. `Source/Protocol/Entity/Server.cpp`
   - `RegisterServerAppsCapabilities()`：写入 `mapExtensions[EXTENSION_UI] = {}`，同时插入 `setModernOnlyExtensions`；不再依赖 `bExtensionPassthrough` 让 legacy 输出。
   - `RegisterServerExtensionsCapabilities()`：若注册键为 `EXTENSION_UI`，同样加入 `setModernOnlyExtensions`，防止通过通用 API 绕过口径。
4. 文档
   - `Readme.md`：删除“legacy `initialize` 可能回显该扩展”的说明，改为“legacy 不声明”。

门禁：

- `unit.message`（`ServerCapabilities` 现代/legacy 序列化）
- `unit.session`
- `integration.apps`
- `integration.legacy`（golden 不变）

### Phase 1：配置期校验（P1）

目标：在 `Start()` 前发现 Apps 配置错误，覆盖规范 MUST。

改动：

1. `Source/Protocol/Entity/Server.cpp`
   - 新增匿名命名空间辅助：
     - `bool IsUiResourceUri(const std::string&)`：前缀 `ui://`。
     - `int ValidateAppsConfiguration(const std::vector<Tool>&, const std::vector<Resource>&)`。
   - 校验复用 Phase 3 的公开 helper `IsUiResourceUri`/`IsMcpAppMimeType`；若先做 Phase 1，则先落地这两个 helper。
   - `Start()` 在 `Session().Ready()` 之前调用校验；仅在 `Session().GetServerCapabilities().mapExtensions` 含 `EXTENSION_UI` 时执行。
   - 校验规则：
     1. 所有 `ui://` 资源都视为 UI 资源，其 `strMimeType` 必须为 `MIME_TYPE_MCP_APP`；非 Apps 资源不得占用 `ui://`。
     2. 工具的 `_meta.ui.resourceUri` 若存在，必须是字符串、以 `ui://` 开头、且能在已注册资源中精确命中。
     3. 工具的 `_meta.ui.visibility` 若存在，必须是数组，元素只能是 `model` 或 `app`。
   - 任一不满足返回 `ERRNO_INVALID_PARAMS`，不启动。
2. `Source/Protocol/Entity/Server.cpp`
   - `RegisterServerUiResources()`：复制入参，为 `ui://` 且 `strMimeType` 为空的资源补 `MIME_TYPE_MCP_APP`，再调用 `RegisterServerResources()`；不修改通用注册 API。
3. 可选 helper（若 Phase 3 不做则放本阶段）
   - `MCP::BuildUiResourceContents(const Resource&, const std::string& strHtml, TextResourceContents&)`：设置 `uri`、`mimeType`、`text`，并通过 `TextResourceContents::SetUiMeta()` 复制 `Resource._meta.ui`，避免应用两处手写不一致。
4. 运行期边界
   - `resources/read` 内容的 MIME 不做运行时强制；由 `BuildUiResourceContents`、配置期校验与文档保证，集成测试覆盖示例路径。
   - 该边界属于已知取舍，必须在验收标准与 README 中写明。

门禁：

- `unit.apps`
- `unit.message`
- `integration.apps`（合法配置通过、非法配置 `Start()` 失败）
- `integration.legacy`（未注册 Apps 时行为不变）

### Phase 2：客户端 UI 能力读取（P1）

目标：提供 `getUiCapability` 等价能力，落实 SHOULD。

改动：

1. `Source/Protocol/Message/BasicMessage.h`
   - `ClientCapabilities` 新增：
     - `const Json::Value* FindExtensionSettings(const std::string& strExtensionId) const;`
     - `bool SupportsMimeType(const std::string& strExtensionId, const std::string& strMimeType) const;`
2. `Source/Protocol/Message/BasicMessage.cpp`
   - 从 `mapExtensions` 优先查找，其次 `jRaw["extensions"]`；`mimeTypes` 必须是非空字符串数组。
   - 不新增 Apps 专用方法，保持通用。
3. 文档
   - README 增加示例：`caps.SupportsExtension(EXTENSION_UI) && caps.SupportsMimeType(EXTENSION_UI, MIME_TYPE_MCP_APP)`。
4. 使用指引
   - 现代请求在 `tools/list`/`tools/call` 中通过 `Request::meta.clientCapabilities` 调用上述 helper。
   - 注册发生在 `Start()` 之前，无法按连接注册不同工具变体；只能按请求判断并走优雅降级。

门禁：

- `unit.message`
- `unit.apps`

### Phase 3：字段、常量与结构化内容（P2）

目标：让应用无需手写魔法字符串即可表达完整 `_meta.ui`，并让 `structuredContent` 走数据模型而不是裸 JSON。

改动：

1. `Source/Protocol/Public/PublicDef.h`
   - 新增键：`visibility`、`domain`、`prefersBorder`、`connectDomains`、`resourceDomains`、`frameDomains`、`baseUriDomains`、`camera`、`microphone`、`geolocation`、`clipboardWrite`。
   - 新增值：`UI_VISIBILITY_MODEL = "model"`、`UI_VISIBILITY_APP = "app"`。
   - 删除 `EXTENSION_MCP_APPS_UI`，统一使用 `EXTENSION_UI`，并更新全部引用（含测试）。
   - 新增公开 helper（放在现有 helper 头，如 `Source/Protocol/Public/StringHelper.h`）：`bool IsUiResourceUri(const std::string&)`、`bool IsMcpAppMimeType(const std::string&)`；Phase 1 的校验改为复用这两个函数。
2. `Source/Protocol/Message/BasicMessage.h/.cpp`
   - `Tool::SetUiVisibility(const std::vector<std::string>&)`：写入 `jMeta["ui"]["visibility"]`，保留 `resourceUri` 等已有键。
   - 新增 `MCP::UiResourceMeta`：`vecConnectDomains`/`vecResourceDomains`/`vecFrameDomains`/`vecBaseUriDomains`、`bCamera`/`bMicrophone`/`bGeolocation`/`bClipboardWrite`、`strDomain`/`bHasDomain`、`bPrefersBorder`/`bHasPrefersBorder`、`jExtra`；`DoSerialize()` 负责组装 `_meta.ui`。
   - `Resource::SetUiMeta(const UiResourceMeta&)`、`TextResourceContents::SetUiMeta(const UiResourceMeta&)`：内部写 `jMeta["ui"]`，保留未建模的键；调用者不再手写 `jMeta`。
   - `Tool` 只保留 `SetUiResourceUri`/`SetUiVisibility`，不提供裸 `jMeta` 写入方式。
   - `BuildUiResourceContents` 内部调用 `TextResourceContents::SetUiMeta()`，从 `Resource` 复制 `_meta.ui`，保证 `resources/read` 与声明一致。
   - 不输出废弃的扁平键 `_meta["ui/resourceUri"]`：`SetUiResourceUri` 只写嵌套路径；应用手写该键属于未支持用法，文档需注明。
3. `Source/Protocol/Message/Response.h/.cpp`、`BasicMessage.h/.cpp`
   - 新增 `MCP::StructuredContent` 数据模型：私有 `Json::Value m_jData`，提供 `Set(const Json::Value&)`、`Get()`、`HasValue()`、`Clear()`、`IsValid()`、`DoSerialize()`、`DoDeserialize()`；不新增 `MessageType`（它不是 JSON-RPC 消息，仅作结果字段模型）。
   - `CallToolResult` 的 `Json::Value jStructuredContent` 改为 `MCP::StructuredContent structuredContent`；仅在 `Feature_StructuredContent` 且 `HasValue()` 时序列化，`IsValid()` 要求对象。
   - `ToolResultContent` 的 `jStructuredContent` 同步改为 `structuredContent`，保持同名同语义字段一致。
   - 迁移 `Example/MCPServer/Source/EchoTask.cpp` 的 `spResult->jStructuredContent = jStructured` 以及 `Tests/UnitTests/TestResponse.cpp`、`TestBasicMessage.cpp` 的直接赋值，改用 `structuredContent.Set(...)`。
4. 计划文档
   - 修正 `4.7` 首句：删除“严格按 overview.md、apps/build.md 实施，不引入本地文档没有的字段”，改为以已入库的 `apps.mdx` 为准。
   - 修正 `4.7`：资源元数据由“只透传 csp/permissions”改为“透传 `csp`/`permissions`/`domain`/`prefersBorder`；CSP 四子域与权限四键均按 JSON 原样透传”。
   - 修正 `4.7`：删除“不为 `_meta.ui` 建强类型”“SDK 不解释、不校验、不拆子域”的旧表述，改为通过 `UiResourceMeta`/setter 建模，未知字段仍由 `jExtra` 透传。
   - 修正 `4.7`：在工具关联中补上 `visibility`。
   - 修正 `D21`：删除“未在入库文档出现”的表述，改为“通过 setter 建模；未知字段只透传，不解释、不校验”。
   - 修正 `Step 0`：删除“不要再创建 apps.mdx（未入库）”。

门禁：

- `unit.constants`
- `unit.message`
- `unit.apps`
- `integration.apps`

### Phase 3B：其余数据模型封装（P2）

目标：接入已有封装类型，并把公共 API 的裸 JSON 参数改为类型化对象。

改动：

1. `MCP::InputRequests` / `MCP::InputResponses`
   - 将公开 `Json::Value jItems` 改为私有 `m_jItems`，提供 `Set(const Json::Value&)`、`Get()`、`Has()`、`Get(key)`、`HasValue()`、`Clear()`、`IsValid()`、`DoSerialize()`、`DoDeserialize()`。
   - `Request.jInputResponses` 改为 `MCP::InputResponses inputResponses`；`Response.jInputRequests` 改为 `MCP::InputRequests inputRequests`。
   - `Request::DoSerialize`/`DoDeserialize`、`Response::WriteVersionResultFields`、`PrepareResult` 的 `bMrtrRetry` 判断改用 `HasValue()`。
   - `NotifyInputRequired` 保留现有 `const Json::Value&` 重载（内部转成 `InputRequests`），并追加 `const MCP::InputRequests&` 重载；三类任务同步追加。
2. `subscriptionId`
   - `Notification.jSubscriptionId`（Json）+ `bHasSubscriptionId` 改为 `MCP::RequestId subscriptionId` + `bHasSubscriptionId`；`SubscriptionsListenResult.jSubscriptionId` 改为 `MCP::RequestId subscriptionId`。
   - 为 `RequestId` 增加 `int WriteValue(Json::Value& jValue) const`（只写标量值，不带键），`DoSerialize` 复用它；`WriteVersionParamsMeta` 与 `SubscriptionsListenResult` 通过它写入 `_meta["io.modelcontextprotocol/subscriptionId"]`，保留 string/integer 类型。
   - 更新 `Session` 中 ACK、订阅通知、优雅关闭三处赋值与序列化；`MakeRequestIdKey` 仍以 `RequestId` 生成键。
3. `SubscriptionFilters`
   - 新增 `MCP::SubscriptionFilters`：`bToolsListChanged`/`bPromptsListChanged`/`bResourcesListChanged`、`vecResourceSubscriptions`；`DoSerialize` 只输出 true 的布尔与非空数组，`DoDeserialize`/`IsValid` 对称实现。
   - `SubscriptionsAcknowledgedNotification.jNotifications` 改为 `MCP::SubscriptionFilters filters`，序列化到 `params.notifications`。
   - `SubscriptionsListenRequest` 保持现有扁平字段不变，内部构造 `SubscriptionFilters` 供 ACK 使用。
4. 通知发送 API（破坏性调整）
   - 删除 `SendSubscriptionNotification(const std::string& method, const Json::Value& params, const std::string& strMatchUri)`。
   - 改为 `int SendSubscriptionNotification(const MCP::ResourceUpdatedNotification& notification, const std::string& strMatchUri)`；`CMCPServerBase`、`CMCPSession`、`SessionAccess` 同步替换。
   - list_changed 继续使用 `SendToolListChanged`/`SendPromptListChanged`/`SendResourceListChanged`。
   - 更新示例与测试，改为构造 `ResourceUpdatedNotification`。
5. `JsonSchema`
   - 新增 `MCP::JsonSchema`：私有 `Json::Value m_jData`，提供 `Set/Get/HasValue/IsValid/DoSerialize/DoDeserialize`；`IsValid()` 要求对象。
   - `Tool.jInputSchema`/`jOutputSchema` 改为 `MCP::JsonSchema inputSchema`/`outputSchema`；同步 `Tool::IsValid`、序列化/反序列化、`EchoServer` 与测试。
   - 这是破坏性字段改名，README 必须给出迁移说明；`ToolResultContent.jContent` 按确认结果保持裸 JSON。
6. 计划文档
   - 修正主计划 Step 2/3/5 中 `structuredContent`、`inputRequests/inputResponses`、`subscriptionId` 的裸 JSON 表述，改为对应数据模型。
   - 修正主计划 Step 2 中 `Tool.inputSchema/outputSchema` 的描述，改为 `JsonSchema`。
   - 修正主计划 Step 4/5 中 `SendSubscriptionNotification` 的签名说明，改为类型化 `ResourceUpdatedNotification`。

门禁：

- `unit.message`
- `unit.request`
- `unit.response`
- `unit.notification`
- `unit.mrtr`
- `unit.subscriptions`
- `integration.apps`
- `integration.modern_core`
- `integration.subscriptions`
- `integration.legacy`

### Phase 4：示例与测试补强（P2）

1. 示例
   - `Example/MCPServer/Source/EchoTask.cpp`：`CEchoAppResourceReadTask` 通过 `TextResourceContents::SetUiMeta()` 补 `_meta.ui`，或直接使用 `BuildUiResourceContents()`，与 `EchoServer.cpp` 中的 `UiResourceMeta` 保持一致。
   - `EchoServer.cpp`：把现有 `appResource.jMeta[...]` 手写改为 `appResource.SetUiMeta(uiMeta)`；给 `echo_app` 调用 `SetUiVisibility({UI_VISIBILITY_MODEL, UI_VISIBILITY_APP})`。
   - `EchoServer.cpp`：定时推送改为构造 `ResourceUpdatedNotification` 调用 typed `SendSubscriptionNotification`。
   - `EchoServer.cpp`：`ParseInputSchema` 改为 `JsonSchema.Set()`，不再直接写 `jInputSchema`/`jOutputSchema`。
   - 清理示例中所有手写 `_meta.ui` 的代码，统一走 setter。
2. 单元测试
   - `Tools::SetUiResourceUri` 保留其它 `_meta.ui` 键，且不产生扁平 `_meta["ui/resourceUri"]`。
   - `SetUiVisibility` 序列化；legacy 不输出。
   - `UiResourceMeta::DoSerialize` 覆盖 csp 四域、permissions 四键、domain、prefersBorder 与 `jExtra` 透传。
   - `Resource::SetUiMeta`/`TextResourceContents::SetUiMeta` 保留 `jMeta` 中未建模的键。
   - `ClientCapabilities::FindExtensionSettings`/`SupportsMimeType` 的正常与缺失分支。
   - `IsUiResourceUri`/`IsMcpAppMimeType` 的正常、空串与前缀不匹配分支。
   - `BuildUiResourceContents`（若实现）复制 uri/mime/meta。
   - `ui://` 前缀、MIME 默认值、`Start()` 校验的四个负例。
   - 迁移后仓库内不再出现示例/测试代码手写 `jMeta["ui"]`。
   - `StructuredContent` 的 `Set/Get/HasValue/IsValid`、序列化/反序列化；非对象输入被拒绝。
   - 迁移后示例/测试不再直接赋值 `jStructuredContent`。
   - `InputRequests`/`InputResponses` 的 `Set/Get/Has/HasValue/IsValid` 与序列化；`Request`/`Response` 的字段改为对应类型。
   - `RequestId::WriteValue` 保留 string/integer 类型；ACK、订阅通知、优雅关闭三处一致。
   - `SubscriptionFilters` 只输出被授予的项；空授予输出空对象。
   - `JsonSchema` 的 `Set/Get/HasValue/IsValid`；非对象输入被拒绝。
   - `SendSubscriptionNotification` 的 typed 签名编译通过，旧签名不再存在（编译期负例可省略）。
3. 集成测试
   - legacy `initialize` 不出现 `io.modelcontextprotocol/ui`，即使已注册。
   - modern `server/discover` 出现该扩展。
   - `tools/list` 带 `visibility`。
   - `resources/read` 的 contents 带完整 `csp`（四子域）、`permissions`（四键）、`domain`、`prefersBorder` 原样透传。
   - 非法配置：`ui://` 资源 MIME 非 mcp-app、`resourceUri` 非 `ui://`、`resourceUri` 不存在、`visibility` 非法值，`Start()` 返回 `ERRNO_INVALID_PARAMS`。
   - 优雅降级：`echo_app` 仍返回文本 `content`。
   - 补充 `blob` 形式 HTML 的读取用例。
   - 将 `unit.apps` 从 `Tests/IntegrationTests/Cases/TestModernCase.cpp` 迁到 `Tests/UnitTests/TestApps.cpp`（或按现有 CMake 约定登记），保持 CTest 名称不变。
4. 构建
   - 若新增源文件，同步 `Example/MCPServer/Build/Windows/MCPServer.vcxproj`；`Tests/CMakeLists.txt` 使用显式源列表时追加。
   - Linux/macOS 走 glob，确认路径命中。

门禁：

- `unit.apps`、`unit.message`、`unit.constants`
- `integration.apps`、`integration.legacy`、`integration.modern_core`
- 全量 `ctest --output-on-failure`

### Phase 5：文档收口（P2）

1. `Readme.md`
   - 删除 legacy 回显扩展的表述（Phase 0 后不再成立）。
   - 补充 `visibility`、`domain`、`prefersBorder`、CSP 四子域、权限四键、`blob` 形式与 HTML5 要求。
   - 写明 `Start()` 会校验 Apps 配置，以及校验失败的错误码。
   - 写明“SDK 不校验 HTML5、不强制工具返回文本”的边界。
   - 写明 `ui://` 仅供 Apps 使用、扁平 `_meta["ui/resourceUri"]` 不再输出、`resources/read` 内容 MIME 不做运行时强制。
   - 写明 UI 元数据统一通过 `UiResourceMeta`/setter 声明，示例不示范手写 `jMeta`。
   - 写明 `structuredContent` 通过 `StructuredContent.Set()` 设置，示例不直接赋值 `jStructuredContent`。
   - 写明破坏性迁移：`Tool.inputSchema`/`outputSchema` 改用 `JsonSchema`；`SendSubscriptionNotification` 改为接收 `ResourceUpdatedNotification`，旧签名已删除。
   - 写明 `inputRequests`/`inputResponses`、`subscriptionId`、ACK 过滤器使用 `InputRequests`/`InputResponses`/`RequestId`/`SubscriptionFilters`。
2. `Doc/plan/2026-07-28-protocol-support-plan.md`
   - 按 Phase 3 修正 4.7 首句/4.7 元数据/4.7 可见性/D21/Step 0。
   - 修正 Step 2/3/5：`structuredContent`、`inputRequests/inputResponses`、`subscriptionId`、ACK 过滤器由裸 `Json::Value` 改为对应数据模型。
   - 修正 Step 2/4/5：`Tool.inputSchema/outputSchema` 改为 `JsonSchema`；`SendSubscriptionNotification` 改为类型化 `ResourceUpdatedNotification`。
   - 在 7.6 增加上述负例与新增字段断言。
3. 新增本文件作为补全计划，不修改已入库的规范与 schema 原文。

## 5. 测试门禁汇总

| 阶段 | 单元 | 集成 |
|---|---|---|
| Phase 0 | `unit.message`、`unit.session` | `integration.apps`、`integration.legacy` |
| Phase 1 | `unit.apps`、`unit.message` | `integration.apps`、`integration.legacy` |
| Phase 2 | `unit.message`、`unit.apps` | — |
| Phase 3 | `unit.constants`、`unit.apps` | `integration.apps` |
| Phase 3B | `unit.message`、`unit.request`、`unit.response`、`unit.notification`、`unit.mrtr`、`unit.subscriptions` | `integration.apps`、`integration.modern_core`、`integration.subscriptions`、`integration.legacy` |
| Phase 4 | 全量 `unit.*` | 全量 `integration.*` |
| Phase 5 | 全量 `ctest` | 全量 `ctest` |

失败即停：任一用例失败不得进入下一阶段；修复后从失败用例重跑，再跑全量回归。

## 6. 验收标准

1. 注册 Apps 后，legacy `initialize` 与 legacy `tools/list`/`resources/read` 均无任何 Apps 痕迹；modern `server/discover` 正常声明。
2. 非法 Apps 配置在 `Start()` 被拒绝：`ui://` 资源 MIME 错误、`resourceUri` 非 `ui://` 或指向不存在的资源、`visibility` 值非法。
3. `RegisterServerUiResources()` 为空的 `ui://` 资源补 `MIME_TYPE_MCP_APP`。
4. `ClientCapabilities` 可直接判断扩展与 `mimeTypes`，无需手写 JSON 解析。
5. `Tool` 可声明 `visibility`；`Resource`/`TextResourceContents` 通过 `UiResourceMeta` setter 声明 `csp`/`permissions`/`domain`/`prefersBorder`，常量齐全，调用者无需手写 `jMeta`。
6. 示例的 `resources/read` 带 `_meta.ui`，与 `resources/list` 一致。
7. `2024-11-05` golden 逐字节不变；全量 `ctest` 通过。
8. README 与计划文档不再有“legacy 支持 Apps”“domain/prefersBorder 未定义”等过时表述。
9. 文档明确：`ui://` 仅供 Apps 使用；扁平 `_meta["ui/resourceUri"]` 不再输出；`resources/read` 内容 MIME 不做运行时强制，由 helper 与文档保证。
10. 文档明确：客户端能力 helper 在 `tools/list`/`tools/call` 中按请求使用，注册期无法按连接注册工具变体。
11. 示例与测试不再手写 `jMeta["ui"]`；`UiResourceMeta` 覆盖全部文档化字段并保留 `jExtra` 透传。
12. `CallToolResult`/`ToolResultContent` 不再暴露裸 `Json::Value`；示例与测试统一使用 `StructuredContent.Set()`。
13. `Request`/`Response` 使用 `InputResponses`/`InputRequests`；示例与测试不再直接构造裸 `inputResponses`/`inputRequests`。
14. `subscriptionId` 使用 `RequestId`，ACK、订阅通知、优雅关闭均保留 string/integer 类型。
15. ACK 使用 `SubscriptionFilters`，只输出被授予的项；空授予输出空对象。
16. `SendSubscriptionNotification` 只接受 `ResourceUpdatedNotification`，旧签名已删除，示例与测试同步。
17. `Tool` 使用 `JsonSchema`；`ToolResultContent.jContent` 保持裸 JSON 并在文档中说明原因。

## 7. 风险与规避

| 风险 | 规避 |
|---|---|
| legacy 过滤误伤其它扩展 | 只过滤 `setModernOnlyExtensions` 中的键；通用扩展仍走 `bExtensionPassthrough`；用集成测试锁定 |
| `Start()` 校验导致既有 App 启动失败 | 仅当注册了 Apps 时校验；错误信息通过错误码暴露；文档提前说明 |
| 校验与动态资源模板冲突 | 只按已注册的具体资源精确匹配；模板不参与 UI 资源引用（`ui://` 资源本就是具体资源） |
| 常量去重影响编译 | 该常量仅本分支引入；一次性更新全部引用并全量构建 |
| 新增字段污染 legacy 输出 | 所有新增键仍受 `Feature_EntityMeta`/`Feature_Extensions` 门控；golden 回归兜底 |
| 发送路径校验导致请求无响应 | 不采用发送路径强制校验（A2/A8） |
| 示例与测试配置漂移 | 示例与 `CTestServer` 使用同一组 csp/permissions/visibility；集成测试断言完整字段 |
| `StructuredContent` 改动影响既有调用方 | 该字段仅本分支引入；迁移示例/测试，未设置时不输出，legacy 行为不变 |
| 破坏性 API 调整影响既有调用方 | 仅涉及本分支新增能力相关 API；README 提供迁移示例，示例/测试同步更新 |
| `JsonSchema` 只接受对象 | MCP schema 要求 `inputSchema`/`outputSchema` 为 object；布尔 schema 不支持，文档写明 |

## 8. 明确不做

1. `postMessage`、sandbox proxy、`ui/*` 生命周期与通知。
2. CSP/permissions 的实际下发与强制执行、display mode、theming、containerDimensions。
3. HTML5 合法性校验、`resources/read` 内容的 MIME 运行时强制。
4. 在发送路径强制执行 `CallToolResult::IsValid()`。
5. UI-only 资源从 `resources/list` 中省略的标记能力。
6. 按客户端能力注册不同工具变体、text-only 变体注册。
7. 2024-11-05 上的任何 MCP Apps 能力。
8. `ToolResultContent.jContent` 的类型化（保持裸 JSON 透传，文档说明原因）。
9. 其余任意结构字段（`_meta`、`arguments`、error data、log data、extension settings 等）保持裸 JSON 透传。
