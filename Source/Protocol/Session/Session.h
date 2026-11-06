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
#include "../Message/Response.h"
#include "../Message/Notification.h"
#include "../Message/BasicMessage.h"
#include "../Transport/Transport.h"
#include "../Task/BasicTask.h"

namespace MCP
{
	// Declared here, defined in Internal/Protocol.h. Session hands the registry to
	// the message layer but neither type is visible outside the SDK.
	struct ProtocolContext;
	class CProtocolRegistry;

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

		~CMCPSession();
		CMCPSession(const CMCPSession&) = delete;
		CMCPSession& operator=(const CMCPSession&) = delete;
		static CMCPSession& GetInstance();

		int Ready();
		int Run();
		int Terminate();
		int CancelTask(const MCP::RequestId& requestId);

		// Internal to the SDK. CMCPServer builds the registry from the versions
		// the application declares, so callers never construct profiles.
		bool IsKnownProtocolVersion(const std::string& strVersion) const;
		int SetSupportedProtocolVersions(const std::vector<std::string>& vecVersions);
		void SetProtocolRegistry(const CProtocolRegistry& registry);
		const CProtocolRegistry& GetProtocolRegistry() const;
		void SetServerCacheOptions(int iTtlMs, const std::string& strCacheScope);
		int GetServerCacheTtlMs() const;
		std::string GetServerCacheScope() const;
		void SetServerInstructions(const std::string& strInstructions);
		std::string GetServerInstructions() const;

		// Reset per-connection MCP session state so that a new client (after the previous
		// transport-level connection dropped) can re-issue `initialize` cleanly. This
		// preserves server-side static registrations (tools/resources/prompts/handlers and
		// server capabilities) while clearing the state machine, cached in-flight messages
		// and the async task thread that belonged to the departed client.
		int ResetForNewClient();

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

		// Read-only diagnostics. Only the legacy handshake path records message
		// history, so a stateless request must not change these counters.
		size_t GetMessageCount(MessageCategory eCategory) const;
		std::shared_ptr<MCP::ProcessRequest> GetServerCallToolsTask(const std::string& strToolName);
		std::shared_ptr<MCP::ProcessRequest> GetServerReadResourceTask(const std::string& strResourceUri);
		std::shared_ptr<MCP::ProcessRequest> GetServerSubscribeResourceTask(const std::string& strResourceUri);
		std::shared_ptr<MCP::ProcessRequest> GetServerGetPromptTask(const std::string& strPromptName);
		std::shared_ptr<MCP::ProcessRequest> GetServerCompleteTask(const std::string& strTaskKey);

		void SetLoggingLevel(const MCP::LoggingLevel& level);
		MCP::LoggingLevel GetLoggingLevel() const;
		int SendLogMessage(const MCP::LoggingLevel& level, const std::string& strLogger, const Json::Value& jData);
		int WriteLogNotification(const MCP::LoggingLevel& level, const std::string& strLogger, const Json::Value& jData);

		// Era resolution and result preparation shared by every task.
		ProtocolContext ResolveProtocolContext(const std::shared_ptr<MCP::Message>& spMsg) const;
		int ValidateProtocolContext(const ProtocolContext& ctxProtocol, const std::shared_ptr<MCP::Request>& spRequest);
		void PrepareResult(const std::shared_ptr<MCP::Request>& spRequest, const std::shared_ptr<MCP::Response>& spResponse);

		// Subscription notifications. Legacy clients receive the change event
		// unconditionally, stateless clients are filtered by their listen request.
		int SendToolListChanged();
		int SendPromptListChanged();
		int SendResourceListChanged();
		int SendSubscriptionNotification(const std::string& strMethod, const Json::Value& jParams, const std::string& strMatchUri = std::string());

	private:
		CMCPSession();

		struct SubscriptionState
		{
			MCP::RequestId requestId;
			std::shared_ptr<const ProtocolContext> spProtocolContext;
			std::shared_ptr<MCP::Request> spListenRequest;
			bool bToolsListChanged{ false };
			bool bPromptsListChanged{ false };
			bool bResourcesListChanged{ false };
			std::set<std::string> setResourceSubscriptions;
			bool bActive{ false };
		};

		int ParseMessage(const std::string& strMsg, std::shared_ptr<MCP::Message>& spMsg);
		int ParseRequest(const std::string& strMsg, const ProtocolContext& ctxProtocol, std::shared_ptr<MCP::Message>& spMsg);
		int ParseResponse(const std::string& strMsg, std::shared_ptr<MCP::Message>& spMsg);
		int ParseNotification(const std::string& strMsg, const ProtocolContext& ctxProtocol, std::shared_ptr<MCP::Message>& spMsg);
		ProtocolContext ResolveProtocolContext(const std::string& strMethod, const Json::Value& jMeta,
			const std::string& strRequestedVersion) const;
		int ProcessMessage(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg);
		int ProcessModernMessage(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg);
		int ProcessModernRequest(const ProtocolContext& ctxProtocol, const std::shared_ptr<MCP::Request>& spRequest);
		int ProcessModernNotification(const ProtocolContext& ctxProtocol, const std::shared_ptr<MCP::Notification>& spNotification);
		int ProcessRequest(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg);
		int ProcessResponse(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg);
		int ProcessNotification(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg);
		int SwitchState(SessionState eState);
		int SendErrorResponse(const std::shared_ptr<MCP::Request>& spRequest, int iErrCode, const std::string& strMessage, const Json::Value& jErrorData = Json::Value());
		int SendListChangedNotification(MessageType eMessageType, const std::string& strMethod);
		int RemoveSubscription(const MCP::RequestId& requestId);
		int CloseSubscriptions();
		int ProcessSubscriptionsListenRequest(const ProtocolContext& ctxProtocol, const std::shared_ptr<MCP::Request>& spRequest);

		// Asynchronous Task
		int EnsureAsyncTaskThread();
		int CommitAsyncTask(const std::shared_ptr<MCP::CMCPTask>& spTask);
		int CancelAsyncTask(const MCP::RequestId& requestId);
		int StartAsyncTaskThread();
		int StopAsyncTaskThread();
		int AsyncThreadProc();

		static CMCPSession s_Instance;

		SessionState m_eSessionState{ SessionState_Original };
		std::shared_ptr<CMCPTransport> m_spTransport;

		std::unique_ptr<CProtocolRegistry> m_upRegistry;
		int m_iCacheTtlMs{ 0 };
		std::string m_strCacheScope{ CACHE_SCOPE_PRIVATE };
		std::string m_strServerInstructions;
		std::unordered_map<std::string, SubscriptionState> m_hashSubscriptions;
		std::mutex m_mtxSubscriptions;

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