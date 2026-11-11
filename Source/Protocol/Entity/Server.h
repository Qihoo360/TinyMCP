#pragma once
// To ensure good cross-platform compatibility, all code within the MCP namespace is written in standard C++.
// The use of platform-specific system APIs is prohibited unless necessary.

#include <memory>
#include <string>
#include <vector>
#include "../Public/PublicDef.h"
#include "../Message/BasicMessage.h"
#include "../Message/Notification.h"
#include "../Task/BasicTask.h"
#include "../Transport/Transport.h"

namespace MCP
{
	// Shared behaviour of every CMCPServer<T>. Keeping it outside the template
	// lets the wiring live in Server.cpp: the header only describes what an
	// application may call, and the session singleton stays hidden in the .cpp.
	class CMCPServerBase
	{
	public:
		CMCPServerBase(const CMCPServerBase&) = delete;
		CMCPServerBase& operator=(const CMCPServerBase&) = delete;

		void SetServerInfo(const MCP::Implementation& serverInfo);
		void SetTransport(const std::shared_ptr<MCP::CMCPTransport>& spTransport);
		std::shared_ptr<MCP::CMCPTransport> GetTransport() const;

		// Server-initiated notifications. Only active listeners receive them, so a
		// server without subscriptions sends nothing.
		int SendToolListChanged();
		int SendPromptListChanged();
		int SendResourceListChanged();
		int SendSubscriptionNotification(const MCP::ResourceUpdatedNotification& notification,
			const std::string& strMatchUri = std::string());

		// Declares the protocol revisions this server serves. Call it before
		// Start(); leaving it unused means every revision the transport can
		// carry. Only an empty declaration is rejected here: which revisions the
		// SDK knows is a registry question, so Start() owns that decision. This is
		// the single pre-Start setting; every other registration takes effect at
		// once and stays effective after Start().
		int SetSupportedProtocolVersions(const std::vector<std::string>& vecVersions);

		void RegisterServerToolsCapabilities(const MCP::Tools& tools);
		void RegisterServerResourcesCapabilities(const MCP::Resources& resources);
		void RegisterServerPromptsCapabilities(const MCP::Prompts& prompts);
		void RegisterServerExperimentalCapabilities(const MCP::Experimental& experimental);
		void RegisterServerLoggingCapabilities();

		// Advertises the stateless-era completions capability. Legacy handshake
		// output keeps using RegisterServerExperimentalCapabilities.
		void RegisterServerCompletionsCapability();

		void RegisterServerExtensionsCapabilities(const std::map<std::string, Json::Value>& mapExtensions);

		// MCP Apps is opt-in; the extension is advertised only after this call.
		void RegisterServerAppsCapabilities();

		void RegisterServerInstructions(const std::string& strInstructions);
		void SetServerCacheOptions(int iTtlMs, const std::string& strCacheScope);

		// Convenience wrapper for UI resources; the UI metadata travels in
		// Resource::jMeta unchanged.
		void RegisterServerUiResources(const std::vector<MCP::Resource>& resources, bool bPagination);

		void RegisterServerTools(const std::vector<MCP::Tool>& tools, bool bPagination);
		void RegisterServerResources(const std::vector<MCP::Resource>& resources, bool bPagination);
		void RegisterServerResourceTemplates(const std::vector<MCP::ResourceTemplate>& resourceTemplates, bool bPagination);
		void RegisterServerPrompts(const std::vector<MCP::Prompt>& prompts, bool bPagination);

		void RegisterToolsTasks(const std::string& strToolName, std::shared_ptr<MCP::ProcessCallToolRequest> spTask);
		void RegisterReadResourceTasks(const std::string& strResourceUri, std::shared_ptr<MCP::ProcessReadResourceRequest> spTask);
		void RegisterSubscribeResourceTasks(const std::string& strResourceUri, std::shared_ptr<MCP::ProcessSubscribeResourceRequest> spTask);
		void RegisterGetPromptTasks(const std::string& strPromptName, std::shared_ptr<MCP::ProcessGetPromptRequest> spTask);
		void RegisterCompleteTasks(const std::string& strTaskKey, std::shared_ptr<MCP::ProcessCompleteRequest> spTask);

		int Start();
		int Stop();

	protected:
		CMCPServerBase() = default;
		~CMCPServerBase() = default;

		// The registry owns every other setting once registered; only the version
		// declaration is kept here because Start() is where it is applied.
		std::vector<std::string> m_vecSupportedProtocolVersions;
	};

	template <class T>
	class CMCPServer : public CMCPServerBase
	{
	public:
		CMCPServer(const CMCPServer&) = delete;
		CMCPServer& operator=(const CMCPServer&) = delete;

		static CMCPServer& GetInstance()
		{
			return T::s_Instance;
		}

		virtual int Initialize() = 0;
		virtual int Uninitialize() = 0;

	protected:
		CMCPServer() = default;
		~CMCPServer() = default;
	};
}
