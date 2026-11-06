#pragma once
// End-to-end client: launches a real MCP server process and exchanges JSON-RPC
// over its stdio pipes. The client deliberately depends only on jsoncpp and the
// C++ standard library, so a defect shared with the SDK parser cannot hide.

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace MCP
{
	namespace Test
	{
		// Resolves and stores the absolute path used to spawn the server process.
		void SetExecutablePath(const char* pszPath);
		const std::string& GetExecutablePath();

		class CStdioClient
		{
		public:
			CStdioClient() = default;
			~CStdioClient();

			CStdioClient(const CStdioClient&) = delete;
			CStdioClient& operator=(const CStdioClient&) = delete;

			int Start(bool bLegacyOnly);
			int Stop(int iExpectedExitCode = 0);

			int Send(const std::string& strLine);

			size_t GetOutputCount() const;
			std::string GetOutput(size_t nIndex) const;
			bool WaitForOutputCount(size_t nCount, int iTimeoutMs = 5000);
			bool WaitForId(int iId, size_t nStartIndex, size_t& nIndex, int iTimeoutMs = 5000);
			bool WaitForMethod(const std::string& strMethod, size_t nStartIndex, size_t& nIndex, int iTimeoutMs = 5000);
			bool WaitForSilence(size_t nExpectedCount, int iMilliseconds);

		private:
			void ReaderLoop();
			void AppendChunk(const char* pData, size_t nSize);
			int WaitForExit(int iTimeoutMs);
			int CloseHandles();

#ifdef _WIN32
			void* m_hProcess{ nullptr };
			void* m_hStdinWrite{ nullptr };
			void* m_hStdoutRead{ nullptr };
#else
			int m_nStdinWrite{ -1 };
			int m_nStdoutRead{ -1 };
			int m_nPid{ -1 };
#endif
			std::thread m_threadReader;
			mutable std::mutex m_mtx;
			std::condition_variable m_cv;
			std::vector<std::string> m_vecOutput;
			std::string m_strPending;
			bool m_bStarted{ false };
			bool m_bEof{ false };
		};
	}
}
