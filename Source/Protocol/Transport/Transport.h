#pragma once
// Ϊ��֤�ϺõĿ�ƽ̨���ԣ�MCP�����ռ�Ĵ����ʹ�ñ�׼c++����
// �Ǳ�Ҫ����£���ֹʹ���ض�ϵͳƽ̨API

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
