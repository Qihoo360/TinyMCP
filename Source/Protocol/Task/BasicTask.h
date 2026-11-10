#pragma once
// To ensure good cross-platform compatibility, all code within the MCP namespace is written in standard C++.
// The use of platform-specific system APIs is prohibited unless necessary.

#include "Task.h"
#include "../Message/Request.h"
#include "../Message/Response.h"
#include <atomic>
#include <memory>
#include <cstdint>

namespace MCP
{
	// Declared here, defined in Internal/TaskServices.h. The session binds the
	// services when it dispatches the task, so this header only forward declares
	// them.
	struct TaskServices;
	struct TaskServicesAccess;

	class ProcessRequest : public MCP::CMCPTask
	{
	public:
		ProcessRequest(const std::shared_ptr<MCP::Request>& spRequest)
			: m_spRequest(spRequest)
		{
		}

		// Task prototypes are cloned per dispatch, so the copy has to carry the
		// request and the (atomic) state flags. The copy starts from an idle
		// prototype, so no in-flight state is ever duplicated.
		ProcessRequest(const ProcessRequest& other);
		ProcessRequest& operator=(const ProcessRequest& other);

		bool IsValid() const override;
		bool IsFinished() const override;
		bool IsCancelled() const override;
		int Execute() override;
		int Cancel() override;

		void SetRequest(const std::shared_ptr<MCP::Request>& spRequest);
		std::shared_ptr<MCP::Request> GetRequest() const;
		int NotifyLogMessage(LoggingLevelValue eLevel, const std::string& strLogger, const Json::Value& jData);

		// Async ordering token assigned by the session. A cancellation only applies
		// to tasks enqueued at or before the cancellation, so a client that reuses a
		// request id after cancelling cannot lose the new request.
		uint64_t GetAsyncSequence() const;
		void SetAsyncSequence(uint64_t nSequence);

		// Set when the session is reset for a new client. A stale task must not
		// write its result into the new client's transport.
		bool IsStale() const;
		void MarkStale();

		// A strong reference to this task for background threads and IPC callbacks
		// that may run after the task reported its result. The session drops its
		// own reference once the task is finished, so any work that can still touch
		// the task afterwards must hold this instead of a raw `this`.
		// Empty when the task was not dispatched by the session (e.g. a prototype
		// or a Clone() that has not been dispatched yet).
		std::shared_ptr<CMCPTask> SharedFromThis() const;

		template <typename TTask>
		std::shared_ptr<TTask> SharedFromThis() const
		{
			return std::dynamic_pointer_cast<TTask>(SharedFromThis());
		}

	protected:
		// Session-side services for this dispatch. Never null for a task the
		// session created; IsValid() reports false otherwise.
		const std::shared_ptr<MCP::TaskServices>& GetTaskServices() const;

		// The single write path for task output. Every response, error and
		// notification goes through it so the stale and missing-service guards are
		// applied uniformly instead of per call site.
		int WriteToClient(const std::string& strMessage);

		// Result progress shared by every request kind. The flags are atomic
		// because the worker thread sets them while the session thread inspects
		// retired tasks during a reset.
		void MarkFinished();
		bool IsFinishedFlag() const;
		void MarkCancelledFlag();
		bool IsCancelledFlag() const;

		// Copies the request era onto the result and applies server defaults
		// before the result is serialized.
		int SerializeResult(const std::shared_ptr<MCP::Response>& spResult, std::string& strResponse);

		// Validates an MRTR round trip against the request's client capabilities.
		// Returns ERRNO_INVALID_PARAMS when neither part is present or the payload
		// is malformed, ERRNO_MISSING_REQUIRED_CLIENT_CAPABILITY when the client
		// did not declare a capability needed by one of the input requests.
		int ValidateInputRequired(const Json::Value& jInputRequests, const std::string& strRequestState) const;
		int SendInputRequiredResult(const std::shared_ptr<MCP::Response>& spResult,
			const Json::Value& jInputRequests, const std::string& strRequestState);
		int SendMissingClientCapability(const Json::Value& jRequiredCapabilities);

		std::shared_ptr<MCP::Request> m_spRequest;

		// Available to derived request kinds that build notifications themselves.
		std::shared_ptr<MCP::TaskServices> m_spServices;

	private:
		friend struct TaskServicesAccess;
		void SetTaskServices(const std::shared_ptr<MCP::TaskServices>& spServices);
		void SetSelf(const std::shared_ptr<MCP::CMCPTask>& spSelf);

		enum StateFlag : unsigned int
		{
			StateFlag_Finished = 1u << 0,
			StateFlag_Cancelled = 1u << 1,
			StateFlag_Stale = 1u << 2
		};

		std::atomic<unsigned int> m_nStateFlags{ 0 };
		uint64_t m_nAsyncSequence{ 0 };

		// Bound by the session to the shared_ptr it dispatches. Weak so the task
		// does not keep itself alive, and never copied so a clone cannot hand out
		// its prototype.
		std::weak_ptr<MCP::CMCPTask> m_wpSelf;
	};

	class ProcessPingRequest : public ProcessRequest
	{
	public:
		ProcessPingRequest(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessRequest(spRequest)
		{

		}

		std::shared_ptr<CMCPTask> Clone() const override;
		int Execute() override;
	};

	class ProcessErrorRequest : public ProcessRequest
	{
	public:
		ProcessErrorRequest(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessRequest(spRequest)
		{

		}

		std::shared_ptr<CMCPTask> Clone() const override;
		int Execute() override;

		void SetErrorCode(int iCode);
		void SetErrorMessage(const std::string& strMessage);
		void SetErrorData(const Json::Value& jErrorData);

	private:
		int m_iCode{ 0 };
		std::string m_strMessage;
		Json::Value m_jErrorData;
	};

