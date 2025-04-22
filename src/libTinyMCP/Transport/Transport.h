#pragma once
// 为保证较好的跨平台特性，MCP命名空间的代码均使用标准c++编码
// 非必要情况下，禁止使用特定系统平台API

#include <string>
#include <mutex>
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
