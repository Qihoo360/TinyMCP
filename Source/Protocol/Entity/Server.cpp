#include "Server.h"
#include "../Internal/SessionAccess.h"
#include "../Session/Session.h"

namespace MCP
{
	namespace
	{
		CMCPSession& Session()
		{
			return CMCPSession::GetInstance();
		}
	}

	void CMCPServerBase::SetServerInfo(const MCP::Implementation& serverInfo)
	{
		Session().SetServerInfo(serverInfo);
	}

	void CMCPServerBase::SetTransport(const std::shared_ptr<MCP::CMCPTransport>& spTransport)
	{
		Session().SetTransport(spTransport);
	}

	std::shared_ptr<MCP::CMCPTransport> CMCPServerBase::GetTransport() const
	{
		return Session().GetTransport();
	}

	int CMCPServerBase::SendToolListChanged()
	{
		return SessionAccess::SendToolListChanged(Session());
	}

	int CMCPServerBase::SendPromptListChanged()
	{
		return SessionAccess::SendPromptListChanged(Session());
	}

	int CMCPServerBase::SendResourceListChanged()
	{
		return SessionAccess::SendResourceListChanged(Session());
	}

	int CMCPServerBase::SendSubscriptionNotification(const std::string& strMethod, const Json::Value& jParams,
		const std::string& strMatchUri)
	{
		return SessionAccess::SendSubscriptionNotification(Session(), strMethod, jParams, strMatchUri);
	}

	int CMCPServerBase::SetSupportedProtocolVersions(const std::vector<std::string>& vecVersions)
	{
		// Duplicate removal happens in the session, which owns the registry; this
		// method only rejects a declaration that asks for nothing.
		if (vecVersions.empty())
			return ERRNO_INVALID_PARAMS;

		m_vecSupportedProtocolVersions = vecVersions;
		return ERRNO_OK;
	}

	void CMCPServerBase::RegisterServerToolsCapabilities(const MCP::Tools& tools)
	{
		m_capabilities.tools = tools;
	}

	void CMCPServerBase::RegisterServerResourcesCapabilities(const MCP::Resources& resources)
	{
		m_capabilities.resources = resources;
	}

	void CMCPServerBase::RegisterServerPromptsCapabilities(const MCP::Prompts& prompts)
	{
		m_capabilities.prompts = prompts;
	}

	void CMCPServerBase::RegisterServerExperimentalCapabilities(const MCP::Experimental& experimental)
	{
		m_capabilities.experimental = experimental;
	}

	void CMCPServerBase::RegisterServerLoggingCapabilities()
	{
		m_capabilities.logging.bExist = true;
	}

	void CMCPServerBase::RegisterServerCompletionsCapability()
	{
		m_capabilities.bCompletions = true;
	}

	void CMCPServerBase::RegisterServerExtensionsCapabilities(const std::map<std::string, Json::Value>& mapExtensions)
	{
		for (const auto& pair : mapExtensions)
			m_capabilities.mapExtensions[pair.first] = pair.second;
	}

	void CMCPServerBase::RegisterServerAppsCapabilities()
	{
		m_capabilities.mapExtensions[EXTENSION_MCP_APPS_UI] = Json::Value(Json::objectValue);
	}

	void CMCPServerBase::RegisterServerInstructions(const std::string& strInstructions)
	{
		Session().SetServerInstructions(strInstructions);
	}

	void CMCPServerBase::SetServerCacheOptions(int iTtlMs, const std::string& strCacheScope)
	{
		Session().SetServerCacheOptions(iTtlMs, strCacheScope);
	}

	void CMCPServerBase::RegisterServerUiResources(const std::vector<MCP::Resource>& resources, bool bPagination)
	{
		RegisterServerResources(resources, bPagination);
	}

	void CMCPServerBase::RegisterServerTools(const std::vector<MCP::Tool>& tools, bool bPagination)
	{
		Session().SetServerToolsPagination(bPagination);
		Session().SetServerTools(tools);
	}

	void CMCPServerBase::RegisterServerResources(const std::vector<MCP::Resource>& resources, bool bPagination)
	{
		Session().SetServerResourcesPagination(bPagination);
		Session().SetServerResources(resources);
	}

	void CMCPServerBase::RegisterServerResourceTemplates(const std::vector<MCP::ResourceTemplate>& resourceTemplates, bool bPagination)
	{
		Session().SetServerResourceTemplatesPagination(bPagination);
		Session().SetServerResourceTemplates(resourceTemplates);
	}

	void CMCPServerBase::RegisterServerPrompts(const std::vector<MCP::Prompt>& prompts, bool bPagination)
	{
		Session().SetServerPromptsPagination(bPagination);
		Session().SetServerPrompts(prompts);
	}

	void CMCPServerBase::RegisterToolsTasks(const std::string& strToolName, std::shared_ptr<MCP::ProcessCallToolRequest> spTask)
	{
		m_hashCallToolsTasks[strToolName] = spTask;
	}

	void CMCPServerBase::RegisterReadResourceTasks(const std::string& strResourceUri, std::shared_ptr<MCP::ProcessReadResourceRequest> spTask)
	{
		m_hashReadResourceTasks[strResourceUri] = spTask;
	}

	void CMCPServerBase::RegisterSubscribeResourceTasks(const std::string& strResourceUri, std::shared_ptr<MCP::ProcessSubscribeResourceRequest> spTask)
	{
		m_hashSubscribeResourceTasks[strResourceUri] = spTask;
	}

	void CMCPServerBase::RegisterGetPromptTasks(const std::string& strPromptName, std::shared_ptr<MCP::ProcessGetPromptRequest> spTask)
	{
		m_hashGetPromptTasks[strPromptName] = spTask;
	}

	void CMCPServerBase::RegisterCompleteTasks(const std::string& strTaskKey, std::shared_ptr<MCP::ProcessCompleteRequest> spTask)
	{
		m_hashCompleteTasks[strTaskKey] = spTask;
	}

	int CMCPServerBase::Start()
	{
		if (!Session().GetTransport())
			Session().SetTransport(std::make_shared<CStdioTransport>());

		int iErrCode = Session().SetSupportedProtocolVersions(m_vecSupportedProtocolVersions);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		Session().SetServerCapabilities(m_capabilities);
		Session().SetServerCallToolsTasks(m_hashCallToolsTasks);
		Session().SetServerReadResourceTasks(m_hashReadResourceTasks);
		Session().SetServerSubscribeResourceTasks(m_hashSubscribeResourceTasks);
		Session().SetServerGetPromptTasks(m_hashGetPromptTasks);
		Session().SetServerCompleteTasks(m_hashCompleteTasks);

		iErrCode = Session().Ready();
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		return Session().Run();
	}

	int CMCPServerBase::Stop()
	{
		return Session().Terminate();
	}
}
