> ## Documentation Index
> Fetch the complete documentation index at: https://modelcontextprotocol.io/llms.txt
> Use this file to discover all available pages before exploring further.

# Extensions Overview

> Optional extensions to the Model Context Protocol

# MCP Extensions

MCP extensions are optional additions to the specification that define capabilities beyond the core protocol. Extensions enable functionality that may be modular (e.g., distinct features like authentication), specialized (e.g., industry-specific logic), or experimental (e.g., features being incubated for potential core inclusion).

Extensions are identified using a unique *extension identifier* with the format: `{vendor-prefix}/{extension-name}`, e.g. `io.modelcontextprotocol/oauth-client-credentials`. Identifiers follow the same rules as [`_meta` keys](https://modelcontextprotocol.io/specification/draft/basic/index#meta), with a mandatory prefix. Official extensions use the `io.modelcontextprotocol` vendor prefix.

> [!TIP]
>
> If you're building a third-party extension, use a reversed domain name you own as the vendor prefix to avoid collisions (similar to Java package naming). For example, a company owning `example.com` would use `com.example/` as their prefix (e.g., `com.example/my-extension`).

## Extension Lifecycle

Every extension is at one of three stages, and may later be promoted into the core specification. The stage decides which changes are allowed, who approves them, and what implementers can expect. For full details, see [SEP-3392: Extension Lifecycle](https://modelcontextprotocol.io/seps/3392-extension-lifecycle.md).

| Stage | Protocol changes | Changes approved by | Entry | What implementers can expect |
| - | - | - | - | - |
| **Experimental** | Any change, at any time, without notice | Extension Maintainer | Added by an Extension Maintainer | Nothing. The design may change or disappear. |
| **Beta** | Any change, with breaking changes expected to be rare | Extension Maintainer | Extensions Track SEP | Real usage is welcome. Breaking changes are documented. |
| **Stable** | Only with a new core protocol revision | Extension Maintainer | Second Extensions Track SEP | Behavior is fixed within a protocol revision. |

An extension starts as Experimental and must pass through Beta. From Beta it either becomes Stable, or a Standards Track SEP promotes it into the core specification. A Stable extension can also be promoted later. Each Beta extension states its intended destination, Stable or Core, which may change while it is in Beta.

An extension's identifier does not change when its stage changes.

## Official Extension Repositories

Official extensions live inside the [Model Context Protocol GitHub organization](https://github.com/modelcontextprotocol/) in repositories with the `ext-` prefix. One repository can hold several related extensions at different stages. Inside the repository, the directory that holds an extension's specification shows its stage: `specification/experimental/`, `specification/beta/` or `specification/stable/`. Extensions that were promoted to the core specification or have ended move to `specification/archived/`.

### MCP Authorization Extensions

> [card] [modelcontextprotocol/ext-auth](https://github.com/modelcontextprotocol/ext-auth)
>
> Extensions for supplementary authorization mechanisms beyond the core
> specification.

| Extension | Stage | Description |
| - | - | - |
| [OAuth Client Credentials](https://modelcontextprotocol.io/extensions/auth/oauth-client-credentials.md) | Stable | OAuth 2.0 client credentials flow for machine-to-machine authentication. |
| [Enterprise-Managed Authorization](https://modelcontextprotocol.io/extensions/auth/enterprise-managed-authorization.md) | Stable | Framework for enterprise environments requiring centralized access control. |

### MCP Apps

> [card] [modelcontextprotocol/ext-apps](https://github.com/modelcontextprotocol/ext-apps)
>
> Extensions for interactive UI elements in conversational MCP clients.

| Extension | Stage | Description |
| - | - | - |
| [MCP Apps](https://modelcontextprotocol.io/extensions/apps/overview.md) | Stable | Allows MCP Servers to display interactive UI elements (charts, forms, video players) inline within conversations |

To get started building MCP Apps, see the [quickstart guide](https://modelcontextprotocol.io/extensions/apps/build.md#getting-started) or read the full [MCP Apps documentation](https://apps.extensions.modelcontextprotocol.io/api/documents/Quickstart.html).

### MCP Tasks

> [card] [modelcontextprotocol/ext-tasks](https://github.com/modelcontextprotocol/ext-tasks)
>
> Extensions for long-running operations.

| Extension | Stage | Description |
| - | - | - |
| [MCP Tasks](https://modelcontextprotocol.io/extensions/tasks/overview.md) | Beta, to Core | Asynchronous task execution for long-running operations, with polling, mid-flight input, and durable handles. |

### Skills over MCP

> [card] [modelcontextprotocol/ext-skills](https://github.com/modelcontextprotocol/ext-skills)
>
> Discover and read Agent Skills from MCP servers.

| Extension | Stage | Description |
| - | - | - |
| [Skills over MCP](https://modelcontextprotocol.io/extensions/skills/overview.md) | Beta, destination not yet stated | Discover workflow instructions and read supporting files through MCP resources. |

See the [Skills overview](https://modelcontextprotocol.io/extensions/skills/overview.md) to get started and the
[client matrix](https://modelcontextprotocol.io/extensions/client-matrix.md) for implementation support.

### MCP Server Cards

> [card] [modelcontextprotocol/ext-server-card](https://github.com/modelcontextprotocol/ext-server-card)
>
> HTTP discovery of MCP server metadata.

| Extension | Stage | Description |
| - | - | - |
| [Server Cards](https://modelcontextprotocol.io/seps/2127-mcp-server-cards.md) | Beta, destination not yet stated | Lets clients discover a server's metadata before connecting to it. |

## Experimental Extensions

Experimental extensions provide an incubation pathway for [Working Groups and Interest Groups](https://modelcontextprotocol.io/community/working-interest-groups.md) to prototype ideas and collaborate on extension concepts before or alongside a SEP.

An Experimental extension lives either under `specification/experimental/` in an existing `ext-` repository, or in its own repository with the `experimental-ext-` prefix (e.g., `experimental-ext-interceptors`). Everything in an `experimental-ext-` repository is Experimental.

### Ground Rules

* Every experimental extension needs to be associated with a Working Group or Interest Group
* Published packages need to clearly indicate their experimental status (e.g., in the README and package name)
* Implementers should pin to a tag or commit, because two revisions of an Experimental extension need not be compatible
* [Core Maintainers](https://modelcontextprotocol.io/community/contributor-ladder.md#core-maintainer) retain oversight of experimental extensions, including the ability to archive or remove them

## Creating Extensions

1. **Prototype**: Add the extension under `specification/experimental/` in an `ext-` repository, or create an `experimental-ext-` repository.
2. **Propose Beta**: Create a SEP in the main MCP repository using the [standard SEP guidelines](https://modelcontextprotocol.io/community/sep-guidelines.md) with type **Extensions Track**. State whether the extension is intended to become Stable or to be promoted to Core.
3. **Implement**: Build at least one reference implementation in an official SDK. This is required before the SEP can be reviewed.
4. **Review**: [Core Maintainers](https://modelcontextprotocol.io/community/contributor-ladder.md#core-maintainer) review the SEP and have final authority over inclusion.
5. **Publish**: Once the SEP is accepted, open a PR that moves the extension to `specification/beta/` in an `ext-` repository. A Core Maintainer approves the move.
6. **Gather evidence**: During Beta, keep a public list of known implementations and the feedback received from them.
7. **Stabilize or promote**: A second Extensions Track SEP moves the extension to Stable. It needs two independent, interoperating implementations (one client-side, one server-side) and a [conformance scenario](https://modelcontextprotocol.io/community/sep-guidelines.md#conformance-test-requirement). Alternatively, a Standards Track SEP promotes the extension into the core specification.

### Requirements

* Extension specifications need to use RFC 2119 language (MUST, SHOULD, MAY)
* Extensions must have an associated working group or interest group, and at least one Extension Maintainer listed in the repository's `MAINTAINERS.md`

### SDK Implementation

SDKs can choose to implement extensions, but it's not required for protocol conformance. SDK maintainers have full autonomy over which extensions they support. Where an SDK does support extensions, SDK documentation should list which extensions are supported.

> [!NOTE]
>
> Extensions are always disabled by default and require explicit opt-in from the developer, at every stage.

### Evolution

Changes to an extension are approved by its Extension Maintainers and don't require Core Maintainer review. What may change depends on the stage:

* **Experimental**: anything, at any time.
* **Beta**: prefer additive changes, such as new optional fields or capability flags in the settings object. Breaking changes are allowed but should be rare and batched, announced in the Working Group or Interest Group first, and recorded in a changelog. An extension that makes a breaking change should add or increment a `revision` string in its settings object, so that peers can detect a mismatch.
* **Stable**: published as revisions keyed to core protocol revisions, such as `specification/stable/<name>/2026-07-28/`. A published revision is immutable. Changes accumulate in a `draft/` directory and take effect only with a new core protocol revision. The behavior of the extension for a pair of peers is the behavior defined for the protocol revision they negotiate.

Breaking changes keep the extension's identifier. A new identifier (e.g., `io.modelcontextprotocol/my-extension-v2`) is used only when old and new behavior must coexist under one protocol revision.

A **breaking change** is any modification that would cause existing implementations to fail or behave incorrectly, including:

* Removing or renaming fields
* Changing field types
* Altering the semantics of existing behavior
* Adding new required fields

### Promotion to Core

When a Standards Track SEP promotes an extension into the core specification in protocol revision R, the feature is core behavior for requests that declare R or later, whether or not the extension is advertised. For earlier revisions, the extension's final revision still applies, so a client that supports earlier revisions should keep advertising the extension. The extension's specification moves to `specification/archived/` once R is released.

## Negotiation

Clients and servers advertise their support for extensions in the `extensions` field within their respective capability declarations.

The examples below use protocol version `2026-07-28` and later. For `2025-11-25` and earlier, see [Initialization-Based Versions](#initialization-based-versions).

### Client Capabilities

Clients advertise extension support in `_meta["io.modelcontextprotocol/clientCapabilities"]` within each request:

```json theme={null}
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "tools/call",
  "params": {
    "name": "get_weather",
    "arguments": {
      "location": "New York"
    },
    "_meta": {
      "io.modelcontextprotocol/protocolVersion": "2026-07-28",
      "io.modelcontextprotocol/clientCapabilities": {
        "extensions": {
          "io.modelcontextprotocol/ui": {
            "mimeTypes": ["text/html;profile=mcp-app"]
          }
        }
      },
      "io.modelcontextprotocol/clientInfo": {
        "name": "ExampleClient",
        "version": "1.0.0"
      }
    }
  }
}
```

### Server Capabilities

Servers advertise extension support in the `server/discover` response:

```json theme={null}
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "resultType": "complete",
    "supportedVersions": ["2026-07-28"],
    "capabilities": {
      "tools": {},
      "extensions": {
        "io.modelcontextprotocol/ui": {}
      }
    },
    "_meta": {
      "io.modelcontextprotocol/serverInfo": {
        "name": "ExampleServer",
        "version": "1.0.0"
      }
    },
    "ttlMs": 3600000,
    "cacheScope": "public"
  }
}
```

Each extension specifies the schema of its settings object; an empty object indicates no settings.

### Initialization-Based Versions

Extensions became an official part of the protocol in version `2026-07-28`. Before that, many SDKs, clients, and servers already negotiated extensions on `2025-11-25` and earlier during the [`initialize` handshake](https://modelcontextprotocol.io/specification/2025-11-25/basic/lifecycle#initialization), using `params.capabilities.extensions` in the request and `result.capabilities.extensions` in the response. The extension identifiers and settings objects follow the same rules as above.

> [!NOTE]
>
> The `extensions` field is not part of the published schemas for these protocol versions, and supporting it is not required for conformance. SDKs that can carry `capabilities.extensions` through `initialize` are encouraged to do so, so that extensions work consistently across protocol versions. Check that your SDK can send and receive this field, and that the extension supports the negotiated protocol version. If the other side's capabilities omit `extensions`, or omit a given extension, treat that extension as unsupported and follow [Graceful Degradation](#graceful-degradation).

**Client request**

Clients advertise extension support in the `initialize` request:

```json theme={null}
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "initialize",
  "params": {
    "protocolVersion": "2025-11-25",
    "capabilities": {
      "roots": {
        "listChanged": true
      },
      "extensions": {
        "io.modelcontextprotocol/ui": {
          "mimeTypes": ["text/html;profile=mcp-app"]
        }
      }
    },
    "clientInfo": {
      "name": "ExampleClient",
      "version": "1.0.0"
    }
  }
}
```

**Server response**

Servers advertise extension support in the `initialize` response:

```json theme={null}
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "protocolVersion": "2025-11-25",
    "capabilities": {
      "tools": {},
      "extensions": {
        "io.modelcontextprotocol/ui": {}
      }
    },
    "serverInfo": {
      "name": "ExampleServer",
      "version": "1.0.0"
    }
  }
}
```

### Graceful Degradation

If one side supports an extension but the other doesn't, the supporting side needs to either fall back to core protocol behavior or reject the request with an appropriate error if the extension is mandatory.

It's a good practice to document expected fallback behavior in your extension. For example, a server offering UI-enhanced tools should still return meaningful text content for clients that don't support the UI extension. On the other hand, a server that requires a specific authentication extension can reject connections from clients that don't support it.
