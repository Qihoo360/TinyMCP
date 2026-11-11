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
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <cstdint>
#include <utility>
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
	struct ProtocolRegistryAccess;
	struct SessionAccess;
	struct TaskServices;

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
		int SetSupportedProtocolVersions(const std::vector<std::string>& vecVersions);
		void SetServerCacheOptions(double iTtlMs, const std::string& strCacheScope);
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

		// Per-entry registration. The server facade writes through immediately so
		// registering after Start() takes effect like every other registration.
		void SetServerCallToolsTask(const std::string& strToolName, const std::shared_ptr<MCP::ProcessCallToolRequest>& spTask);
		void SetServerReadResourceTask(const std::string& strResourceUri, const std::shared_ptr<MCP::ProcessReadResourceRequest>& spTask);
		void SetServerSubscribeResourceTask(const std::string& strResourceUri, const std::shared_ptr<MCP::ProcessSubscribeResourceRequest>& spTask);
		void SetServerGetPromptTask(const std::string& strPromptName, const std::shared_ptr<MCP::ProcessGetPromptRequest>& spTask);
		void SetServerCompleteTask(const std::string& strTaskKey, const std::shared_ptr<MCP::ProcessCompleteRequest>& spTask);
		MCP::Implementation GetServerInfo() const;
		MCP::ServerCapabilities GetServerCapabilities() const;
		// Applies fnUpdate to the capabilities under the registration lock, so
		// concurrent Register*Capabilities calls cannot lose each other's change.
		void UpdateServerCapabilities(const std::function<void(MCP::ServerCapabilities&)>& fnUpdate);
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

		void SetLoggingLevel(const MCP::LoggingLevel& level);
		MCP::LoggingLevel GetLoggingLevel() const;
		int SendLogMessage(const MCP::LoggingLevel& level, const std::string& strLogger, const Json::Value& jData);
		int WriteLogNotification(const MCP::LoggingLevel& level, const std::string& strLogger, const Json::Value& jData);

	private:
		CMCPSession();

		// Internal collaborators. Applications go through CMCPServer; tests reach
		// the read-only diagnostics through Internal/SessionAccess.h.
		friend struct SessionAccess;

		// The registry is derived from the version declaration, so it stays out of
		// the public API. SDK code reaches it through ProtocolRegistryAccess.
		friend struct ProtocolRegistryAccess;

		// Read-only diagnostics. Only the legacy handshake path records message
		// history, so a stateless request must not change these counters.
		size_t GetMessageCount(MessageCategory eCategory) const;
		std::shared_ptr<MCP::ProcessRequest> GetServerCallToolsTask(const std::string& strToolName);
		std::shared_ptr<MCP::ProcessRequest> GetServerReadResourceTask(const std::string& strResourceUri);
		std::shared_ptr<MCP::ProcessRequest> GetServerSubscribeResourceTask(const std::string& strResourceUri);
		std::shared_ptr<MCP::ProcessRequest> GetServerGetPromptTask(const std::string& strPromptName);
		std::shared_ptr<MCP::ProcessRequest> GetServerCompleteTask(const std::string& strTaskKey);

		double GetServerCacheTtlMs() const;
		std::string GetServerCacheScope() const;

		// Era resolution and result preparation shared by every task.
		std::shared_ptr<const ProtocolContext> ResolveProtocolContext(const std::shared_ptr<MCP::Message>& spMsg) const;
		int ValidateProtocolContext(const std::shared_ptr<const ProtocolContext>& spProtocolContext, const std::shared_ptr<MCP::Request>& spRequest);
		void PrepareResult(const std::shared_ptr<MCP::Request>& spRequest, const std::shared_ptr<MCP::Response>& spResponse);

		// Subscription notifications. Legacy clients receive the change event
		// unconditionally, stateless clients are filtered by their listen request.
		int SendToolListChanged();
		int SendPromptListChanged();
		int SendResourceListChanged();
		int SendSubscriptionNotification(const MCP::ResourceUpdatedNotification& notification, const std::string& strMatchUri = std::string());
		// Copies spTemplate once per matching subscription and stamps it with that
		// subscription's id before writing.
		int SendSubscriptionNotificationToTargets(const std::shared_ptr<MCP::Notification>& spTemplate,
			const std::string& strMethod, const std::string& strMatchUri = std::string());
		void SetProtocolRegistry(const CProtocolRegistry& registry);
		const CProtocolRegistry& GetProtocolRegistry() const;

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
		int ParseRequest(const std::string& strMsg, const std::shared_ptr<const ProtocolContext>& spProtocolContext, std::shared_ptr<MCP::Message>& spMsg);
		int ParseResponse(const std::string& strMsg, std::shared_ptr<MCP::Message>& spMsg);
		int ParseNotification(const std::string& strMsg, const std::shared_ptr<const ProtocolContext>& spProtocolContext, std::shared_ptr<MCP::Message>& spMsg);
		std::shared_ptr<const ProtocolContext> ResolveProtocolContext(const std::string& strMethod, const Json::Value& jMeta,
			const std::string& strRequestedVersion) const;
		int ProcessMessage(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg);
		int ProcessModernMessage(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg);
		int ProcessModernRequest(const std::shared_ptr<const ProtocolContext>& spProtocolContext, const std::shared_ptr<MCP::Request>& spRequest);
		int ProcessModernNotification(const std::shared_ptr<const ProtocolContext>& spProtocolContext, const std::shared_ptr<MCP::Notification>& spNotification);
		int ProcessRequest(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg);
		int ProcessResponse(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg);
		int ProcessNotification(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg);
		int SwitchState(SessionState eState);
		int SendErrorResponse(const std::shared_ptr<MCP::Request>& spRequest, int iErrCode, const std::string& strMessage, const Json::Value& jErrorData = Json::Value());
		int SendListChangedNotification(MessageType eMessageType, const std::string& strMethod);
		int RemoveSubscription(const MCP::RequestId& requestId);
		int CloseSubscriptions();
		int ProcessSubscriptionsListenRequest(const std::shared_ptr<const ProtocolContext>& spProtocolContext, const std::shared_ptr<MCP::Request>& spRequest);

		// Asynchronous Task
		int EnsureAsyncTaskThread();
		int CommitAsyncTask(const std::shared_ptr<MCP::CMCPTask>& spTask);
		int CancelAsyncTask(const MCP::RequestId& requestId);
		int StartAsyncTaskThread();
		int StopAsyncTaskThread();
		// bBoundedWait caps the join at a few seconds so a blocking tool cannot hang
		// shutdown. pbAbandoned reports that the worker is still running and therefore
		// still owns the task cache; callers must not touch it in that case.
		int ResetAsyncTaskThread(bool bBoundedWait = false, bool* pbAbandoned = nullptr);
		// Called once the worker is stopped: drops queued work, retires the task
		// cache and destroys every finished task outside the lock. Unfinished tasks
		// stay parked because background work may still reference them. Does
		// nothing to the cache when bAbandoned, since the worker still owns it.
		void ReleaseFinishedTasks(bool bAbandoned);
		// Shutdown only: cancels every parked task that is still running, waits up
		// to tmTimeout for it to finish and for its background work to drop the
		// task, then destroys it here instead of during static teardown. Tasks
		// that do not settle in time stay parked.
		void DrainUnfinishedTasks(std::chrono::milliseconds tmTimeout);
		int AsyncThreadProc(uint64_t nEpoch);
		// Caller holds m_mtxAsyncThread. True when a pending cancellation targets
		// this task (only cancellations issued after it was enqueued count).
		bool IsAsyncTaskCancelledLocked(const std::shared_ptr<MCP::CMCPTask>& spTask);

		// Tasks never reach back to the session singleton. The session binds the
		// services they need when it creates or clones them for dispatch.
		template <typename TTask>
		std::shared_ptr<TTask> MakeProcessTask(const std::shared_ptr<MCP::Request>& spRequest);
		void BindTaskServices(const std::shared_ptr<MCP::CMCPTask>& spTask);

		static CMCPSession s_Instance;

		std::atomic<SessionState> m_eSessionState{ SessionState_Original };

		// Read on the reader thread and on every async task thread, replaced only
		// before Ready(); the mutex keeps a late SetTransport from racing readers.
		mutable std::mutex m_mtxTransport;
		std::shared_ptr<CMCPTransport> m_spTransport;

		// Server options are published before Run() and read by tasks afterwards,
		// so the accessors take the lock even though the write happens once.
		mutable std::mutex m_mtxServerOptions;

		std::unique_ptr<CProtocolRegistry> m_upRegistry;
		std::shared_ptr<TaskServices> m_spTaskServices;
		double m_iCacheTtlMs{ 0 };
		std::string m_strCacheScope{ CACHE_SCOPE_PRIVATE };
		std::string m_strServerInstructions;
		std::unordered_map<std::string, SubscriptionState> m_hashSubscriptions;
		std::mutex m_mtxSubscriptions;

		// Registrations: server info, capabilities, the advertised lists and the
		// task prototypes. They may be registered after Start(), so every reader
		// and writer takes this lock.
		mutable std::mutex m_mtxRegistrations;

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

		mutable std::mutex m_mtxMessages;
		// Only counts are kept: storing every message of a long-lived connection
		// would grow without bound, and nothing reads the messages back.
		std::unordered_map<MessageCategory, size_t> m_hashMessageCount;
		std::unordered_map<std::string, std::shared_ptr<MCP::ProcessCallToolRequest>> m_hashCallToolsTasks;
		std::unordered_map<std::string, std::shared_ptr<MCP::ProcessReadResourceRequest>> m_hashReadResourceTasks;
		std::unordered_map<std::string, std::shared_ptr<MCP::ProcessSubscribeResourceRequest>> m_hashSubscribeResourceTasks;
		std::unordered_map<std::string, MCP::RequestId> m_hashUri2SubscribeResourceTaskId;
		std::unordered_map<std::string, std::shared_ptr<MCP::ProcessGetPromptRequest>> m_hashGetPromptTasks;
		std::unordered_map<std::string, std::shared_ptr<MCP::ProcessCompleteRequest>> m_hashCompleteTasks;

		// Asynchronous Task. m_mtxTaskThread serializes creation, joining and
		// reset of the worker so shutdown cannot race a newly spawned thread.
		std::mutex m_mtxTaskThread;
		std::unique_ptr<std::thread> m_upTaskThread;
		std::atomic_bool m_bRunAsyncTask{ true };
		std::mutex m_mtxAsyncThread;
		std::condition_variable m_cvAsyncThread;
		std::deque<std::shared_ptr<MCP::CMCPTask>> m_deqAsyncTasks;

		// Cancellations only apply to tasks enqueued at or before the cancellation,
		// so reusing a request id after cancelling cannot drop the new request.
		std::vector<std::pair<MCP::RequestId, uint64_t>> m_vecCancelledTaskIds;
		uint64_t m_nAsyncTaskSequence{ 0 };

		std::vector<std::shared_ptr<MCP::CMCPTask>> m_vecAsyncTasksCache;

		// Client generation. Bumped by ResetForNewClient; a task that finishes after
		// the bump is retired instead of writing into the next client's transport.
		uint64_t m_nClientGeneration{ 0 };

		// Tasks from a previous client are parked here so background threads and
		// IPC registrations holding a raw pointer can finish safely.
		std::vector<std::shared_ptr<MCP::CMCPTask>> m_vecRetiredTasks;

		// Worker generations. The epoch is bumped whenever a new worker starts and
		// the exiting worker publishes its own epoch, so a wait can never observe a
		// stale "already exited" signal from a previous incarnation.
		std::atomic<uint64_t> m_nAsyncThreadEpoch{ 0 };
		std::atomic<uint64_t> m_nAsyncThreadExitedEpoch{ 0 };

		// Reset/terminate waits for the current worker generation to finish, with an
		// upper bound on shutdown so a blocking tool cannot hang exit.
		std::mutex m_mtxAsyncExit;
		std::condition_variable m_cvAsyncExit;
	};
}