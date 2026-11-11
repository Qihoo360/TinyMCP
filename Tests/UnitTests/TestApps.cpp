#include "../TestHarness.h"
#include "../IntegrationTests/TestServer.h"

#include <Message/Request.h>
#include <Public/PublicDef.h>
#include <Internal/Protocol.h>
#include <Internal/SessionAccess.h>
#include <Internal/VersionExtras.h>
#include <Session/Session.h>

#include <string>
#include <vector>

using namespace MCP;
using namespace MCP::Test;

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
