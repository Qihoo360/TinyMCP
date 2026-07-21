#pragma once
// HTTP with Server-Sent Events (SSE) transport for MCP 2024-11-05.
// The header intentionally hides cpp-httplib types so that user code does not need
// to expose the underlying HTTP dependency. The implementation lives in SSETransport.cpp.

#include <string>
#include <vector>
#include <deque>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <thread>
#include "../Public/PublicDef.h"
#include "Transport.h"

namespace MCP
{
	class CSSETransport : public CMCPTransport
	{
	public:
		CSSETransport();
		CSSETransport(const std::string& strHost, int iPort);
		CSSETransport(const std::string& strHost, int iPort, const std::string& strSseEndpoint, const std::string& strMessageEndpoint);
		~CSSETransport() override;

		// Configure origin whitelist for DNS-rebinding protection.
		// An empty Origin header is treated as allowed (e.g. same-origin curl / MCP Inspector).
		// Otherwise the request's Origin must start with one of the configured prefixes.
		// Defaults: "http://localhost", "http://127.0.0.1".
		void SetAllowedOrigins(const std::vector<std::string>& vecOrigins);

		int Connect() override;
		int Disconnect() override;
		int Read(std::string& strOut) override;
		int Write(const std::string& strIn) override;
		int Error(const std::string& strIn) override;

	private:
		// Non-copyable
		CSSETransport(const CSSETransport&) = delete;
		CSSETransport& operator=(const CSSETransport&) = delete;

		// Origin whitelist check. Returns true if strOrigin is empty or prefix-matches any allowed origin.
		bool IsOriginAllowed(const std::string& strOrigin) const;

		// Generate a random session id used for the /messages endpoint URI.
		static std::string GenerateSessionId();

		std::string m_strHost;
		int m_iPort;
		std::string m_strSseEndpoint;
		std::string m_strMessageEndpoint;
		std::string m_strSessionId;
		std::vector<std::string> m_vecAllowedOrigins;

		// Opaque pimpl holding the httplib::Server instance so we do not leak httplib types
		// through the public header.
		struct ServerHolder;
		std::unique_ptr<ServerHolder> m_upServerHolder;
		std::unique_ptr<std::thread> m_upServerThread;

		// Lifecycle flag. Read() returns termination when this becomes false.
		std::atomic_bool m_bRunning{ false };

		// Inbound queue: HTTP POST /messages pushes bodies here; Read() pops them.
		std::deque<std::string> m_deqIncoming;
		std::mutex m_mtxIncoming;
		std::condition_variable m_cvIncoming;

		// Outbound queue: Write() pushes JSON-RPC messages here; the SSE chunked provider drains them.
		std::deque<std::string> m_deqOutgoing;
		std::mutex m_mtxOutgoing;
		std::condition_variable m_cvOutgoing;

		// Single-session gate. Only one active SSE stream is served; the second GET /sse returns 409.
		std::atomic_bool m_bClientConnected{ false };
		std::atomic_bool m_bEndpointSent{ false };

		// Set by the SSE releaser when a client's stream terminates, consumed by Read() in
		// the Session::Run() thread to perform MCP session-level reset (state machine,
		// async task thread, cached message history) without cross-thread mutation of
		// CMCPSession's private data.
		std::atomic_bool m_bResetPending{ false };
	};
}
