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
		MCP::ServerCapabilities capabilities = Session().GetServerCapabilities();
		capabilities.tools = tools;
		Session().SetServerCapabilities(capabilities);
	}

	void CMCPServerBase::RegisterServerResourcesCapabilities(const MCP::Resources& resources)
	{
		MCP::ServerCapabilities capabilities = Session().GetServerCapabilities();
		capabilities.resources = resources;
		Session().SetServerCapabilities(capabilities);
	}

	void CMCPServerBase::RegisterServerPromptsCapabilities(const MCP::Prompts& prompts)
	{
		MCP::ServerCapabilities capabilities = Session().GetServerCapabilities();
		capabilities.prompts = prompts;
		Session().SetServerCapabilities(capabilities);
	}

	void CMCPServerBase::RegisterServerExperimentalCapabilities(const MCP::Experimental& experimental)
	{
		MCP::ServerCapabilities capabilities = Session().GetServerCapabilities();
		capabilities.experimental = experimental;
		Session().SetServerCapabilities(capabilities);
	}

	void CMCPServerBase::RegisterServerLoggingCapabilities()
	{
		MCP::ServerCapabilities capabilities = Session().GetServerCapabilities();
		capabilities.logging.bExist = true;
		Session().SetServerCapabilities(capabilities);
	}

	void CMCPServerBase::RegisterServerCompletionsCapability()
	{
		MCP::ServerCapabilities capabilities = Session().GetServerCapabilities();
		capabilities.bCompletions = true;
		Session().SetServerCapabilities(capabilities);
	}

	void CMCPServerBase::RegisterServerExtensionsCapabilities(const std::map<std::string, Json::Value>& mapExtensions)
	{
		MCP::ServerCapabilities capabilities = Session().GetServerCapabilities();
		for (const auto& pair : mapExtensions)
			capabilities.mapExtensions[pair.first] = pair.second;

		capabilities.bExtensionPassthrough = !capabilities.mapExtensions.empty();
		Session().SetServerCapabilities(capabilities);
	}

	void CMCPServerBase::RegisterServerAppsCapabilities()
	{
		MCP::ServerCapabilities capabilities = Session().GetServerCapabilities();
		capabilities.mapExtensions[EXTENSION_MCP_APPS_UI] = Json::Value(Json::objectValue);
		capabilities.bExtensionPassthrough = true;
		Session().SetServerCapabilities(capabilities);
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
		Session().SetServerCallToolsTask(strToolName, spTask);
	}

	void CMCPServerBase::RegisterReadResourceTasks(const std::string& strResourceUri, std::shared_ptr<MCP::ProcessReadResourceRequest> spTask)
	{
		Session().SetServerReadResourceTask(strResourceUri, spTask);
	}

	void CMCPServerBase::RegisterSubscribeResourceTasks(const std::string& strResourceUri, std::shared_ptr<MCP::ProcessSubscribeResourceRequest> spTask)
	{
		Session().SetServerSubscribeResourceTask(strResourceUri, spTask);
	}

	void CMCPServerBase::RegisterGetPromptTasks(const std::string& strPromptName, std::shared_ptr<MCP::ProcessGetPromptRequest> spTask)
	{
		Session().SetServerGetPromptTask(strPromptName, spTask);
	}

	void CMCPServerBase::RegisterCompleteTasks(const std::string& strTaskKey, std::shared_ptr<MCP::ProcessCompleteRequest> spTask)
	{
		Session().SetServerCompleteTask(strTaskKey, spTask);
	}

	int CMCPServerBase::Start()
	{
		if (!Session().GetTransport())
			Session().SetTransport(std::make_shared<CStdioTransport>());

		// Everything else was already written through to the session by its
		// registration call; only the version declaration is applied here.
		int iErrCode = Session().SetSupportedProtocolVersions(m_vecSupportedProtocolVersions);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

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
