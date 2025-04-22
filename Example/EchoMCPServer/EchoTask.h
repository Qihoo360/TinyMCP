#pragma once

#include <Task/BasicTask.h>
#include <Message/Request.h>
#include <string>


namespace Implementation
{
	class CEchoTask : public MCP::ProcessCallToolRequest
	{
	public:
		static constexpr const char* TOOL_NAME = "echo";
		static constexpr const char* TOOL_DESCRIPTION = u8"Receive the data sent by the client and then return the exact same data to the client.";
		static constexpr const char* TOOL_INPUT_SCHEMA = u8R"({"type":"object","properties":{"input":{"type":"string","description":"client input data"}},"required":["input"]})";
		static constexpr const char* TOOL_ARGUMENT_INPUT = "input";

		CEchoTask(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessCallToolRequest(spRequest)
		{

		}

		std::shared_ptr<CMCPTask> Clone() const override;

		// If it's a time-consuming task, you need to start a thread to execute it asynchronously.
		int Execute() override;
		// This method is used to cancel time-consuming asynchronous tasks.
		int Cancel() override;
	};
}
