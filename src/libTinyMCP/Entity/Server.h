#pragma once
// 为保证较好的跨平台特性，MCP命名空间的代码均使用标准c++编码
// 非必要情况下，禁止使用特定系统平台API

#include <memory>
#include "../Public/PublicDef.h"
#include "../Session/Session.h"
#include "../Transport/Transport.h"

namespace MCP
{
	template <class T>
	class CMCPServer
	{
	public:
		CMCPServer(const CMCPServer&) = delete;
		CMCPServer& operator=(const CMCPServer&) = delete;
		static CMCPServer& GetInstance()
		{
			return T::s_Instance;
		}

		void SetServerInfo(const MCP::Implementation& serverInfo)
		{
			MCP::CMCPSession::GetInstance().SetServerInfo(serverInfo);
		}

		void SetTransport(const std::shared_ptr<MCP::CMCPTransport>& spTransport)
		{
			MCP::CMCPSession::GetInstance().SetTransport(spTransport);
		}

		void RegisterServerToolsCapabilities(const MCP::Tools& tools)
		{
			m_capabilities.tools = tools;
		}

		void RegisterServerResourcesCapabilities(const MCP::Resources& resources)
		{
			m_capabilities.resources = resources;
		}

		void RegisterServerPromptsCapabilities(const MCP::Prompts& prompts)
		{
			m_capabilities.prompts = prompts;
		}

		void RegisterServerTools(const std::vector<MCP::Tool>& tools, bool bPagination)
		{
			MCP::CMCPSession::GetInstance().SetServerToolsPagination(bPagination);
			MCP::CMCPSession::GetInstance().SetServerTools(tools);
		}

		void RegisterToolsTasks(const std::string& strToolName, std::shared_ptr<MCP::ProcessCallToolRequest> spTask)
		{
			m_hashCallToolsTasks[strToolName] = spTask;
		}

		virtual int Initialize() = 0;

		int Start()
		{
			if (!MCP::CMCPSession::GetInstance().GetTransport())
				MCP::CMCPSession::GetInstance().SetTransport(std::make_shared<CStdioTransport>());
			MCP::CMCPSession::GetInstance().SetServerCapabilities(m_capabilities);
			MCP::CMCPSession::GetInstance().SetServerCallToolsTasks(m_hashCallToolsTasks);

			int iErrCode = MCP::CMCPSession::GetInstance().Ready();
			if (ERRNO_OK != iErrCode)
				return iErrCode;

			MCP::CMCPSession::GetInstance().Run();

			return ERRNO_OK;
		}

		int Stop()
		{
			MCP::CMCPSession::GetInstance().Terminate();

			return ERRNO_OK;
		}

	protected:
		CMCPServer() = default;
		~CMCPServer() = default;

		MCP::ServerCapabilities m_capabilities;
		std::unordered_map<std::string, std::shared_ptr<MCP::ProcessCallToolRequest>> m_hashCallToolsTasks;
	};
}
