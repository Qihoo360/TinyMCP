> ## Documentation Index
> Fetch the complete documentation index at: https://modelcontextprotocol.io/llms.txt
> Use this file to discover all available pages before exploring further.

# Extension Support Matrix

> Which MCP clients implement which official extensions

This matrix shows which MCP clients support each [official extension](https://modelcontextprotocol.io/extensions/overview.md). Extensions are always opt-in: a client only uses an extension if both client and server declare support in the `extensions` field of their [capabilities](https://modelcontextprotocol.io/extensions/overview.md#negotiation).

> [!NOTE]
>
> This list is maintained by the community. If you notice any inaccuracies or would like to add or update information, please [submit a pull request](https://github.com/modelcontextprotocol/modelcontextprotocol/pulls).

## Extension overview

| Extension | Identifier | Description |
| - | - | - |
| [MCP Apps](https://modelcontextprotocol.io/extensions/apps/overview.md) | `io.modelcontextprotocol/ui` | Interactive HTML interfaces rendered inline in the conversation |
| [OAuth Client Credentials](https://modelcontextprotocol.io/extensions/auth/oauth-client-credentials.md) | `io.modelcontextprotocol/oauth-client-credentials` | Machine-to-machine auth without interactive user login |
| [Enterprise-Managed Authorization](https://modelcontextprotocol.io/extensions/auth/enterprise-managed-authorization.md) | `io.modelcontextprotocol/enterprise-managed-authorization` | Centralized access control via enterprise IdP |
| [Skills over MCP](https://modelcontextprotocol.io/extensions/skills/overview.md) | `io.modelcontextprotocol/skills` | Discover skills and read their instructions and supporting files |

## Support matrix

| Client | [MCP Apps](https://modelcontextprotocol.io/extensions/apps/overview.md) | [OAuth Client Credentials](https://modelcontextprotocol.io/extensions/auth/oauth-client-credentials.md) | [Enterprise Auth](https://modelcontextprotocol.io/extensions/auth/enterprise-managed-authorization.md) | [Skills](https://modelcontextprotocol.io/extensions/skills/overview.md) | [Tasks](https://modelcontextprotocol.io/extensions/tasks/overview.md) |
| - | :-: | :-: | :-: | :-: | :-: |
| [Claude (web)](https://claude.ai) | | | | | |
| [Claude Desktop](https://claude.ai/download) | | | | | |
| [Claude Code](https://claude.com/claude-code) | | | | | |
| [VS Code GitHub Copilot](https://code.visualstudio.com/) | | | | | |
| [Microsoft 365 Copilot](https://www.microsoft.com/microsoft-365-copilot) | | | | | |
| [Goose](https://block.github.io/goose/) | | | | | |
| [Postman](https://postman.com) | | | | | |
| [MCPJam](https://www.mcpjam.com/) | | | | | |
| [ChatGPT](https://chatgpt.com/) | | | | [Partial](https://developers.openai.com/plugins/build/mcp-server#import-skills-from-the-mcp-server) | |
| [Cursor](https://cursor.com/) | | | | | |
| [Archestra.AI](https://www.archestra.ai/) | | | | | |
| [PostHog Code](https://posthog.com/code/) | | | | | |
| [Kiro Crew](https://kiro.dev/crew/) | | | | | |
| [fast-agent](https://github.com/evalstate/fast-agent) | | | | [Partial](https://github.com/evalstate/fast-agent/blob/main/docs/docs/mcp/skills-over-mcp.md) | |
| [MCP Inspector](https://github.com/modelcontextprotocol/inspector) | | | | [Partial](https://github.com/modelcontextprotocol/inspector/blob/main/clients/cli/README.md#skill-verification---verify) | |
| [mcpc MCP CLI](https://github.com/apify/mcpc) | | | | | |
| [Glean](https://www.glean.com/) | | | | | |

> [!NOTE]
>
> Auth extension support (OAuth Client Credentials and Enterprise-Managed Authorization) is tracked separately from the core MCP authorization features (DCR, CIMD). Check each extension's specification and the [ext-auth repository](https://github.com/modelcontextprotocol/ext-auth) for the latest implementation status.

## Adding extension support to your client

If you're building an MCP client and want to implement extension support:

1. Review the extension specification (e.g., in the [ext-auth](https://github.com/modelcontextprotocol/ext-auth) or [ext-apps](https://github.com/modelcontextprotocol/ext-apps) repository)
2. Declare support in the `extensions` field of the `io.modelcontextprotocol/clientCapabilities` your client sends in each request's `_meta`, and read the server's `extensions` from its [`server/discover`](https://modelcontextprotocol.io/specification/draft/server/discover) response
3. Implement the extension's protocol requirements
4. Submit a pull request to update this matrix

See [Extensions Overview](https://modelcontextprotocol.io/extensions/overview.md#negotiation) for details on the capability negotiation mechanism.
