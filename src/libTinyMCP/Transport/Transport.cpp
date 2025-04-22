#include "Transport.h"
#include "../Public/PublicDef.h"
#include <iostream>

namespace MCP
{
	int CStdioTransport::Connect()
	{
		return ERRNO_OK;
	}

	int CStdioTransport::Disconnect()
	{
		return ERRNO_OK;
	}

	int CStdioTransport::Read(std::string& strOut)
	{
		const std::lock_guard<std::recursive_mutex> _lock(m_mtxStdin);

		if (std::getline(std::cin, strOut))
		{
			return ERRNO_OK;
		}
		else
		{
			if (std::cin.eof())
			{
				return ERRNO_INTERNAL_INPUT_TERMINATE;
			}
			else
			{
				return ERRNO_INTERNAL_INPUT_ERROR;
			}
		}

		return ERRNO_INTERNAL_ERROR;
	}

	int CStdioTransport::Write(const std::string& strIn)
	{
		const std::lock_guard<std::recursive_mutex> _lock(m_mtxStdout);
		std::cout << strIn << std::endl;

		return ERRNO_OK;
	}

	int CStdioTransport::Error(const std::string& strIn)
	{
		return ERRNO_OK;
	}
}