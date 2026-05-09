#pragma once
// To ensure good cross-platform compatibility, all code within the MCP namespace is written in standard C++.
// The use of platform-specific system APIs is prohibited unless necessary.

#include <memory>
#include <vector>
#include <deque>
#include <set>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include "../Public/PublicDef.h"
#include "../Message/Request.h"
#include "../Message/BasicMessage.h"
#include "../Transport/Transport.h"
#include "../Task/BasicTask.h"

namespace MCP
{
	class CMCPSession
	{
	public:
		enum SessionState
		{
			SessionState_Original,
			SessionState_Initializing,
			SessionState_Initialized,
			SessionState_Shutting,
			SessionState_Shut,
		};

		~CMCPSession() = default;
		CMCPSession(const CMCPSession&) = delete;
		CMCPSession& operator=(const CMCPSession&) = delete;
		static CMCPSession& GetInstance();

		int Ready();
		int Run();
		int Terminate();
		int CancelTask(const MCP::RequestId& requestId);

		void SetTransport(const std::shared_ptr<CMCPTransport>& spTransport);
		void SetServerInfo(const MCP::Implementation& impl);
		void SetServerCapabilities(const MCP::ServerCapabilities& capabilities);
		void SetServerToolsPagination(bool bPagination);
		void SetServerTools(const std::vector<MCP::Tool>& tools);
		void SetServerResourcesPagination(bool bPagination);
		void SetServerResources(const std::vector<MCP::Resource>& resources);
		void SetServerResourceTemplatesPagination(bool bPagination);
		void SetServerResourceTemplates(const std::vector<MCP::ResourceTemplate>& resourceTemplates);
		void SetServerPromptsPagination(bool bPagination);
		void SetServerPrompts(const std::vector<MCP::Prompt>& prompts);
		void SetServerCallToolsTasks(const std::unordered_map<std::string, std::shared_ptr<MCP::ProcessCallToolRequest>>& hashCallToolsTasks);
		void SetServerReadResourceTasks(const std::unordered_map<std::string, std::shared_ptr<MCP::ProcessReadResourceRequest>>& hashReadResourceTasks);
		void SetServerSubscribeResourceTasks(const std::unordered_map<std::string, std::shared_ptr<MCP::ProcessSubscribeResourceRequest>>& hashSubscribeResourceTasks);
		void SetServerGetPromptTasks(const std::unordered_map<std::string, std::shared_ptr<MCP::ProcessGetPromptRequest>>& hashGetPromptTasks);
		void SetServerCompleteTasks(const std::unordered_map<std::string, std::shared_ptr<MCP::ProcessCompleteRequest>>& hashCompleteTasks);
		MCP::Implementation GetServerInfo() const;
		MCP::ServerCapabilities GetServerCapabilities() const;
		bool GetServerToolsPagination() const;
		std::vector<MCP::Tool> GetServerTools() const;
		bool GetServerResourcesPagination() const;
		std::vector<MCP::Resource> GetServerResources() const;
		bool GetServerResourceTemplatesPagination() const;
		std::vector<MCP::ResourceTemplate> GetServerResourceTemplates() const;
		bool GetServerPromptsPagination() const;
		std::vector<MCP::Prompt> GetServerPrompts() const;
		std::shared_ptr<CMCPTransport> GetTransport() const;
		SessionState GetSessionState() const;
		std::shared_ptr<MCP::ProcessRequest> GetServerCallToolsTask(const std::string& strToolName);
		std::shared_ptr<MCP::ProcessRequest> GetServerReadResourceTask(const std::string& strResourceUri);
		std::shared_ptr<MCP::ProcessRequest> GetServerSubscribeResourceTask(const std::string& strResourceUri);
		std::shared_ptr<MCP::ProcessRequest> GetServerGetPromptTask(const std::string& strPromptName);
		std::shared_ptr<MCP::ProcessRequest> GetServerCompleteTask(const std::string& strTaskKey);

		void SetLoggingLevel(const MCP::LoggingLevel& level);
		MCP::LoggingLevel GetLoggingLevel() const;
		int SendLogMessage(const MCP::LoggingLevel& level, const std::string& strLogger, const Json::Value& jData);

	private:
		CMCPSession() = default;
		int ParseMessage(const std::string& strMsg, std::shared_ptr<MCP::Message>& spMsg);
		int ParseRequest(const std::string& strMsg, std::shared_ptr<MCP::Message>& spMsg);
		int ParseResponse(const std::string& strMsg, std::shared_ptr<MCP::Message>& spMsg);
		int ParseNotification(const std::string& strMsg, std::shared_ptr<MCP::Message>& spMsg);
		int ProcessMessage(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg);
		int ProcessRequest(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg);
		int ProcessResponse(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg);
		int ProcessNotification(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg);
		int SwitchState(SessionState eState);

		// Asynchronous Task
		int CommitAsyncTask(const std::shared_ptr<MCP::CMCPTask>& spTask);
		int CancelAsyncTask(const MCP::RequestId& requestId);
		int StartAsyncTaskThread();
		int StopAsyncTaskThread();
		int AsyncThreadProc();

		static CMCPSession s_Instance;

		SessionState m_eSessionState{ SessionState_Original };
		std::shared_ptr<CMCPTransport> m_spTransport;

		MCP::Implementation m_serverInfo;
		MCP::ServerCapabilities m_capabilities;
		std::vector<MCP::Tool> m_tools;
		std::vector<MCP::Resource> m_resources;
		std::vector<MCP::ResourceTemplate> m_resourceTemplates;
		std::vector<MCP::Prompt> m_prompts;
		bool m_bToolsPagination{ false };
		bool m_bResourcesPagination{ false };
		bool m_bResourceTemplatesPagination{ false };
		bool m_bPromptsPagination{ false };
		MCP::LoggingLevel m_loggingLevel;

		std::unordered_map<MessageCategory, std::vector<std::shared_ptr<MCP::Message>>> m_hashMessage;
		std::unordered_map<std::string, std::shared_ptr<MCP::ProcessCallToolRequest>> m_hashCallToolsTasks;
		std::unordered_map<std::string, std::shared_ptr<MCP::ProcessReadResourceRequest>> m_hashReadResourceTasks;
		std::unordered_map<std::string, std::shared_ptr<MCP::ProcessSubscribeResourceRequest>> m_hashSubscribeResourceTasks;
		std::unordered_map<std::string, MCP::RequestId> m_hashUri2SubscribeResourceTaskId;
		std::unordered_map<std::string, std::shared_ptr<MCP::ProcessGetPromptRequest>> m_hashGetPromptTasks;
		std::unordered_map<std::string, std::shared_ptr<MCP::ProcessCompleteRequest>> m_hashCompleteTasks;

		// Asynchronous Task
		std::unique_ptr<std::thread> m_upTaskThread;
		std::atomic_bool m_bRunAsyncTask{ true };
		std::mutex m_mtxAsyncThread;
		std::condition_variable m_cvAsyncThread;
		std::deque<std::shared_ptr<MCP::CMCPTask>> m_deqAsyncTasks;
		std::vector<MCP::RequestId> m_vecCancelledTaskIds;
		std::vector<std::shared_ptr<MCP::CMCPTask>> m_vecAsyncTasksCache;
	};
}