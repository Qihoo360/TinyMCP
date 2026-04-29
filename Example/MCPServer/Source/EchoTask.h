#pragma once

#include <Task/BasicTask.h>
#include <Message/Request.h>
#include <string>
#include <atomic>
#include <thread>


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

	class CEchoResourceReadTask : public MCP::ProcessReadResourceRequest
	{
	public:
		CEchoResourceReadTask(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessReadResourceRequest(spRequest)
		{

		}

		std::shared_ptr<CMCPTask> Clone() const override;

		// If it's a time-consuming task, you need to start a thread to execute it asynchronously.
		int Execute() override;
		// This method is used to cancel time-consuming asynchronous tasks.
		int Cancel() override;
	};

	class CEchoResourceSubscribeTask : public MCP::ProcessSubscribeResourceRequest
	{
	public:
		CEchoResourceSubscribeTask(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessSubscribeResourceRequest(spRequest)
		{

		}

		std::shared_ptr<CMCPTask> Clone() const override;

		// If it's a time-consuming task, you need to start a thread to execute it asynchronously.
		int Execute() override;
		// This method is used to cancel time-consuming asynchronous tasks.
		int Cancel() override;

	private:
		std::atomic_bool m_bNeedCancel{ false };
		std::unique_ptr<std::thread> m_upSubscribeThread;
	};

	class CEchoGetPromptTask : public MCP::ProcessGetPromptRequest
	{
	public:
		static constexpr const char* PROMPT_CODE_REVIEW = "code_review";
		static constexpr const char* PROMPT_EXPLAIN_CODE = "explain_code";

		CEchoGetPromptTask(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessGetPromptRequest(spRequest)
		{

		}

		std::shared_ptr<CMCPTask> Clone() const override;

		// If it's a time-consuming task, you need to start a thread to execute it asynchronously.
		int Execute() override;
		// This method is used to cancel time-consuming asynchronous tasks.
		int Cancel() override;
	};
}
