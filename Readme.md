# TinyMCP

## What is TinyMCP?
TinyMCP is a lightweight C++ SDK for implementing the MCP Server.

## What is MCP?
MCP is an open protocol that standardizes how applications provide context to LLMs. Think of MCP like a USB-C port for AI applications. Just as USB-C provides a standardized way to connect your devices to various peripherals and accessories, MCP provides a standardized way to connect AI models to different data sources and tools.
> [Reference for details](https://modelcontextprotocol.io/introduction)

## TinyMCP supported system platforms
* Windows
* Linux
* MacOS
* Android
* Other platforms that support standart c++...

## Usage Guide
Please check the [wiki](https://github.com/Qihoo360/TinyMCP/wiki) for more information.

## Dependencies
[jsoncpp 1.9.5](https://github.com/open-source-parsers/jsoncpp)

## MCP protocol version
[MCP Specification 2024-11-05](https://modelcontextprotocol.io/specification/2024-11-05/index)

## Code structure brief
| directory | brief |
| --- | --- |
| Public | SDK public directory, used to store global type definitions, global constant resources, helper classes, etc. |
| Message | Definition and implementation of the basic data structures of the MCP protocol, mainly including the implementation of protocol data such as standard requests, responses, and notifications that follow the JSON-RPC 2.0 protocol. |
| Session | Definition and implementation of MCP protocol interaction sessions, mainly including the implementation of stateful sessions based on the C/S architecture and the maintenance of the complete MCP session lifecycle (initialization phase, running phase, and closing phase). |
| Transport | Definition of data transmission channels between various endpoints based on the C/S architecture, which also includes an implementation of a default stdio transmission channel. |
| Task | Definition and implementation of various MCP tasks, such as the three - way handshake and capability negotiation task during the initialization phase, and the processing tasks for major requests like Prompts (not yet implemented), Resources (not yet implemented), Tools (already implemented), etc. Time - consuming tasks are executed asynchronously, while ordinary tasks are executed synchronously. |
| Entity | Definition of various protocol entity classes for MCP, mainly including the base class of the Server following the Singleton pattern. |

## Feature support brief
> Although not all MCP features have been implemented yet, it is already sufficient for the Tools-type MCP Server.

| Feature | Remarks | Supported or not |
| --- | --- | --- |
| Base Protocol | Core JSON-RPC message types | Yes |
| Lifecycle Management | Connection initialization, capability negotiation, and session control | Yes |
| Transports | stdio | Yes |
| Cancellation | Cancellation of in-progress requests through notification messages. | Yes |
| Progress | Progress tracking for long-running operations through notification messages. | Yes |
| Tools | Tools enable models to interact with external systems, such as querying databases, calling APIs, or performing computations. | Yes |
| Pagination | Pagination allows servers to yield results in smaller chunks rather than all at once. | Yes |
| Ping | Ping mechanism that allows either party to verify that their counterpart is still responsive and the connection is alive. | Yes |
| Custom Notifications | JSON-RPC 2.0 notifications that are sent by an MCP Server or Client using custom method names, intended for transmitting arbitrary business data. | Yes |
| Transports | HTTP with Server-Sent Events (SSE) | Not yet |
| Resources | Resources allow servers to share data that provides context to language models, such as files, database schemas, or application-specific information. | Not yet |
| Prompts | Prompts allow servers to provide structured messages and instructions for interacting with language models. | Not yet |
| Completion | A standardized way for servers to offer argument autocompletion suggestions for prompts and resource URIs. | Not yet |
| Logging | A standardized way for servers to send structured log messages to clients. | Not yet |

## LICENSE
TinyMCP is licensed under the MIT License - see the LICENSE file for details.
