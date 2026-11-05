#include "MemoryTransport.h"

#include <chrono>

namespace MCP
{
	namespace Test
	{
		int CMemoryTransport::Connect()
		{
			{
				const std::lock_guard<std::mutex> _lock(m_mtx);
				m_bConnected = true;
			}
			m_cv.notify_all();

			return ERRNO_OK;
		}

		int CMemoryTransport::Disconnect()
		{
			{
				const std::lock_guard<std::mutex> _lock(m_mtx);
				m_bConnected = false;
				m_bInputClosed = true;
			}
			m_cv.notify_all();

			return ERRNO_OK;
		}

		int CMemoryTransport::Read(std::string& strOut)
		{
			std::unique_lock<std::mutex> _lock(m_mtx);
			m_cv.wait(_lock, [this]() {
				return !m_deqInput.empty() || m_bInputClosed;
			});

			if (m_deqInput.empty())
				return ERRNO_INTERNAL_INPUT_TERMINATE;

			strOut = m_deqInput.front();
			m_deqInput.pop_front();

			return ERRNO_OK;
		}

		int CMemoryTransport::Write(const std::string& strIn)
		{
			{
				const std::lock_guard<std::mutex> _lock(m_mtx);
				m_vecOutput.push_back(strIn);
			}
			m_cv.notify_all();

			return ERRNO_OK;
		}

		int CMemoryTransport::Error(const std::string& strIn)
		{
			return ERRNO_OK;
		}

		void CMemoryTransport::PushInput(const std::string& strLine)
		{
			{
				const std::lock_guard<std::mutex> _lock(m_mtx);
				if (m_bInputClosed)
					return;
				m_deqInput.push_back(strLine);
			}
			m_cv.notify_all();
		}

		void CMemoryTransport::CloseInput()
		{
			{
				const std::lock_guard<std::mutex> _lock(m_mtx);
				m_bInputClosed = true;
			}
			m_cv.notify_all();
		}

		bool CMemoryTransport::WaitForConnected(int iTimeoutMs)
		{
			std::unique_lock<std::mutex> _lock(m_mtx);
			return m_cv.wait_for(_lock, std::chrono::milliseconds(iTimeoutMs), [this]() {
				return m_bConnected;
			});
		}

		bool CMemoryTransport::WaitForOutputCount(size_t nCount, int iTimeoutMs)
		{
			std::unique_lock<std::mutex> _lock(m_mtx);
			return m_cv.wait_for(_lock, std::chrono::milliseconds(iTimeoutMs), [this, nCount]() {
				return m_vecOutput.size() >= nCount;
			});
		}

		size_t CMemoryTransport::GetOutputCount() const
		{
			const std::lock_guard<std::mutex> _lock(m_mtx);
			return m_vecOutput.size();
		}

		std::string CMemoryTransport::GetOutput(size_t nIndex) const
		{
			const std::lock_guard<std::mutex> _lock(m_mtx);
			if (nIndex >= m_vecOutput.size())
				return std::string();

			return m_vecOutput[nIndex];
		}
	}
}
