#pragma once
// To ensure good cross-platform compatibility, all code within the MCP namespace is written in standard C++.
// The use of platform-specific system APIs is prohibited unless necessary.

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
