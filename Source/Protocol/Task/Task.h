#pragma once
// Ϊ��֤�ϺõĿ�ƽ̨���ԣ�MCP�����ռ�Ĵ����ʹ�ñ�׼c++����
// �Ǳ�Ҫ����£���ֹʹ���ض�ϵͳƽ̨API

#include <memory>
#include "../Message/Message.h"

namespace MCP
{
	class CMCPTask
	{
	public:
		virtual ~CMCPTask() {}
		virtual std::shared_ptr<CMCPTask> Clone() const = 0;
		virtual bool IsValid() const = 0;
		virtual bool IsFinished() const = 0;
		virtual bool IsCancelled() const = 0;
		virtual int Execute() = 0;
		virtual int Cancel() = 0;
	};
}
