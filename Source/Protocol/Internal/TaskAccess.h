#pragma once
// Internal to the SDK. The session binds its services to a task when it
// dispatches it, so the task never has to look the session up itself.

#include "../Task/BasicTask.h"
#include "TaskServices.h"

namespace MCP
{
	struct TaskServicesAccess
	{
		static void Set(MCP::ProcessRequest& task, const std::shared_ptr<MCP::TaskServices>& spServices)
		{
			task.SetTaskServices(spServices);
		}

		static void SetSelf(MCP::ProcessRequest& task, const std::shared_ptr<MCP::CMCPTask>& spSelf)
		{
			task.SetSelfKeepAlive(spSelf);
		}
	};
}
