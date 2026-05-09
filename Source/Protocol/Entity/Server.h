#pragma once
// To ensure good cross-platform compatibility, all code within the MCP namespace is written in standard C++.
// The use of platform-specific system APIs is prohibited unless necessary.

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

		void RegisterServerExperimentalCapabilities(const MCP::Experimental& experimental)
		{
			m_capabilities.experimental = experimental;
		}

		void RegisterServerLoggingCapabilities()
		{
			m_capabilities.logging.bExist = true;
		}

		void RegisterServerTools(const std::vector<MCP::Tool>& tools, bool bPagination)
		{
			MCP::CMCPSession::GetInstance().SetServerToolsPagination(bPagination);
			MCP::CMCPSession::GetInstance().SetServerTools(tools);
		}

		void RegisterServerResources(const std::vector<MCP::Resource>& resources, bool bPagination)
		{
			MCP::CMCPSession::GetInstance().SetServerResourcesPagination(bPagination);
			MCP::CMCPSession::GetInstance().SetServerResources(resources);
		}

		void RegisterServerResourceTemplates(const std::vector<MCP::ResourceTemplate>& resourceTemplates, bool bPagination)
		{
			MCP::CMCPSession::GetInstance().SetServerResourceTemplatesPagination(bPagination);
			MCP::CMCPSession::GetInstance().SetServerResourceTemplates(resourceTemplates);
		}

		void RegisterServerPrompts(const std::vector<MCP::Prompt>& prompts, bool bPagination)
		{
			MCP::CMCPSession::GetInstance().SetServerPromptsPagination(bPagination);
			MCP::CMCPSession::GetInstance().SetServerPrompts(prompts);
		}

		void RegisterToolsTasks(const std::string& strToolName, std::shared_ptr<MCP::ProcessCallToolRequest> spTask)
		{
			m_hashCallToolsTasks[strToolName] = spTask;
		}

		void RegisterReadResourceTasks(const std::string& strResourceUri, std::shared_ptr<MCP::ProcessReadResourceRequest> spTask)
		{
			m_hashReadResourceTasks[strResourceUri] = spTask;
		}

		void RegisterSubscribeResourceTasks(const std::string& strResourceUri, std::shared_ptr<MCP::ProcessSubscribeResourceRequest> spTask)
		{
			m_hashSubscribeResourceTasks[strResourceUri] = spTask;
		}

		void RegisterGetPromptTasks(const std::string& strPromptName, std::shared_ptr<MCP::ProcessGetPromptRequest> spTask)
		{
			m_hashGetPromptTasks[strPromptName] = spTask;
		}

		void RegisterCompleteTasks(const std::string& strTaskKey, std::shared_ptr<MCP::ProcessCompleteRequest> spTask)
		{
			m_hashCompleteTasks[strTaskKey] = spTask;
		}

		virtual int Initialize() = 0;
		virtual int Uninitialize() = 0;

		int Start()
		{
			if (!MCP::CMCPSession::GetInstance().GetTransport())
				MCP::CMCPSession::GetInstance().SetTransport(std::make_shared<CStdioTransport>());
			MCP::CMCPSession::GetInstance().SetServerCapabilities(m_capabilities);
			MCP::CMCPSession::GetInstance().SetServerCallToolsTasks(m_hashCallToolsTasks);
			MCP::CMCPSession::GetInstance().SetServerReadResourceTasks(m_hashReadResourceTasks);
			MCP::CMCPSession::GetInstance().SetServerSubscribeResourceTasks(m_hashSubscribeResourceTasks);
			MCP::CMCPSession::GetInstance().SetServerGetPromptTasks(m_hashGetPromptTasks);
			MCP::CMCPSession::GetInstance().SetServerCompleteTasks(m_hashCompleteTasks);

			int iErrCode = MCP::CMCPSession::GetInstance().Ready();
			if (ERRNO_OK != iErrCode)
				return iErrCode;

			return MCP::CMCPSession::GetInstance().Run();
		}

		int Stop()
		{
			return MCP::CMCPSession::GetInstance().Terminate();
		}

	protected:
		CMCPServer() = default;
		~CMCPServer() = default;

		MCP::ServerCapabilities m_capabilities;
		std::unordered_map<std::string, std::shared_ptr<MCP::ProcessCallToolRequest>> m_hashCallToolsTasks;
		std::unordered_map<std::string, std::shared_ptr<MCP::ProcessReadResourceRequest>> m_hashReadResourceTasks;
		std::unordered_map<std::string, std::shared_ptr<MCP::ProcessSubscribeResourceRequest>> m_hashSubscribeResourceTasks;
		std::unordered_map<std::string, std::shared_ptr<MCP::ProcessGetPromptRequest>> m_hashGetPromptTasks;
		std::unordered_map<std::string, std::shared_ptr<MCP::ProcessCompleteRequest>> m_hashCompleteTasks;
	};
}
