#include "../TestHarness.h"
#include "../IntegrationTests/TestServer.h"

#include <Message/Request.h>
#include <Public/PublicDef.h>
#include <Internal/Protocol.h>
#include <Internal/SessionAccess.h>
#include <Internal/VersionExtras.h>
#include <Session/Session.h>

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

using namespace MCP;
using namespace MCP::Test;

namespace
{
	class CAppToolTask : public MCP::ProcessCallToolRequest
	{
	public:
		CAppToolTask(const std::shared_ptr<MCP::Request>& spRequest)
			: ProcessCallToolRequest(spRequest)
		{
		}

		std::shared_ptr<MCP::CMCPTask> Clone() const override
		{
			auto spClone = std::make_shared<CAppToolTask>(nullptr);
			if (spClone)
				*spClone = *this;

			return spClone;
		}

		int Execute() override
		{
			return MCP::ERRNO_OK;
		}
	};

	MCP::AppDefinition MakeValidApp(const std::string& strToolName, const std::string& strResourceUri,
		const std::string& strHtml)
	{
		MCP::AppDefinition app;
		app.tool.strName = strToolName;
		app.tool.jInputSchema = Json::Value(Json::objectValue);
		app.resource.strName = "One-stop App";
		app.resource.strUri = strResourceUri;
		app.spToolTask = std::make_shared<CAppToolTask>(nullptr);
		app.strHtml = strHtml;

		return app;
	}

	const MCP::Tool* FindRegisteredTool(const std::vector<MCP::Tool>& vecTools, const std::string& strName)
	{
		for (const auto& tool : vecTools)
		{
			if (tool.strName == strName)
				return &tool;
		}

		return nullptr;
	}

	const MCP::Resource* FindRegisteredResource(const std::vector<MCP::Resource>& vecResources, const std::string& strUri)
	{
		for (const auto& resource : vecResources)
		{
			if (resource.strUri == strUri)
				return &resource;
		}

		return nullptr;
	}
}


