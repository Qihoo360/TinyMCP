#pragma once
// Specific business modules, which are not part of the MCP namespace, may adopt platform-specific APIs.

#include "../../Protocol/Transport/Transport.h"
#include <Windows.h>

namespace Implementation
{
	class CNonConsoleStdioTransport : public MCP::CMCPTransport
	{
	public:
		static constexpr unsigned long TRANSPORT_BUFFER_LEN = 4096;

		int Connect() override;
		int Disconnect() override;
		int Read(std::string& strOut) override;
		int Write(const std::string& strIn) override;
		int Error(const std::string& strIn) override;

	private:
		HANDLE m_hStdIn{ NULL };
		HANDLE m_hStdOut{ NULL };
		HANDLE m_hStdErr{ NULL };

		std::recursive_mutex m_mtxStdin;
		std::recursive_mutex m_mtxStdout;
		std::recursive_mutex m_mtxStderr;
	};
}