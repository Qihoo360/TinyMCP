#pragma once
// Drives a test server over a memory transport: sets the transport, starts the
// server loop on a background thread, sends request lines and waits for output.

#include "MemoryTransport.h"

#include <memory>
#include <string>
#include <thread>

namespace MCP
{
	namespace Test
	{
		class CDriver
		{
		public:
			CDriver() = default;
			~CDriver();

			CDriver(const CDriver&) = delete;
			CDriver& operator=(const CDriver&) = delete;

			int Start(int iTimeoutMs = 5000, bool bStatelessTransport = true);
			int Stop();

			int Send(const std::string& strLine);
			bool WaitForOutputCount(size_t nCount, int iTimeoutMs = 5000);

			size_t GetOutputCount() const;
			std::string GetOutput(size_t nIndex) const;
			std::shared_ptr<CMemoryTransport> GetTransport() const;

		private:
			std::shared_ptr<CMemoryTransport> m_spTransport;
			std::unique_ptr<std::thread> m_upThread;
			bool m_bStarted{ false };
		};
	}
}
