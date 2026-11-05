#include "Driver.h"
#include "TestServer.h"

#include <Session/Session.h>

#include <exception>
#include <iostream>

namespace MCP
{
	namespace Test
	{
		CDriver::~CDriver()
		{
			Stop();
		}

		int CDriver::Start(int iTimeoutMs)
		{
			if (m_bStarted)
				return ERRNO_OK;

			m_spTransport = std::make_shared<CMemoryTransport>();
			if (!m_spTransport)
				return ERRNO_INTERNAL_ERROR;

			MCP::CMCPSession::GetInstance().SetTransport(m_spTransport);

			// CMCPServer::Start() performs Ready() (transport Connect) and then Run()
			// on this thread; the driver waits for the connection before sending data.
			// Exceptions escaping the server loop would call std::terminate and hide the
			// failure behind an abort dialog, so report them explicitly instead.
			m_upThread = std::make_unique<std::thread>([]() {
				try
				{
					MCP::Test::CTestServer::s_Instance.Start();
				}
				catch (const std::exception& e)
				{
					std::cout << "Server thread exception: " << e.what() << std::endl;
				}
				catch (...)
				{
					std::cout << "Server thread exception: unknown" << std::endl;
				}
			});
			if (!m_upThread)
				return ERRNO_INTERNAL_ERROR;

			m_bStarted = true;
			if (!m_spTransport->WaitForConnected(iTimeoutMs))
			{
				Stop();
				return ERRNO_INTERNAL_ERROR;
			}

			return ERRNO_OK;
		}

		int CDriver::Stop()
		{
			if (!m_bStarted)
				return ERRNO_OK;

			if (m_spTransport)
				m_spTransport->CloseInput();

			if (m_upThread && m_upThread->joinable())
				m_upThread->join();
			m_upThread.reset();

			MCP::CMCPSession::GetInstance().Terminate();
			m_bStarted = false;

			return ERRNO_OK;
		}

		int CDriver::Send(const std::string& strLine)
		{
			if (!m_spTransport)
				return ERRNO_INTERNAL_ERROR;

			m_spTransport->PushInput(strLine);

			return ERRNO_OK;
		}

		bool CDriver::WaitForOutputCount(size_t nCount, int iTimeoutMs)
		{
			if (!m_spTransport)
				return false;

			return m_spTransport->WaitForOutputCount(nCount, iTimeoutMs);
		}

		size_t CDriver::GetOutputCount() const
		{
			if (!m_spTransport)
				return 0;

			return m_spTransport->GetOutputCount();
		}

		std::string CDriver::GetOutput(size_t nIndex) const
		{
			if (!m_spTransport)
				return std::string();

			return m_spTransport->GetOutput(nIndex);
		}

		std::shared_ptr<CMemoryTransport> CDriver::GetTransport() const
		{
			return m_spTransport;
		}
	}
}
