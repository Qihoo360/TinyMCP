#pragma once
// 为保证较好的跨平台特性，MCP命名空间的代码均使用标准c++编码
// 非必要情况下，禁止使用特定系统平台API

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