TEST_CASE("unit.apps")
{
	const std::shared_ptr<const ProtocolProfile> pModernProfile = ProtocolRegistryAccess::Get(CMCPSession::GetInstance()).Find(PROTOCOL_VER_2026_07_28);
	CHECK(pModernProfile != nullptr);
	if (!pModernProfile)
		return;

	ProtocolContext ctxModern;
	ctxModern.strVersion = pModernProfile->strVersion;
	ctxModern.pProfile = pModernProfile;
	ProtocolContext ctxLegacy;

	Tool tool;
	tool.strName = "echo_app";
	tool.jInputSchema = Json::Value(Json::objectValue);
	tool.SetUiResourceUri("ui://echo/app.html");
	tool.SetUiVisibility({ UI_VISIBILITY_MODEL, UI_VISIBILITY_APP });

	Json::Value jTool;
	CHECK_EQ(ERRNO_OK, tool.DoSerialize(jTool));
	VersionExtrasAccess::Serialize(tool, jTool, ctxModern);
	CHECK_EQ(std::string("ui://echo/app.html"),
		jTool[MSG_KEY_META][MSG_KEY_UI][MSG_KEY_RESOURCE_URI].asString());
	CHECK_EQ(static_cast<Json::ArrayIndex>(2),
		jTool[MSG_KEY_META][MSG_KEY_UI][MSG_KEY_VISIBILITY].size());
	CHECK_EQ(std::string(UI_VISIBILITY_MODEL),
		jTool[MSG_KEY_META][MSG_KEY_UI][MSG_KEY_VISIBILITY][0].asString());

	Json::Value jLegacyTool;
	CHECK_EQ(ERRNO_OK, tool.DoSerialize(jLegacyTool));
	VersionExtrasAccess::Serialize(tool, jLegacyTool, ctxLegacy);
	CHECK(!jLegacyTool.isMember(MSG_KEY_META));

	Resource resource;
	resource.strName = "Echo App";
	resource.strUri = "ui://echo/app.html";
	resource.strMimeType = "text/html;profile=mcp-app";
	UiResourceMeta resourceUiMeta;
	resourceUiMeta.bClipboardWrite = true;
	resource.SetUiMeta(resourceUiMeta);

	Json::Value jResource;
	CHECK_EQ(ERRNO_OK, resource.DoSerialize(jResource));
	CHECK_EQ(std::string("ui://echo/app.html"), jResource[MSG_KEY_URI].asString());
	CHECK_EQ(std::string("text/html;profile=mcp-app"), jResource[MSG_KEY_MIMETYPE].asString());
	VersionExtrasAccess::Serialize(resource, jResource, ctxModern);
	CHECK(jResource[MSG_KEY_META][MSG_KEY_UI][MSG_KEY_PERMISSIONS][MSG_KEY_CLIPBOARD_WRITE].asBool());

	// Legacy serialization must not grow the UI metadata.
	Json::Value jLegacyResource;
	CHECK_EQ(ERRNO_OK, resource.DoSerialize(jLegacyResource));
	VersionExtrasAccess::Serialize(resource, jLegacyResource, ctxLegacy);
	CHECK(!jLegacyResource.isMember(MSG_KEY_META));

	// ui:// resources registered through the convenience API get the MCP App
	// MIME type by default.
	CTestServer& server = CTestServer::s_Instance;
	CMCPSession& session = CMCPSession::GetInstance();
	const MCP::ServerCapabilities savedCapabilities = session.GetServerCapabilities();
	const std::vector<MCP::Tool> vecSavedTools = session.GetServerTools();
	const std::vector<MCP::Resource> vecSavedResources = session.GetServerResources();
	const std::shared_ptr<MCP::CMCPTransport> spSavedTransport = session.GetTransport();

	MCP::Resource defaultMimeResource;
	defaultMimeResource.strName = "Echo App";
	defaultMimeResource.strUri = "ui://echo/app.html";
	server.RegisterServerUiResources({ defaultMimeResource }, false);
	const std::vector<MCP::Resource> vecDefaultMime = session.GetServerResources();
	CHECK_EQ(static_cast<size_t>(1), vecDefaultMime.size());
	if (!vecDefaultMime.empty())
		CHECK_EQ(std::string(MIME_TYPE_MCP_APP), vecDefaultMime[0].strMimeType);

	// Start() rejects every configuration the Apps specification forbids.
	MCP::ServerCapabilities appsCapabilities;
	appsCapabilities.mapExtensions[EXTENSION_UI] = Json::Value(Json::objectValue);
	session.SetServerCapabilities(appsCapabilities);

	MCP::Resource uiResource;
	uiResource.strName = "Echo App";
	uiResource.strUri = "ui://echo/app.html";
	uiResource.strMimeType = MIME_TYPE_MCP_APP;

	// A ui:// resource must carry the MCP App MIME type.
	MCP::Resource wrongMimeResource = uiResource;
	wrongMimeResource.strMimeType = "text/html";
	session.SetServerResources({ wrongMimeResource });
	session.SetServerTools({});
	CHECK_EQ(ERRNO_INVALID_PARAMS, server.Start());

	// A tool resource URI must use the ui:// scheme.
	MCP::Tool nonUiTool;
	nonUiTool.strName = "echo_app";
	nonUiTool.SetUiResourceUri("https://example.com/app.html");
	session.SetServerResources({ uiResource });
	session.SetServerTools({ nonUiTool });
	CHECK_EQ(ERRNO_INVALID_PARAMS, server.Start());

	// A tool resource URI must reference a registered resource.
	MCP::Tool missingTool;
	missingTool.strName = "echo_app";
	missingTool.SetUiResourceUri("ui://echo/missing.html");
	session.SetServerTools({ missingTool });
	CHECK_EQ(ERRNO_INVALID_PARAMS, server.Start());

	// Visibility only accepts the documented values.
	MCP::Tool visibilityTool;
	visibilityTool.strName = "echo_app";
	visibilityTool.SetUiResourceUri("ui://echo/app.html");
	visibilityTool.SetUiVisibility({ UI_VISIBILITY_MODEL, "host" });
	session.SetServerTools({ visibilityTool });
	CHECK_EQ(ERRNO_INVALID_PARAMS, server.Start());

	session.SetServerCapabilities(savedCapabilities);
	session.SetServerTools(vecSavedTools);
	session.SetServerResources(vecSavedResources);
	session.SetTransport(spSavedTransport);
}

