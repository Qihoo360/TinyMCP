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

	// 2026-07-28 MRTR example. The first call answers with input_required and an
	// opaque requestState; the retry that carries inputResponses completes the call.
	class CEchoConfirmTask : public MCP::ProcessCallToolRequest
	{
	public:
		static constexpr const char* TOOL_NAME = "echo_confirm";
		static constexpr const char* TOOL_DESCRIPTION = u8"Ask the client to confirm before returning the echoed data (MRTR example).";
		static constexpr const char* TOOL_INPUT_SCHEMA = u8R"({"type":"object","properties":{"input":{"type":"string","description":"client input data"}},"required":["input"]})";
		static constexpr const char* TOOL_ARGUMENT_INPUT = "input";

		CEchoConfirmTask(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessCallToolRequest(spRequest)
		{

		}

		std::shared_ptr<CMCPTask> Clone() const override;

		int Execute() override;
		int Cancel() override;
	};

	// MCP Apps example. The tool returns plain text for clients that do not
	// render the UI, and points to the UI resource through _meta.ui.resourceUri.
	class CEchoAppTask : public MCP::ProcessCallToolRequest
	{
	public:
		static constexpr const char* TOOL_NAME = "echo_app";
		static constexpr const char* TOOL_DESCRIPTION = u8"Return the echoed data together with an MCP Apps UI resource.";
		static constexpr const char* TOOL_INPUT_SCHEMA = u8R"({"type":"object","properties":{"input":{"type":"string","description":"client input data"}},"required":["input"]})";
		static constexpr const char* TOOL_OUTPUT_SCHEMA = u8R"({"type":"object","properties":{"echo":{"type":"string","description":"the echoed input"},"length":{"type":"integer","description":"character count of the echoed input"}},"required":["echo","length"]})";
		static constexpr const char* TOOL_ARGUMENT_INPUT = "input";

		CEchoAppTask(const std::shared_ptr<MCP::Request>& spRequest,
			const std::string& strToolName = TOOL_NAME)
			: ProcessCallToolRequest(spRequest)
			, m_strToolName(strToolName)
		{

		}

		std::shared_ptr<CMCPTask> Clone() const override;

		int Execute() override;
		int Cancel() override;

	private:
		std::string m_strToolName;
	};

	// MCP Apps constants shared by the server registration and the subscription
	// demo.
	constexpr const char* ECHO_APP_RESOURCE_URI = "ui://echo/app.html";
	constexpr const char* ECHO_FILE_APP_TOOL_NAME = "echo_file_app";
	constexpr const char* ECHO_FILE_APP_TOOL_DESCRIPTION = u8"Same as echo_app, but its HTML is loaded from a local file at registration time.";
	constexpr const char* ECHO_FILE_APP_RESOURCE_URI = "ui://echo/file-app.html";

	// The MCP Apps view served by the built-in resources/read task. It speaks the
	// View<->Host bridge, which belongs to the host, not to the SDK.
	const char* GetEchoAppHtml();

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

	class CEchoCompleteTask : public MCP::ProcessCompleteRequest
	{
	public:
		CEchoCompleteTask(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessCompleteRequest(spRequest)
		{

		}

		std::shared_ptr<CMCPTask> Clone() const override;

		// If it's a time-consuming task, you need to start a thread to execute it asynchronously.
		int Execute() override;
		// This method is used to cancel time-consuming asynchronous tasks.
		int Cancel() override;
	};
}
