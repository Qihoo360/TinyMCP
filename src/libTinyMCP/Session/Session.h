#pragma once
// 为保证较好的跨平台特性，MCP命名空间的代码均使用标准c++编码
// 非必要情况下，禁止使用特定系统平台API

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

		void SetTransport(const std::shared_ptr<CMCPTransport>& spTransport);
		void SetServerInfo(const MCP::Implementation& impl);
		void SetServerCapabilities(const MCP::ServerCapabilities& capabilities);
		void SetServerToolsPagination(bool bPagination);
		void SetServerTools(const std::vector<MCP::Tool>& tools);
		void SetServerCallToolsTasks(const std::unordered_map<std::string, std::shared_ptr<MCP::ProcessCallToolRequest>>& hashCallToolsTasks);
		MCP::Implementation GetServerInfo() const;
		MCP::ServerCapabilities GetServerCapabilities() const;
		bool GetServerToolsPagination() const;
		std::vector<MCP::Tool> GetServerTools() const;
		std::shared_ptr<CMCPTransport> GetTransport() const;
		SessionState GetSessionState() const;
		std::shared_ptr<MCP::ProcessRequest> GetServerCallToolsTask(const std::string& strToolName);

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

		// 异步任务
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
		bool m_bToolsPagination{ false };

		std::unordered_map<MessageCategory, std::vector<std::shared_ptr<MCP::Message>>> m_hashMessage;
		std::unordered_map<std::string, std::shared_ptr<MCP::ProcessCallToolRequest>> m_hashCallToolsTasks;

		// 异步任务
		std::unique_ptr<std::thread> m_upTaskThread;
		std::atomic_bool m_bRunAsyncTask{ true };
		std::mutex m_mtxAsyncThread;
		std::condition_variable m_cvAsyncThread;
		std::deque<std::shared_ptr<MCP::CMCPTask>> m_deqAsyncTasks;
		std::vector<MCP::RequestId> m_vecCancelledTaskIds;
		std::vector<std::shared_ptr<MCP::CMCPTask>> m_vecAsyncTasksCache;
	};
}