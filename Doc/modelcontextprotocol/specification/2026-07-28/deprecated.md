> ## Documentation Index
> Fetch the complete documentation index at: https://modelcontextprotocol.io/llms.txt
> Use this file to discover all available pages before exploring further.

# Deprecated Features

This page is the registry of specification features that are currently in the
**Deprecated** state under the
[feature lifecycle and deprecation policy](https://modelcontextprotocol.io/community/feature-lifecycle.md)
([SEP-2596](https://github.com/modelcontextprotocol/modelcontextprotocol/pull/2596)).

A Deprecated feature remains part of the specification but is scheduled for
removal: new implementations **SHOULD NOT** adopt it, and existing
implementations **SHOULD** migrate before the feature's earliest removal. The
earliest removal marks when a feature becomes *eligible* for removal; the
actual removal is a Core Maintainer decision taken during release preparation
and may happen later.

This registry is a derived view kept consistent with the per-feature
deprecation notices and changelog entries, which are the normative records.

## Deprecated

| Feature | Deprecation SEP | Deprecated in | Migration path | Earliest removal |
| - | - | - | - | - |
| [Roots](https://modelcontextprotocol.io/specification/2026-07-28/client/roots.md) | [SEP-2577](https://github.com/modelcontextprotocol/modelcontextprotocol/pull/2577) | `2026-07-28` | Pass directories or files via tool parameters, resource URIs, or server configuration | First revision released on or after 2027-07-28 |
| [Sampling](https://modelcontextprotocol.io/specification/2026-07-28/client/sampling.md) | [SEP-2577](https://github.com/modelcontextprotocol/modelcontextprotocol/pull/2577) | `2026-07-28` | Integrate directly with LLM provider APIs | First revision released on or after 2027-07-28 |
| [Logging](https://modelcontextprotocol.io/specification/2026-07-28/server/utilities/logging.md) | [SEP-2577](https://github.com/modelcontextprotocol/modelcontextprotocol/pull/2577) | `2026-07-28` | Log to `stderr` for stdio transports; use [OpenTelemetry](https://opentelemetry.io/) for observability | First revision released on or after 2027-07-28 |
| [Dynamic Client Registration](https://modelcontextprotocol.io/specification/2026-07-28/basic/authorization/client-registration.md#dynamic-client-registration) | [PR #2858](https://github.com/modelcontextprotocol/modelcontextprotocol/pull/2858) | `2026-07-28` | [Client ID Metadata Documents](https://modelcontextprotocol.io/specification/2026-07-28/basic/authorization/client-registration.md#client-id-metadata-documents) | First revision released on or after 2027-07-28 |
| `includeContext: "thisServer"` / `"allServers"` ([Sampling](https://modelcontextprotocol.io/specification/2026-07-28/client/sampling.md#capabilities)) | [SEP-2596](https://github.com/modelcontextprotocol/modelcontextprotocol/pull/2596) | `2025-11-25` | Omit the field or use `"none"` | Follows Sampling ([SEP-2577](https://github.com/modelcontextprotocol/modelcontextprotocol/pull/2577)) |
| [HTTP+SSE transport](https://modelcontextprotocol.io/specification/2024-11-05/basic/transports.md#http-with-sse) | [SEP-2596](https://github.com/modelcontextprotocol/modelcontextprotocol/pull/2596) | `2025-03-26` | [Streamable HTTP](https://modelcontextprotocol.io/specification/2026-07-28/basic/transports/streamable-http.md) | Three months after SEP-2596 reaches Final |

The HTTP+SSE transport and the `includeContext` values were already described
as deprecated before the lifecycle policy existed; SEP-2596 reclassifies them
as Deprecated under its [transition provisions](https://modelcontextprotocol.io/community/feature-lifecycle.md).

## Removed

No features have been removed under this policy yet. When a Deprecated feature
is removed, its row moves to this section with a link to the changelog entry
recording the removal.
