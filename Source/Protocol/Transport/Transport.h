#pragma once
// To ensure good cross-platform compatibility, all code within the MCP namespace is written in standard C++.
// The use of platform-specific system APIs is prohibited unless necessary.

#include <functional>
#include <string>
#include <mutex>
#include <utility>
#include "../Public/PublicDef.h"

namespace MCP
{
	class CMCPTransport
	{
	public:
		virtual ~CMCPTransport(){}

		virtual int Connect() = 0;
		virtual int Disconnect() = 0;
		virtual int Read(std::string& strOut) = 0;
		virtual int Write(const std::string& strIn) = 0;
		virtual int Error(const std::string& strIn) = 0;

		// Stateless requests only need a line-oriented message stream. Transports
		// that carry MCP inside a session-based channel (for example HTTP+SSE)
		// report false so the server can refuse stateless revisions up front.
		virtual bool SupportsStatelessProtocol() const { return true; }

		// Hook invoked on the transport's reader thread when the current client's
		// channel has closed and the next message may belong to a new client. The
		// session registers its per-connection reset here, so transports never need
		// to know about MCP session state.
		void SetOnClientDisconnected(std::function<void()> fnOnClientDisconnected)
		{
			m_fnOnClientDisconnected = std::move(fnOnClientDisconnected);
		}

	protected:
		void NotifyClientDisconnected() const
		{
			if (m_fnOnClientDisconnected)
				m_fnOnClientDisconnected();
		}

	private:
		std::function<void()> m_fnOnClientDisconnected;
	};

	class CStdioTransport : public CMCPTransport
	{
	public:
		int Connect() override;
		int Disconnect() override;
		int Read(std::string& strOut) override;
		int Write(const std::string& strIn) override;
		int Error(const std::string& strIn) override;

	private:
		std::recursive_mutex m_mtxStdin;
		std::recursive_mutex m_mtxStdout;
		std::recursive_mutex m_mtxStderr;
	};
}
