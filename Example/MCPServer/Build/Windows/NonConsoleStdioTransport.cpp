#include "stdafx.h"
#include "NonConsoleStdioTransport.h"
#include "../../Protocol/Public/StringHelper.h"
#include <memory>
#include <atlstr.h>
#include <CppHelper/StringHelper.h>

namespace Implementation
{
	int CNonConsoleStdioTransport::Connect()
	{
		m_hStdIn = ::GetStdHandle(STD_INPUT_HANDLE);
		if (NULL == m_hStdIn || INVALID_HANDLE_VALUE == m_hStdIn)
			return MCP::ERRNO_INTERNAL_ERROR;
		m_hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
		if (NULL == m_hStdOut || INVALID_HANDLE_VALUE == m_hStdOut)
			return MCP::ERRNO_INTERNAL_ERROR;
		m_hStdErr = ::GetStdHandle(STD_ERROR_HANDLE);
		if (NULL == m_hStdErr || INVALID_HANDLE_VALUE == m_hStdErr)
			return MCP::ERRNO_INTERNAL_ERROR;

		return MCP::ERRNO_OK;
	}

	int CNonConsoleStdioTransport::Disconnect()
	{
		return MCP::ERRNO_OK;
	}

	int CNonConsoleStdioTransport::Read(std::string& strOut)
	{
		const std::lock_guard<std::recursive_mutex> _lock(m_mtxStdin);

		int iErrCode{ MCP::ERRNO_OK };

		CStringA Tmp;
		DWORD dwRead = 0;
		DWORD cbDataUsed = 0;
		while (true)
		{
			if (ReadFile(
				m_hStdIn,
				(LPBYTE)Tmp.GetBuffer(cbDataUsed + TRANSPORT_BUFFER_LEN) + cbDataUsed,
				TRANSPORT_BUFFER_LEN * sizeof(Tmp[0]),
				&dwRead, nullptr) == FALSE)
			{
				iErrCode = MCP::ERRNO_INTERNAL_INPUT_ERROR;
				break;
			}

			if (0 == dwRead)
			{
				iErrCode = MCP::ERRNO_INTERNAL_INPUT_TERMINATE;
				break;
			}

			cbDataUsed += dwRead;

			DWORD cbAvail{ 0 };
			if (!PeekNamedPipe(m_hStdIn, nullptr, 0, nullptr, &cbAvail, nullptr) || cbAvail == 0)
			{
				break;
			}
			else
			{
#ifdef DEBUG
				if (true)
#else
				if (g_bOutputLog)
#endif // DEBUG
				{
					CString strLog;
					strLog.Format(L"[%s] >>>>>>receive more data from cbAvail=%u\r\n", __FUNCTIONW__, cbAvail);
					OutputDebugString(strLog.GetString());
				}

			}
		}

		Tmp.ReleaseBufferSetLength(cbDataUsed);
		strOut = Tmp;

#ifdef _DEBUG
		if (true)
#else
		if (g_bOutputLog)
#endif // _DEBUG
		{
			CString strLog;
			strLog.Format(L"[%s] >>>>>>receive data from client=%s\r\n", __FUNCTIONW__, YY::UTF8ToUnicode(strOut.c_str()));
			OutputDebugString(strLog.GetString());
		}

		return iErrCode;
	}

	int CNonConsoleStdioTransport::Write(const std::string& strIn)
	{
		std::string strWrite = strIn;

#ifdef _WIN32
		if (strWrite.rfind("\r\n") != strWrite.length() - 2)
		{
			strWrite.append("\r\n");
		}
#else
		if (strWrite.rfind("\n") != strWrite.length() - 1)
		{
			strWrite.append("\n");
		}
#endif // _WIN32

		const std::lock_guard<std::recursive_mutex> _lock(m_mtxStdout);

		DWORD cbWrite{ 0 };
		if (!WriteFile(m_hStdOut, strWrite.c_str(), strlen(strWrite.c_str()) * sizeof(char), &cbWrite, nullptr))
			return MCP::ERRNO_INTERNAL_OUTPUT_ERROR;

#ifdef _DEBUG
		if (true)
#else
		if (g_bOutputLog)
#endif // _DEBUG
		{
			CString strLog;
			strLog.Format(L"[%s] <<<<<<send data to client=%s\r\n", __FUNCTIONW__, YY::UTF8ToUnicode(strWrite.c_str()));
			OutputDebugString(strLog.GetString());
		}

		return MCP::ERRNO_OK;
	}

	int CNonConsoleStdioTransport::Error(const std::string& strIn)
	{
		return MCP::ERRNO_OK;
	}
}