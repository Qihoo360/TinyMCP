#include "SSETransport.h"
#include "../Public/PublicDef.h"
#include "../Session/Session.h"

// cpp-httplib is a header-only HTTP library shipped inside the repository at
// Source/External/cpp-httplib/httplib.h. It is intentionally included only inside
// this .cpp so the SDK public header does not leak httplib types.
#include <httplib.h>

#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace MCP
{
	// PImpl body: keep the httplib::Server instance out of the public header.
	struct CSSETransport::ServerHolder
	{
		httplib::Server server;
	};

	CSSETransport::CSSETransport()
		: CSSETransport(std::string("127.0.0.1"), 8080, std::string("/sse"), std::string("/messages"))
	{
	}

	CSSETransport::CSSETransport(const std::string& strHost, int iPort)
		: CSSETransport(strHost, iPort, std::string("/sse"), std::string("/messages"))
	{
	}

	CSSETransport::CSSETransport(const std::string& strHost, int iPort, const std::string& strSseEndpoint, const std::string& strMessageEndpoint)
		: m_strHost(strHost)
		, m_iPort(iPort)
		, m_strSseEndpoint(strSseEndpoint)
		, m_strMessageEndpoint(strMessageEndpoint)
	{
		// Default DNS-rebinding whitelist covers the two local-loopback origins recommended by the spec.
		m_vecAllowedOrigins.emplace_back("http://localhost");
		m_vecAllowedOrigins.emplace_back("http://127.0.0.1");
	}

	CSSETransport::~CSSETransport()
	{
		// Ensure background HTTP thread is joined and httplib::Server is stopped.
		Disconnect();
	}

	void CSSETransport::SetAllowedOrigins(const std::vector<std::string>& vecOrigins)
	{
		m_vecAllowedOrigins = vecOrigins;
	}

	bool CSSETransport::IsOriginAllowed(const std::string& strOrigin) const
	{
		if (strOrigin.empty())
			return true;

		for (const auto& strAllowed : m_vecAllowedOrigins)
		{
			if (strAllowed.empty())
				continue;
			if (strOrigin.compare(0, strAllowed.size(), strAllowed) == 0)
				return true;
		}
		return false;
	}

	std::string CSSETransport::GenerateSessionId()
	{
		// 128-bit hex session id (32 hex chars). Non-cryptographic randomness is sufficient here
		// because Origin whitelist and same-process session gating are the actual security barriers.
		std::random_device rd;
		std::mt19937_64 gen(static_cast<uint64_t>(rd()) ^ static_cast<uint64_t>(
			std::chrono::high_resolution_clock::now().time_since_epoch().count()));
		std::uniform_int_distribution<uint64_t> dist;

		uint64_t lo = dist(gen);
		uint64_t hi = dist(gen);

		std::ostringstream oss;
		oss << std::hex << std::setfill('0') << std::setw(16) << hi << std::setw(16) << lo;
		return oss.str();
	}

	int CSSETransport::Connect()
	{
		if (m_bRunning.load())
			return ERRNO_INTERNAL_ERROR;

		m_strSessionId = GenerateSessionId();
		m_bClientConnected.store(false);
		m_bEndpointSent.store(false);
		{
			std::lock_guard<std::mutex> _lock(m_mtxIncoming);
			m_deqIncoming.clear();
		}
		{
			std::lock_guard<std::mutex> _lock(m_mtxOutgoing);
			m_deqOutgoing.clear();
		}

		m_upServerHolder = std::unique_ptr<ServerHolder>(new ServerHolder());
		auto& srv = m_upServerHolder->server;

		// GET /sse -> long-lived Server-Sent Events stream. Single-session gate returns 409
		// if a client is already connected.
		srv.Get(m_strSseEndpoint.c_str(), [this](const httplib::Request& req, httplib::Response& res)
		{
			std::string strOrigin;
			if (req.has_header("Origin"))
				strOrigin = req.get_header_value("Origin");
			if (!IsOriginAllowed(strOrigin))
			{
				res.status = 403;
				res.set_content("forbidden origin", "text/plain");
				return;
			}

			bool bExpected = false;
			if (!m_bClientConnected.compare_exchange_strong(bExpected, true))
			{
				res.status = 409;
				res.set_content("another SSE client already connected", "text/plain");
				return;
			}

			res.set_header("Cache-Control", "no-cache");
			res.set_header("Connection", "keep-alive");
			res.set_header("Access-Control-Allow-Origin", strOrigin.empty() ? std::string("*") : strOrigin);

			// The chunked content provider is driven by cpp-httplib on a per-connection worker thread.
			// It must not capture `this` beyond the transport's lifetime; Disconnect() joins the listen
			// thread which in turn tears down all active connections before returning.
			res.set_chunked_content_provider("text/event-stream",
				[this](size_t /*offset*/, httplib::DataSink& sink) -> bool
				{
					if (!m_bRunning.load())
					{
						sink.done();
						return false;
					}

					// First frame per MCP 2024-11-05: event: endpoint, data: <messageEndpoint>?sessionId=<id>.
					// The path is relative; MCP Inspector / browsers resolve it against the SSE origin.
					if (!m_bEndpointSent.load())
					{
						std::string strEndpointUri = m_strMessageEndpoint + "?sessionId=" + m_strSessionId;
						std::string strFrame = "event: endpoint\ndata: " + strEndpointUri + "\n\n";
						if (!sink.write(strFrame.data(), strFrame.size()))
							return false;
						m_bEndpointSent.store(true);
					}

					// Drain outbound queue. Poll the peer-alive flag on a short tick so we can
					// detect a client disconnect promptly (otherwise we would only notice on the
					// next sink.write, which may be up to 15s away when the queue is idle).
					// We still emit an SSE comment keep-alive every ~15s so intermediaries do
					// not close idle connections.
					std::vector<std::string> vecPending;
					bool bKeepAliveDue = false;
					{
						std::unique_lock<std::mutex> _lock(m_mtxOutgoing);
						auto tpDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(15);
						while (m_bRunning.load() && m_deqOutgoing.empty())
						{
							if (!sink.is_writable())
								return false;
							if (std::chrono::steady_clock::now() >= tpDeadline)
							{
								bKeepAliveDue = true;
								break;
							}
							m_cvOutgoing.wait_for(_lock, std::chrono::seconds(1), [this]()
							{
								return !m_deqOutgoing.empty() || !m_bRunning.load();
							});
						}

						while (!m_deqOutgoing.empty())
						{
							vecPending.push_back(std::move(m_deqOutgoing.front()));
							m_deqOutgoing.pop_front();
						}
					}

					if (!m_bRunning.load())
					{
						sink.done();
						return false;
					}
					if (!sink.is_writable())
						return false;

					if (vecPending.empty())
					{
						if (!bKeepAliveDue)
							return true;
						// SSE comment keep-alive per WHATWG spec: lines starting with ':' are ignored by the client.
						static const char szKeepAlive[] = ": keep-alive\n\n";
						if (!sink.write(szKeepAlive, sizeof(szKeepAlive) - 1))
							return false;
						return true;
					}

					for (const auto& strMsg : vecPending)
					{
						// Write() guarantees every queue element ends with exactly one '\n'.
						// SSE forbids raw LF inside a "data:" line (each LF starts a new data
						// field), so we cannot inline that '\n'. To make the client observe
						// event.data == "<json>\n" -- honoring the MCP requirement that every
						// interchange payload is LF-terminated -- emit the JSON on one data
						// line followed by an empty data line: the SSE spec joins consecutive
						// data fields with '\n', yielding "<json>\n" as the final data buffer.
						// Defensive strip of trailing CR/LF handles any unnormalized caller.
						std::string strData = strMsg;
						while (!strData.empty() && (strData.back() == '\n' || strData.back() == '\r'))
							strData.pop_back();

						std::string strFrame = "event: message\ndata: " + strData + "\ndata: \n\n";
						if (!sink.write(strFrame.data(), strFrame.size()))
							return false;
					}

					return true;
				},
				[this](bool /*success*/)
				{
					// Releaser: invoked by cpp-httplib after the chunked stream terminates
					// (either provider returned false, the peer closed the TCP connection,
					// or Disconnect() stopped the server). Reset per-session state so a new
					// SSE client can reconnect cleanly instead of being permanently rejected
					// with 409. Drain both queues to avoid cross-session bleed-through: any
					// half-consumed inbound bytes or unsent outbound messages belonged to the
					// prior session and must not leak into the next one.
					{
						std::lock_guard<std::mutex> _lock(m_mtxIncoming);
						m_deqIncoming.clear();
					}
					{
						std::lock_guard<std::mutex> _lock(m_mtxOutgoing);
						m_deqOutgoing.clear();
					}
					// Rotate the session id so any late POST /messages?sessionId=<old>
					// from the departed client is rejected instead of being fed to the
					// next client's session by mistake.
					m_strSessionId = GenerateSessionId();
					m_bEndpointSent.store(false);
					m_bClientConnected.store(false);
					// Signal the Session::Run() thread (blocked in Read()) that the MCP
					// session-level state must be reset before serving the next client.
					// Doing the reset on the reader thread avoids racing with
					// CMCPSession's non-thread-safe internal state.
					m_bResetPending.store(true);
					// Wake any Read() waiter so it observes the drained state promptly.
					m_cvIncoming.notify_all();
				});
		});

		// POST /messages?sessionId=... -> feed a single JSON-RPC message into the inbound queue.
		srv.Post(m_strMessageEndpoint.c_str(), [this](const httplib::Request& req, httplib::Response& res)
		{
			std::string strOrigin;
			if (req.has_header("Origin"))
				strOrigin = req.get_header_value("Origin");
			if (!IsOriginAllowed(strOrigin))
			{
				res.status = 403;
				res.set_content("forbidden origin", "text/plain");
				return;
			}

			if (!req.has_param("sessionId") || req.get_param_value("sessionId") != m_strSessionId)
			{
				res.status = 404;
				res.set_content("session not found", "text/plain");
				return;
			}

			if (req.body.empty())
			{
				res.status = 400;
				res.set_content("empty body", "text/plain");
				return;
			}

			{
				std::lock_guard<std::mutex> _lock(m_mtxIncoming);
				m_deqIncoming.push_back(req.body);
			}
			m_cvIncoming.notify_one();

			res.status = 202;
			res.set_content("", "text/plain");
		});

		// Fail fast if the port cannot be bound instead of silently retrying forever.
		if (!srv.bind_to_port(m_strHost.c_str(), m_iPort))
		{
			m_upServerHolder.reset();
			return ERRNO_INTERNAL_ERROR;
		}

		m_bRunning.store(true);
		m_upServerThread = std::unique_ptr<std::thread>(new std::thread([this]()
		{
			// listen_after_bind() blocks until stop() is called on the httplib::Server instance.
			m_upServerHolder->server.listen_after_bind();
		}));

		return ERRNO_OK;
	}

	int CSSETransport::Disconnect()
	{
		if (!m_bRunning.exchange(false))
			return ERRNO_OK;

		// Wake any thread waiting in Read() or in the SSE provider so they can observe m_bRunning == false.
		m_cvIncoming.notify_all();
		m_cvOutgoing.notify_all();

		if (m_upServerHolder)
			m_upServerHolder->server.stop();

		if (m_upServerThread && m_upServerThread->joinable())
			m_upServerThread->join();
		m_upServerThread.reset();
		m_upServerHolder.reset();

		return ERRNO_OK;
	}

	int CSSETransport::Read(std::string& strOut)
	{
		for (;;)
		{
			std::unique_lock<std::mutex> _lock(m_mtxIncoming);
			m_cvIncoming.wait(_lock, [this]()
			{
				return !m_deqIncoming.empty() || !m_bRunning.load() || m_bResetPending.load();
			});

			if (!m_bRunning.load() && m_deqIncoming.empty())
				return ERRNO_INTERNAL_INPUT_TERMINATE;

			// If the previous client just disconnected, reset MCP session state before
			// handing the next message to Session::Run(). This runs on the reader thread
			// (single owner of CMCPSession's mutable state) so it does not race with
			// concurrent ProcessRequest/ProcessNotification calls.
			if (m_bResetPending.exchange(false))
			{
				_lock.unlock();
				CMCPSession::GetInstance().ResetForNewClient();
				// Loop back to re-check the incoming queue: the new client's first message
				// may already be sitting there, or we go back to waiting.
				continue;
			}

			strOut = std::move(m_deqIncoming.front());
			m_deqIncoming.pop_front();
			return ERRNO_OK;
		}
	}

	int CSSETransport::Write(const std::string& strIn)
	{
		if (!m_bRunning.load())
			return ERRNO_INTERNAL_OUTPUT_ERROR;

		std::string strNormalized = strIn;
		if (strNormalized.empty())
			return ERRNO_INTERNAL_OUTPUT_ERROR;

		{
			std::lock_guard<std::mutex> _lock(m_mtxOutgoing);
			m_deqOutgoing.push_back(std::move(strNormalized));
		}
		m_cvOutgoing.notify_one();
		return ERRNO_OK;
	}

	int CSSETransport::Error(const std::string& /*strIn*/)
	{
		// Diagnostic logging in the MCP model is delivered through logging/message notifications,
		// which reach the client via Write(). SSE stderr has no equivalent, matching CStdioTransport::Error.
		return ERRNO_OK;
	}
}
