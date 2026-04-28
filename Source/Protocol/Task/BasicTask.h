#pragma once
// To ensure good cross-platform compatibility, all code within the MCP namespace is written in standard C++.
// The use of platform-specific system APIs is prohibited unless necessary.

#include "Task.h"
#include "../Message/Request.h"
#include "../Message/Response.h"
#include <memory>

namespace MCP
{
	class ProcessRequest : public MCP::CMCPTask
	{
	public:
		ProcessRequest(const std::shared_ptr<MCP::Request>& spRequest)
			: m_spRequest(spRequest)
		{
		}

		bool IsValid() const override;
		bool IsFinished() const override;
		bool IsCancelled() const override;
		int Execute() override;
		int Cancel() override;

		void SetRequest(const std::shared_ptr<MCP::Request>& spRequest);
		std::shared_ptr<MCP::Request> GetRequest() const;

	protected:
		std::shared_ptr<MCP::Request> m_spRequest;
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

	private:
		bool m_bFinished{ false };
		bool m_bCancelled{ false };
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

	private:
		bool m_bFinished{ false };
		bool m_bCancelled{ false };
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

	protected:
		bool m_bFinished{ false };
		bool m_bCancelled{ false };
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
}