	class ProcessInitializeRequest : public ProcessRequest
	{
	public:
		ProcessInitializeRequest(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessRequest(spRequest)
		{

		}

		std::shared_ptr<CMCPTask> Clone() const override;
		int Execute() override;
	};

	class ProcessListToolsRequest : public ProcessRequest
	{
	public:
		ProcessListToolsRequest(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessRequest(spRequest)
		{

		}

		std::shared_ptr<CMCPTask> Clone() const override;
		int Execute() override;
	};

	class ProcessCallToolRequest : public ProcessRequest
	{
	public:
		ProcessCallToolRequest(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessRequest(spRequest)
		{

		}

		bool IsFinished() const override;
		bool IsCancelled() const override;
		std::shared_ptr<MCP::CallToolResult> BuildResult();
		int NotifyProgress(int iProgress, int iTotal);
		int NotifyResult(std::shared_ptr<MCP::CallToolResult> spResult);
		int NotifyCancelled();
		int NotifyInputRequired(const Json::Value& jInputRequests, const std::string& strRequestState);
		int NotifyMissingClientCapability(const Json::Value& jRequiredCapabilities);
	};

	class ProcessListResourcesRequest : public ProcessRequest
	{
	public:
		ProcessListResourcesRequest(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessRequest(spRequest)
		{

		}

		std::shared_ptr<CMCPTask> Clone() const override;
		int Execute() override;
	};

	class ProcessReadResourceRequest : public ProcessRequest
	{
	public:
		ProcessReadResourceRequest(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessRequest(spRequest)
		{

		}

		bool IsFinished() const override;
		bool IsCancelled() const override;
		std::shared_ptr<MCP::ReadResourceResult> BuildResult();
		int NotifyProgress(int iProgress, int iTotal);
		int NotifyResult(std::shared_ptr<MCP::ReadResourceResult> spResult);
		int NotifyError(int iCode, const std::string& strMessage, const Json::Value& jErrData);
		int NotifyCancelled();
		int NotifyInputRequired(const Json::Value& jInputRequests, const std::string& strRequestState);
		int NotifyMissingClientCapability(const Json::Value& jRequiredCapabilities);
	};

	class ProcessSubscribeResourceRequest : public ProcessRequest
	{
	public:
		ProcessSubscribeResourceRequest(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessRequest(spRequest)
		{

		}

		bool IsFinished() const override;
		bool IsCancelled() const override;
		int NotifyResult();
		int NotifyError(int iCode, const std::string& strMessage, const Json::Value& jErrData);
		int NotifyUpdated();
		int NotifyCancelled();
	};

	class ProcessUnsubscribeResourceRequest : public ProcessRequest
	{
	public:
		ProcessUnsubscribeResourceRequest(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessRequest(spRequest)
		{

		}

		void SetRequestIdToUnsubscribe(const MCP::RequestId& requestId);
		int NotifyResult();
		int NotifyError(int iCode, const std::string& strMessage, const Json::Value& jErrData);

		std::shared_ptr<CMCPTask> Clone() const override;
		int Execute() override;

	private:
		MCP::RequestId m_requestIdToUnsubscribe;
	};

	class ProcessListResourceTemplatesRequest : public ProcessRequest
	{
	public:
		ProcessListResourceTemplatesRequest(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessRequest(spRequest)
		{

		}

		std::shared_ptr<CMCPTask> Clone() const override;
		int Execute() override;
	};

	class ProcessListPromptsRequest : public ProcessRequest
	{
	public:
		ProcessListPromptsRequest(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessRequest(spRequest)
		{

		}

		std::shared_ptr<CMCPTask> Clone() const override;
		int Execute() override;
	};

	class ProcessGetPromptRequest : public ProcessRequest
	{
	public:
		ProcessGetPromptRequest(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessRequest(spRequest)
		{

		}

		bool IsFinished() const override;
		bool IsCancelled() const override;
		std::shared_ptr<MCP::GetPromptResult> BuildResult();
		int NotifyProgress(int iProgress, int iTotal);
		int NotifyResult(std::shared_ptr<MCP::GetPromptResult> spResult);
		int NotifyError(int iCode, const std::string& strMessage, const Json::Value& jErrData);
		int NotifyCancelled();
		int NotifyInputRequired(const Json::Value& jInputRequests, const std::string& strRequestState);
		int NotifyMissingClientCapability(const Json::Value& jRequiredCapabilities);
	};

	class ProcessCompleteRequest : public ProcessRequest
	{
	public:
		ProcessCompleteRequest(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessRequest(spRequest)
		{

		}

		bool IsFinished() const override;
		bool IsCancelled() const override;
		std::shared_ptr<MCP::CompleteResult> BuildResult();
		int NotifyProgress(int iProgress, int iTotal);
		int NotifyResult(std::shared_ptr<MCP::CompleteResult> spResult);
		int NotifyError(int iCode, const std::string& strMessage, const Json::Value& jErrData);
		int NotifyCancelled();
	};

	class ProcessDiscoverRequest : public ProcessRequest
	{
	public:
		ProcessDiscoverRequest(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessRequest(spRequest)
		{

		}

		std::shared_ptr<CMCPTask> Clone() const override;
		int Execute() override;
	};

	class ProcessSetLevelRequest : public ProcessRequest
	{
	public:
		ProcessSetLevelRequest(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessRequest(spRequest)
		{

		}

		std::shared_ptr<CMCPTask> Clone() const override;
		int Execute() override;
	};
}