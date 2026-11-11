#include "Server.h"
#include "../Internal/SessionAccess.h"
#include "../Public/StringHelper.h"
#include "../Session/Session.h"

#include <fstream>
#include <set>
#include <sstream>

namespace MCP
{
	namespace
	{
		CMCPSession& Session()
		{
			return CMCPSession::GetInstance();
		}

		bool IsStringArrayOf(const Json::Value& jValue, const std::set<std::string>& setAllowed)
		{
			if (!jValue.isArray())
				return false;

			for (const auto& jItem : jValue)
			{
				if (!jItem.isString() || setAllowed.find(jItem.asString()) == setAllowed.end())
					return false;
			}

			return true;
		}

		// Rejects Apps declarations the specification forbids, before the server
		// starts serving. Dynamic resource templates never take part in UI
		// references, so only registered concrete resources are matched.
		int ValidateAppsConfiguration(const std::vector<MCP::Tool>& vecTools, const std::vector<MCP::Resource>& vecResources)
		{
			std::set<std::string> setUiResourceUris;
			for (const auto& resource : vecResources)
			{
				if (!IsUiResourceUri(resource.strUri))
					continue;

				if (!IsMcpAppMimeType(resource.strMimeType))
					return ERRNO_INVALID_PARAMS;

				setUiResourceUris.insert(resource.strUri);
			}

			const std::set<std::string> setVisibility{ UI_VISIBILITY_MODEL, UI_VISIBILITY_APP };
			for (const auto& tool : vecTools)
			{
				if (!tool.jMeta.isObject()
					|| !tool.jMeta.isMember(MSG_KEY_UI)
					|| !tool.jMeta[MSG_KEY_UI].isObject())
					continue;

				const Json::Value& jUi = tool.jMeta[MSG_KEY_UI];
				if (jUi.isMember(MSG_KEY_RESOURCE_URI))
				{
					const Json::Value& jResourceUri = jUi[MSG_KEY_RESOURCE_URI];
					if (!jResourceUri.isString() || !IsUiResourceUri(jResourceUri.asString()))
						return ERRNO_INVALID_PARAMS;

					if (setUiResourceUris.find(jResourceUri.asString()) == setUiResourceUris.end())
						return ERRNO_INVALID_PARAMS;
				}

				if (jUi.isMember(MSG_KEY_VISIBILITY) && !IsStringArrayOf(jUi[MSG_KEY_VISIBILITY], setVisibility))
					return ERRNO_INVALID_PARAMS;
			}

			return ERRNO_OK;
		}

		// A one-stop App is only accepted when every part needed at serve time is
		// present. Nothing is registered until this passes, so a rejected call
		// leaves the server exactly as it was.
		int ValidateAppDefinition(const MCP::AppDefinition& app)
		{
			if (app.tool.strName.empty() || !app.tool.IsValid())
				return ERRNO_INVALID_PARAMS;

			if (!IsUiResourceUri(app.resource.strUri))
				return ERRNO_INVALID_PARAMS;

			if (app.resource.strName.empty())
				return ERRNO_INVALID_PARAMS;

			if (!app.resource.strMimeType.empty() && !IsMcpAppMimeType(app.resource.strMimeType))
				return ERRNO_INVALID_PARAMS;

			if (app.strHtml.empty())
				return ERRNO_INVALID_PARAMS;

			if (!app.spToolTask)
				return ERRNO_INVALID_PARAMS;

			const Json::Value& jMeta = app.tool.jMeta;
			if (jMeta.isObject()
				&& jMeta.isMember(MSG_KEY_UI)
				&& jMeta[MSG_KEY_UI].isObject()
				&& jMeta[MSG_KEY_UI].isMember(MSG_KEY_RESOURCE_URI))
			{
				const Json::Value& jResourceUri = jMeta[MSG_KEY_UI][MSG_KEY_RESOURCE_URI];
				if (!jResourceUri.isString() || jResourceUri.asString() != app.resource.strUri)
					return ERRNO_INVALID_PARAMS;
			}

			return ERRNO_OK;
		}