TEST_CASE("unit.apps")
{
	CTestServer& server = CTestServer::s_Instance;
	CMCPSession& session = CMCPSession::GetInstance();
	const MCP::ServerCapabilities savedCapabilities = session.GetServerCapabilities();
	const std::vector<MCP::Tool> savedTools = session.GetServerTools();
	const std::vector<MCP::Resource> savedResources = session.GetServerResources();
	const std::shared_ptr<MCP::CMCPTransport> savedTransport = session.GetTransport();

	session.SetServerCapabilities(MCP::ServerCapabilities());
	session.SetServerResources({});

	// A previously registered tool must survive the one-stop registration.
	MCP::Tool existingTool;
	existingTool.strName = "existing";
	existingTool.jInputSchema = Json::Value(Json::objectValue);
	server.RegisterServerTools({ existingTool }, false);

	const std::string strUri = "ui://echo/onestop.html";
	const std::string strHtml = u8"<!DOCTYPE html><p>你好 App</p>";

	// Every invalid definition is rejected before anything is registered.
	{
		MCP::AppDefinition emptyName = MakeValidApp(std::string(), strUri, strHtml);
		CHECK_EQ(ERRNO_INVALID_PARAMS, server.RegisterServerApp(emptyName));

		MCP::AppDefinition nonUi = MakeValidApp("onestop_app", "https://example.com/app.html", strHtml);
		CHECK_EQ(ERRNO_INVALID_PARAMS, server.RegisterServerApp(nonUi));

		MCP::AppDefinition wrongMime = MakeValidApp("onestop_app", strUri, strHtml);
		wrongMime.resource.strMimeType = "text/html";
		CHECK_EQ(ERRNO_INVALID_PARAMS, server.RegisterServerApp(wrongMime));

		MCP::AppDefinition emptyHtml = MakeValidApp("onestop_app", strUri, std::string());
		CHECK_EQ(ERRNO_INVALID_PARAMS, server.RegisterServerApp(emptyHtml));

		MCP::AppDefinition noTask = MakeValidApp("onestop_app", strUri, strHtml);
		noTask.spToolTask.reset();
		CHECK_EQ(ERRNO_INVALID_PARAMS, server.RegisterServerApp(noTask));

		MCP::AppDefinition mismatch = MakeValidApp("onestop_app", strUri, strHtml);
		mismatch.tool.SetUiResourceUri("ui://echo/other.html");
		CHECK_EQ(ERRNO_INVALID_PARAMS, server.RegisterServerApp(mismatch));

		CHECK_EQ(static_cast<size_t>(1), session.GetServerTools().size());
		CHECK(session.GetServerResources().empty());
		const MCP::ServerCapabilities unchanged = session.GetServerCapabilities();
		CHECK(unchanged.mapExtensions.find(EXTENSION_UI) == unchanged.mapExtensions.end());
	}

	// A valid definition declares the extension and registers every part.
	{
		MCP::AppDefinition app = MakeValidApp("onestop_app", strUri, strHtml);
		app.uiResourceMeta.bPrefersBorder = true;
		app.uiResourceMeta.bHasPrefersBorder = true;
		app.tool.SetUiVisibility({ UI_VISIBILITY_APP });
		CHECK_EQ(ERRNO_OK, server.RegisterServerApp(app));

		const MCP::ServerCapabilities capabilities = session.GetServerCapabilities();
		CHECK(capabilities.mapExtensions.find(EXTENSION_UI) != capabilities.mapExtensions.end());
		CHECK(capabilities.setModernOnlyExtensions.find(EXTENSION_UI) != capabilities.setModernOnlyExtensions.end());

		const std::vector<MCP::Tool> tools = session.GetServerTools();
		CHECK_EQ(static_cast<size_t>(2), tools.size());
		CHECK(FindRegisteredTool(tools, "existing") != nullptr);

		const MCP::Tool* pAppTool = FindRegisteredTool(tools, "onestop_app");
		CHECK(pAppTool != nullptr);
		if (pAppTool)
		{
			CHECK_EQ(strUri, pAppTool->jMeta[MSG_KEY_UI][MSG_KEY_RESOURCE_URI].asString());
			// An explicit visibility is preserved instead of being overwritten.
			CHECK_EQ(static_cast<Json::ArrayIndex>(1), pAppTool->jMeta[MSG_KEY_UI][MSG_KEY_VISIBILITY].size());
			CHECK_EQ(std::string(UI_VISIBILITY_APP),
				pAppTool->jMeta[MSG_KEY_UI][MSG_KEY_VISIBILITY][0].asString());
		}

		const std::vector<MCP::Resource> resources = session.GetServerResources();
		CHECK_EQ(static_cast<size_t>(1), resources.size());
		const MCP::Resource* pResource = FindRegisteredResource(resources, strUri);
		CHECK(pResource != nullptr);
		if (pResource)
		{
			CHECK_EQ(std::string(MIME_TYPE_MCP_APP), pResource->strMimeType);
			CHECK(pResource->jMeta[MSG_KEY_UI][MSG_KEY_PREFERS_BORDER].asBool());
		}
	}

	// Visibility defaults to {model, app} when the tool does not declare one.
	{
		MCP::AppDefinition app = MakeValidApp("onestop_default_visibility", "ui://echo/onestop2.html", strHtml);
		CHECK_EQ(ERRNO_OK, server.RegisterServerApp(app));

		const std::vector<MCP::Tool> tools = session.GetServerTools();
		const MCP::Tool* pTool = FindRegisteredTool(tools, "onestop_default_visibility");
		CHECK(pTool != nullptr);
		if (pTool)
		{
			const Json::Value& jVisibility = pTool->jMeta[MSG_KEY_UI][MSG_KEY_VISIBILITY];
			CHECK_EQ(static_cast<Json::ArrayIndex>(2), jVisibility.size());
			CHECK_EQ(std::string(UI_VISIBILITY_MODEL), jVisibility[0].asString());
			CHECK_EQ(std::string(UI_VISIBILITY_APP), jVisibility[1].asString());
		}
	}

	// RegisterServerAppFromFile reads the HTML in binary mode.
	{
		const std::string strPath = "unit_apps_view.html";
		{
			std::ofstream file(strPath.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
			file << strHtml;
		}

		MCP::AppDefinition fileApp = MakeValidApp("onestop_file_app", "ui://echo/file.html", std::string());
		CHECK_EQ(ERRNO_OK, server.RegisterServerAppFromFile(fileApp, strPath));
		std::remove(strPath.c_str());

		const std::vector<MCP::Tool> tools = session.GetServerTools();
		CHECK(FindRegisteredTool(tools, "onestop_file_app") != nullptr);
		const std::vector<MCP::Resource> resources = session.GetServerResources();
		CHECK(FindRegisteredResource(resources, "ui://echo/file.html") != nullptr);

		// A missing file is rejected.
		MCP::AppDefinition missingApp = MakeValidApp("onestop_missing", "ui://echo/missing.html", std::string());
		CHECK_EQ(ERRNO_INVALID_PARAMS, server.RegisterServerAppFromFile(missingApp, "unit_apps_missing.html"));

		// An empty file is rejected as well.
		const std::string strEmptyPath = "unit_apps_empty.html";
		{
			std::ofstream file(strEmptyPath.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
		}
		MCP::AppDefinition emptyApp = MakeValidApp("onestop_empty", "ui://echo/empty.html", std::string());
		CHECK_EQ(ERRNO_INVALID_PARAMS, server.RegisterServerAppFromFile(emptyApp, strEmptyPath));
		std::remove(strEmptyPath.c_str());

		const std::vector<MCP::Tool> afterFailures = session.GetServerTools();
		CHECK(FindRegisteredTool(afterFailures, "onestop_missing") == nullptr);
		CHECK(FindRegisteredTool(afterFailures, "onestop_empty") == nullptr);
	}

	// The built-in read task is cloned per dispatch like every other prototype.
	{
		MCP::Resource uiResource;
		uiResource.strName = "One-stop App";
		uiResource.strUri = strUri;

		MCP::ProcessStaticUiResourceReadTask prototype(nullptr, uiResource, strHtml);
		std::shared_ptr<MCP::CMCPTask> spClone = prototype.Clone();
		CHECK(spClone != nullptr);
		CHECK(spClone.get() != &prototype);
		CHECK(std::dynamic_pointer_cast<MCP::ProcessStaticUiResourceReadTask>(spClone) != nullptr);
		CHECK_EQ(ERRNO_OK, spClone->Cancel());
	}

	session.SetServerCapabilities(savedCapabilities);
	session.SetServerTools(savedTools);
	session.SetServerResources(savedResources);
	session.SetTransport(savedTransport);
}
