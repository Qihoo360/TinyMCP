#include "StdioClient.h"

#include <Public/PublicDef.h>

#include <json/json.h>

#include <chrono>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace MCP
{
	namespace Test
	{
		namespace
		{
			std::string s_strExecutablePath;
		}

		void SetExecutablePath(const char* pszPath)
		{
			char szBuffer[4096] = { 0 };

#ifdef _WIN32
			DWORD nLength = GetModuleFileNameA(nullptr, szBuffer, sizeof(szBuffer));
			if (nLength > 0 && nLength < sizeof(szBuffer))
				s_strExecutablePath.assign(szBuffer, static_cast<size_t>(nLength));
#elif defined(__linux__)
			ssize_t nLength = readlink("/proc/self/exe", szBuffer, sizeof(szBuffer) - 1);
			if (nLength > 0)
				s_strExecutablePath.assign(szBuffer, static_cast<size_t>(nLength));
#endif

			if (s_strExecutablePath.empty() && pszPath)
				s_strExecutablePath = pszPath;
		}

		const std::string& GetExecutablePath()
		{
			return s_strExecutablePath;
		}

		CStdioClient::~CStdioClient()
		{
			Stop(0);
		}

		int CStdioClient::Start(bool bLegacyOnly)
		{
			if (m_bStarted)
				return ERRNO_OK;

			if (s_strExecutablePath.empty())
				return ERRNO_INTERNAL_ERROR;

#ifdef _WIN32
			SECURITY_ATTRIBUTES sa{};
			sa.nLength = sizeof(sa);
			sa.bInheritHandle = TRUE;

			HANDLE hStdinRead = nullptr;
			HANDLE hStdinWrite = nullptr;
			HANDLE hStdoutRead = nullptr;
			HANDLE hStdoutWrite = nullptr;

			if (!CreatePipe(&hStdinRead, &hStdinWrite, &sa, 0))
				return ERRNO_INTERNAL_ERROR;

			if (!CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0))
			{
				CloseHandle(hStdinRead);
				CloseHandle(hStdinWrite);
				return ERRNO_INTERNAL_ERROR;
			}

			SetHandleInformation(hStdinWrite, HANDLE_FLAG_INHERIT, 0);
			SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0);

			HANDLE hNull = CreateFileA("NUL", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
				&sa, OPEN_EXISTING, 0, nullptr);
			if (INVALID_HANDLE_VALUE == hNull)
				hNull = nullptr;

			STARTUPINFOA si{};
			si.cb = sizeof(si);
			si.dwFlags = STARTF_USESTDHANDLES;
			si.hStdInput = hStdinRead;
			si.hStdOutput = hStdoutWrite;
			si.hStdError = (hNull ? hNull : GetStdHandle(STD_ERROR_HANDLE));

			std::string strCommand = "\"" + s_strExecutablePath + "\" --server";
			if (bLegacyOnly)
				strCommand += " --legacy-only";

			std::vector<char> vecCommand(strCommand.begin(), strCommand.end());
			vecCommand.push_back('\0');

			PROCESS_INFORMATION pi{};
			BOOL bCreated = CreateProcessA(nullptr, vecCommand.data(), nullptr, nullptr, TRUE,
				CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);

			CloseHandle(hStdinRead);
			CloseHandle(hStdoutWrite);
			if (hNull)
				CloseHandle(hNull);

			if (!bCreated)
			{
				CloseHandle(hStdinWrite);
				CloseHandle(hStdoutRead);
				return ERRNO_INTERNAL_ERROR;
			}

			CloseHandle(pi.hThread);
			m_hProcess = pi.hProcess;
			m_hStdinWrite = hStdinWrite;
			m_hStdoutRead = hStdoutRead;
#else
			int vecStdin[2] = { -1, -1 };
			int vecStdout[2] = { -1, -1 };

			if (0 != pipe(vecStdin) || 0 != pipe(vecStdout))
				return ERRNO_INTERNAL_ERROR;

			pid_t nPid = fork();
			if (nPid < 0)
				return ERRNO_INTERNAL_ERROR;

			if (0 == nPid)
			{
				dup2(vecStdin[0], STDIN_FILENO);
				dup2(vecStdout[1], STDOUT_FILENO);

				int nDevNull = open("/dev/null", O_WRONLY);
				if (nDevNull >= 0)
				{
					dup2(nDevNull, STDERR_FILENO);
					close(nDevNull);
				}

				close(vecStdin[0]);
				close(vecStdin[1]);
				close(vecStdout[0]);
				close(vecStdout[1]);

				char* pszArgs[4] = { nullptr, nullptr, nullptr, nullptr };
				int nArgs = 0;
				pszArgs[nArgs++] = const_cast<char*>(s_strExecutablePath.c_str());
				pszArgs[nArgs++] = const_cast<char*>("--server");
				if (bLegacyOnly)
					pszArgs[nArgs++] = const_cast<char*>("--legacy-only");

				execv(s_strExecutablePath.c_str(), pszArgs);
				_exit(127);
			}

			close(vecStdin[0]);
			close(vecStdout[1]);

			m_nStdinWrite = vecStdin[1];
			m_nStdoutRead = vecStdout[0];
			m_nPid = static_cast<int>(nPid);
#endif

			m_bStarted = true;
			m_threadReader = std::thread(&CStdioClient::ReaderLoop, this);

			return ERRNO_OK;
		}

		int CStdioClient::Stop(int iExpectedExitCode)
		{
			if (!m_bStarted)
				return ERRNO_OK;

#ifdef _WIN32
			if (m_hStdinWrite)
			{
				CloseHandle((HANDLE)m_hStdinWrite);
				m_hStdinWrite = nullptr;
			}
#else
			if (m_nStdinWrite >= 0)
			{
				close(m_nStdinWrite);
				m_nStdinWrite = -1;
			}
#endif

			int iExitCode = WaitForExit(10000);

			if (m_threadReader.joinable())
				m_threadReader.join();

			CloseHandles();
			m_bStarted = false;

			return (iExitCode == iExpectedExitCode) ? ERRNO_OK : ERRNO_INTERNAL_ERROR;
		}

		int CStdioClient::Send(const std::string& strLine)
		{
			if (!m_bStarted)
				return ERRNO_INTERNAL_ERROR;

			std::string strMessage = strLine + "\n";

#ifdef _WIN32
			const char* pData = strMessage.data();
			size_t nRemaining = strMessage.size();
			while (nRemaining > 0)
			{
				DWORD nWritten = 0;
				if (!WriteFile((HANDLE)m_hStdinWrite, pData, static_cast<DWORD>(nRemaining), &nWritten, nullptr)
					|| 0 == nWritten)
				{
					return ERRNO_INTERNAL_ERROR;
				}

				pData += nWritten;
				nRemaining -= nWritten;
			}
#else
			size_t nOffset = 0;
			while (nOffset < strMessage.size())
			{
				ssize_t nWritten = write(m_nStdinWrite, strMessage.data() + nOffset, strMessage.size() - nOffset);
				if (nWritten <= 0)
					return ERRNO_INTERNAL_ERROR;

				nOffset += static_cast<size_t>(nWritten);
			}
#endif

			return ERRNO_OK;
		}

		size_t CStdioClient::GetOutputCount() const
		{
			std::lock_guard<std::mutex> _lock(m_mtx);
			return m_vecOutput.size();
		}

		std::string CStdioClient::GetOutput(size_t nIndex) const
		{
			std::lock_guard<std::mutex> _lock(m_mtx);
			if (nIndex >= m_vecOutput.size())
				return std::string();

			return m_vecOutput[nIndex];
		}

		bool CStdioClient::WaitForOutputCount(size_t nCount, int iTimeoutMs)
		{
			std::unique_lock<std::mutex> lock(m_mtx);
			if (m_vecOutput.size() >= nCount)
				return true;

			return m_cv.wait_for(lock, std::chrono::milliseconds(iTimeoutMs), [this, nCount]()
			{
				return m_vecOutput.size() >= nCount || m_bEof;
			}) && m_vecOutput.size() >= nCount;
		}

		bool CStdioClient::WaitForId(int iId, size_t nStartIndex, size_t& nIndex, int iTimeoutMs)
		{
			std::unique_lock<std::mutex> lock(m_mtx);
			auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(iTimeoutMs);

			while (true)
			{
				for (size_t i = nStartIndex; i < m_vecOutput.size(); ++i)
				{
					Json::Reader reader;
					Json::Value jMessage;
					if (reader.parse(m_vecOutput[i], jMessage) && jMessage.isObject()
						&& jMessage.isMember(MSG_KEY_ID) && jMessage[MSG_KEY_ID].isInt()
						&& jMessage[MSG_KEY_ID].asInt() == iId)
					{
						nIndex = i;
						return true;
					}
				}

				if (m_bEof)
					return false;

				if (std::cv_status::timeout == m_cv.wait_until(lock, deadline))
					return false;
			}
		}

		bool CStdioClient::WaitForMethod(const std::string& strMethod, size_t nStartIndex, size_t& nIndex, int iTimeoutMs)
		{
			std::unique_lock<std::mutex> lock(m_mtx);
			auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(iTimeoutMs);

			while (true)
			{
				for (size_t i = nStartIndex; i < m_vecOutput.size(); ++i)
				{
					Json::Reader reader;
					Json::Value jMessage;
					if (reader.parse(m_vecOutput[i], jMessage) && jMessage.isObject()
						&& jMessage.isMember(MSG_KEY_METHOD)
						&& jMessage[MSG_KEY_METHOD].asString() == strMethod)
					{
						nIndex = i;
						return true;
					}
				}

				if (m_bEof)
					return false;

				if (std::cv_status::timeout == m_cv.wait_until(lock, deadline))
					return false;
			}
		}

		bool CStdioClient::WaitForSilence(size_t nExpectedCount, int iMilliseconds)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(iMilliseconds));

			std::lock_guard<std::mutex> _lock(m_mtx);
			return m_vecOutput.size() == nExpectedCount;
		}

		void CStdioClient::ReaderLoop()
		{
			std::vector<char> vecBuffer(4096);

			while (true)
			{
				bool bHasData = false;
				size_t nRead = 0;

#ifdef _WIN32
				DWORD nBytes = 0;
				if (ReadFile((HANDLE)m_hStdoutRead, vecBuffer.data(), static_cast<DWORD>(vecBuffer.size()), &nBytes, nullptr)
					&& nBytes > 0)
				{
					bHasData = true;
					nRead = static_cast<size_t>(nBytes);
				}
#else
				ssize_t nBytes = read(m_nStdoutRead, vecBuffer.data(), vecBuffer.size());
				if (nBytes > 0)
				{
					bHasData = true;
					nRead = static_cast<size_t>(nBytes);
				}
#endif

				if (!bHasData)
					break;

				AppendChunk(vecBuffer.data(), nRead);
			}

			{
				std::lock_guard<std::mutex> _lock(m_mtx);
				m_bEof = true;
			}
			m_cv.notify_all();
		}

		void CStdioClient::AppendChunk(const char* pData, size_t nSize)
		{
			std::lock_guard<std::mutex> _lock(m_mtx);
			m_strPending.append(pData, nSize);

			size_t nPosition = 0;
			while (std::string::npos != (nPosition = m_strPending.find('\n')))
			{
				std::string strLine = m_strPending.substr(0, nPosition);
				m_strPending.erase(0, nPosition + 1);

				if (!strLine.empty() && '\r' == strLine.back())
					strLine.pop_back();

				if (!strLine.empty())
					m_vecOutput.push_back(strLine);
			}

			m_cv.notify_all();
		}

		int CStdioClient::WaitForExit(int iTimeoutMs)
		{
#ifdef _WIN32
			DWORD dwResult = WaitForSingleObject((HANDLE)m_hProcess, static_cast<DWORD>(iTimeoutMs));
			if (WAIT_TIMEOUT == dwResult)
			{
				TerminateProcess((HANDLE)m_hProcess, 1);
				WaitForSingleObject((HANDLE)m_hProcess, 5000);
			}

			DWORD dwExitCode = 1;
			GetExitCodeProcess((HANDLE)m_hProcess, &dwExitCode);
			return static_cast<int>(dwExitCode);
#else
			auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(iTimeoutMs);

			while (true)
			{
				int nStatus = 0;
				pid_t nResult = waitpid(m_nPid, &nStatus, WNOHANG);
				if (nResult == m_nPid)
				{
					m_nPid = -1;
					if (WIFEXITED(nStatus))
						return WEXITSTATUS(nStatus);
					if (WIFSIGNALED(nStatus))
						return 128 + WTERMSIG(nStatus);

					return 1;
				}

				if (nResult < 0)
					return 1;

				if (std::chrono::steady_clock::now() >= deadline)
				{
					kill(m_nPid, SIGKILL);
					waitpid(m_nPid, &nStatus, 0);
					m_nPid = -1;
					return 137;
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
#endif
		}

		int CStdioClient::CloseHandles()
		{
#ifdef _WIN32
			if (m_hStdoutRead)
			{
				CloseHandle((HANDLE)m_hStdoutRead);
				m_hStdoutRead = nullptr;
			}

			if (m_hProcess)
			{
				CloseHandle((HANDLE)m_hProcess);
				m_hProcess = nullptr;
			}
#else
			if (m_nStdoutRead >= 0)
			{
				close(m_nStdoutRead);
				m_nStdoutRead = -1;
			}
#endif

			return ERRNO_OK;
		}
	}
}
