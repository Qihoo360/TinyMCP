#pragma once
// In-memory transport used by integration tests. Input lines are queued by the
// test driver and output lines are captured so they can be asserted byte by byte.

#include <Transport/Transport.h>

#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <vector>

namespace MCP
{
	namespace Test
	{
		class CMemoryTransport : public MCP::CMCPTransport
		{
		public:
			CMemoryTransport() = default;
			~CMemoryTransport() = default;

			int Connect() override;
			int Disconnect() override;
			int Read(std::string& strOut) override;
			int Write(const std::string& strIn) override;
			int Error(const std::string& strIn) override;

			void PushInput(const std::string& strLine);
			void CloseInput();

			bool WaitForConnected(int iTimeoutMs);
			bool WaitForOutputCount(size_t nCount, int iTimeoutMs);

			size_t GetOutputCount() const;
			std::string GetOutput(size_t nIndex) const;

		private:
			mutable std::mutex m_mtx;
			std::condition_variable m_cv;
			std::deque<std::string> m_deqInput;
			std::vector<std::string> m_vecOutput;
			bool m_bInputClosed{ false };
			bool m_bConnected{ false };
		};
	}
}
