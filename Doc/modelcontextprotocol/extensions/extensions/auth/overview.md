> ## Documentation Index
> Fetch the complete documentation index at: https://modelcontextprotocol.io/llms.txt
> Use this file to discover all available pages before exploring further.

# Authorization Extensions

> Supplementary authorization mechanisms for the Model Context Protocol

The [ext-auth repository](https://github.com/modelcontextprotocol/ext-auth) contains official MCP extensions that add authorization capabilities beyond the core MCP specification. These extensions address specific real-world scenarios where the standard OAuth 2.0 authorization code flow isn't the right fit.

> [card] [modelcontextprotocol/ext-auth](https://github.com/modelcontextprotocol/ext-auth)
>
> Source code, specifications, and reference implementations for MCP
> authorization extensions.

## Why authorization extensions?

The core MCP specification includes a robust [authorization framework](https://modelcontextprotocol.io/specification/latest/basic/authorization.md) built on OAuth 2.0. That framework handles the common case well: a user interactively grants an MCP client permission to access a server on their behalf.

But not every MCP deployment fits this pattern:

* **Machine-to-machine integrations** don't have a human in the loop. Background services, CI pipelines, and automated workflows need to authenticate without interactive user consent flows.
* **Enterprise environments** often have centralized identity providers (IdPs) that enforce policy across all applications. Requiring employees to authorize each MCP server individually creates friction and bypasses existing security controls.

The ext-auth extensions address these gaps.

## Available extensions

> [card] [OAuth Client Credentials](https://modelcontextprotocol.io/extensions/auth/oauth-client-credentials.md)
>
> Machine-to-machine authentication using the OAuth 2.0 client credentials
> flow. No user interaction required.

> [card] [Enterprise-Managed Authorization](https://modelcontextprotocol.io/extensions/auth/enterprise-managed-authorization.md)
>
> Centralized access control via enterprise identity providers. Employees
> access MCP servers through their organization's IdP.

## Choosing the right extension

| Scenario | Recommended extension |
| - | - |
| Background service or daemon accessing an MCP server | [OAuth Client Credentials](https://modelcontextprotocol.io/extensions/auth/oauth-client-credentials.md) |
| CI/CD pipeline calling MCP tools | [OAuth Client Credentials](https://modelcontextprotocol.io/extensions/auth/oauth-client-credentials.md) |
| Server-to-server API integration | [OAuth Client Credentials](https://modelcontextprotocol.io/extensions/auth/oauth-client-credentials.md) |
| Enterprise employees accessing MCP servers at work | [Enterprise-Managed Authorization](https://modelcontextprotocol.io/extensions/auth/enterprise-managed-authorization.md) |
| Organization-wide MCP access policy enforcement | [Enterprise-Managed Authorization](https://modelcontextprotocol.io/extensions/auth/enterprise-managed-authorization.md) |
| Standard interactive user authorization | Core MCP spec (no extension needed) |

## Client support

Authorization extension support varies by client. See the [client matrix](https://modelcontextprotocol.io/extensions/client-matrix.md) for a full breakdown. Both extensions require explicit support from the MCP client — they are never active by default.

## Specification

Both extensions are specified in the [ext-auth repository](https://github.com/modelcontextprotocol/ext-auth/tree/main/specification/stable). They use the standard MCP [extension negotiation](https://modelcontextprotocol.io/extensions/overview.md#negotiation) mechanism: clients declare support in the `extensions` field of the `io.modelcontextprotocol/clientCapabilities` they send in each request's `_meta`, and servers advertise theirs in the capabilities returned by [`server/discover`](https://modelcontextprotocol.io/specification/draft/server/discover).