		bool HasUiVisibility(const Json::Value& jMeta)
		{
			return jMeta.isObject()
				&& jMeta.isMember(MSG_KEY_UI)
				&& jMeta[MSG_KEY_UI].isObject()
				&& jMeta[MSG_KEY_UI].isMember(MSG_KEY_VISIBILITY);
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

	int CMCPServerBase::SendSubscriptionNotification(const MCP::ResourceUpdatedNotification& notification,
		const std::string& strMatchUri)
	{
		return SessionAccess::SendSubscriptionNotification(Session(), notification, strMatchUri);
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
		Session().UpdateServerCapabilities([&](MCP::ServerCapabilities& capabilities)
			{
				capabilities.tools = tools;
			});
	}

	void CMCPServerBase::RegisterServerResourcesCapabilities(const MCP::Resources& resources)
	{
		Session().UpdateServerCapabilities([&](MCP::ServerCapabilities& capabilities)
			{
				capabilities.resources = resources;
			});
	}

	void CMCPServerBase::RegisterServerPromptsCapabilities(const MCP::Prompts& prompts)
	{
		Session().UpdateServerCapabilities([&](MCP::ServerCapabilities& capabilities)
			{
				capabilities.prompts = prompts;
			});
	}

	void CMCPServerBase::RegisterServerExperimentalCapabilities(const MCP::Experimental& experimental)
	{
		Session().UpdateServerCapabilities([&](MCP::ServerCapabilities& capabilities)
			{
				capabilities.experimental = experimental;
			});
	}

	void CMCPServerBase::RegisterServerLoggingCapabilities()
	{
		Session().UpdateServerCapabilities([&](MCP::ServerCapabilities& capabilities)
			{
				capabilities.logging.bExist = true;
			});
	}

	void CMCPServerBase::RegisterServerCompletionsCapability()
	{
		Session().UpdateServerCapabilities([&](MCP::ServerCapabilities& capabilities)
			{
				capabilities.bCompletions = true;
			});
	}

	void CMCPServerBase::RegisterServerExtensionsCapabilities(const std::map<std::string, Json::Value>& mapExtensions)
	{
		Session().UpdateServerCapabilities([&](MCP::ServerCapabilities& capabilities)
			{
				for (const auto& pair : mapExtensions)
				{
					capabilities.mapExtensions[pair.first] = pair.second;
					// MCP Apps is modern-only even when registered through the
					// generic extension API.
					if (pair.first == EXTENSION_UI)
						capabilities.setModernOnlyExtensions.insert(pair.first);
				}

				capabilities.bExtensionPassthrough = !capabilities.mapExtensions.empty();
			});
	}

	void CMCPServerBase::RegisterServerAppsCapabilities()
	{
		Session().UpdateServerCapabilities([&](MCP::ServerCapabilities& capabilities)
			{
				capabilities.mapExtensions[EXTENSION_UI] = Json::Value(Json::objectValue);
				capabilities.setModernOnlyExtensions.insert(EXTENSION_UI);
				capabilities.bExtensionPassthrough = true;
			});
	}

	void CMCPServerBase::RegisterServerInstructions(const std::string& strInstructions)
	{
		Session().SetServerInstructions(strInstructions);
	}

	void CMCPServerBase::SetServerCacheOptions(double iTtlMs, const std::string& strCacheScope)
	{
		Session().SetServerCacheOptions(iTtlMs, strCacheScope);
	}

	void CMCPServerBase::RegisterServerUiResources(const std::vector<MCP::Resource>& resources, bool bPagination)
	{
		// ui:// resources are MCP Apps by definition, so a missing MIME type is
		// filled in here instead of changing the generic registration API.
		std::vector<MCP::Resource> vecUiResources = resources;
		for (auto& resource : vecUiResources)
		{
			if (IsUiResourceUri(resource.strUri) && resource.strMimeType.empty())
				resource.strMimeType = MIME_TYPE_MCP_APP;
		}

		RegisterServerResources(vecUiResources, bPagination);
	}

	int CMCPServerBase::RegisterServerApp(const MCP::AppDefinition& app)
	{
		int iErrCode = ValidateAppDefinition(app);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		// The App definition is the declaration, so the extension is advertised here
		// instead of asking the application to announce it a second time.
		RegisterServerAppsCapabilities();

		MCP::Tool tool = app.tool;
		tool.SetUiResourceUri(app.resource.strUri);
		if (!HasUiVisibility(tool.jMeta))
			tool.SetUiVisibility({ UI_VISIBILITY_MODEL, UI_VISIBILITY_APP });

		MCP::Resource resource = app.resource;
		resource.SetUiMeta(app.uiResourceMeta);

		// The bulk registration APIs replace the whole list, so the existing
		// declarations are carried over; a one-stop App adds to the server instead
		// of replacing what was registered before it.
		const bool bToolsPagination = Session().GetServerToolsPagination();
		std::vector<MCP::Tool> vecTools = Session().GetServerTools();
		vecTools.push_back(tool);
		RegisterServerTools(vecTools, bToolsPagination);

		const bool bResourcesPagination = Session().GetServerResourcesPagination();
		std::vector<MCP::Resource> vecResources = Session().GetServerResources();
		vecResources.push_back(resource);
		RegisterServerUiResources(vecResources, bResourcesPagination);

		RegisterToolsTasks(tool.strName, app.spToolTask);

		auto spReadTask = std::make_shared<MCP::ProcessStaticUiResourceReadTask>(nullptr, resource, app.strHtml);
		if (!spReadTask)
			return ERRNO_INTERNAL_ERROR;
		RegisterReadResourceTasks(resource.strUri, spReadTask);

		return ERRNO_OK;
	}

	int CMCPServerBase::RegisterServerAppFromFile(const MCP::AppDefinition& app, const std::string& strHtmlFilePath)
	{
		if (strHtmlFilePath.empty())
			return ERRNO_INVALID_PARAMS;

		std::ifstream ifs(strHtmlFilePath.c_str(), std::ios::in | std::ios::binary);
		if (!ifs.is_open())
			return ERRNO_INVALID_PARAMS;

		// Binary mode keeps the file byte-for-byte, so line endings and a UTF-8 BOM
		// survive unchanged.
		std::ostringstream oss;
		oss << ifs.rdbuf();
		if (ifs.bad())
			return ERRNO_INVALID_PARAMS;

		std::string strHtml = oss.str();
		if (strHtml.empty())
			return ERRNO_INVALID_PARAMS;

		MCP::AppDefinition fileApp = app;
		fileApp.strHtml = strHtml;

		return RegisterServerApp(fileApp);
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

		// Only validate when the application opted into MCP Apps, so a server that
		// never registered it keeps its previous startup behaviour.
		const MCP::ServerCapabilities capabilities = Session().GetServerCapabilities();
		if (capabilities.mapExtensions.find(EXTENSION_UI) != capabilities.mapExtensions.end())
		{
			iErrCode = ValidateAppsConfiguration(Session().GetServerTools(), Session().GetServerResources());
			if (ERRNO_OK != iErrCode)
				return iErrCode;
		}

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
