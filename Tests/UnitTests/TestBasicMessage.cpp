#include "../TestHarness.h"

#include <Message/BasicMessage.h>
#include <Public/PublicDef.h>
#include <Internal/Protocol.h>
#include <Internal/VersionExtras.h>

#include <string>
#include <vector>

using namespace MCP;

namespace
{
	ProtocolContext MakeLegacyContext()
	{
		return ProtocolContext();
	}

	ProtocolContext MakeModernContext()
	{
		ProtocolContext ctxProtocol;
		const std::shared_ptr<const ProtocolProfile> pProfile = CProtocolRegistry::GetGlobalRegistry().Find(PROTOCOL_VER_2026_07_28);
		if (pProfile)
		{
			ctxProtocol.strVersion = pProfile->strVersion;
			ctxProtocol.pProfile = pProfile;
		}
		return ctxProtocol;
	}

	Icon MakeTestIcon()
	{
		Icon icon;
		icon.strSrc = "https://example.com/icon.png";
		icon.strMimeType = "image/png";
		icon.vecSizes.push_back("64x64");
		icon.strTheme = "dark";
		return icon;
	}
}

TEST_CASE("unit.message")
{
	const ProtocolContext ctxLegacy = MakeLegacyContext();
	const ProtocolContext ctxModern = MakeModernContext();

	// Implementation: legacy output stays byte compatible even when extras exist.
	Implementation impl;
	impl.strName = "tinymcp";
	impl.strVersion = "1.0.0";
	impl.strTitle = "Tiny MCP";
	impl.strDescription = "Server SDK";
	impl.strWebsiteUrl = "https://example.com";
	impl.vecIcons.push_back(MakeTestIcon());

	Json::Value jLegacyImpl(Json::objectValue);
	impl.DoSerialize(jLegacyImpl);
	VersionExtrasAccess::Serialize(impl, jLegacyImpl, ctxLegacy);

	Json::Value jExpectedImpl(Json::objectValue);
	jExpectedImpl[MSG_KEY_NAME] = "tinymcp";
	jExpectedImpl[MSG_KEY_VERSION] = "1.0.0";
	CHECK_JSON_EQ(jLegacyImpl, jExpectedImpl);

	Json::Value jModernImpl(Json::objectValue);
	impl.DoSerialize(jModernImpl);
	VersionExtrasAccess::Serialize(impl, jModernImpl, ctxModern);
	CHECK_EQ(std::string("Tiny MCP"), jModernImpl[MSG_KEY_TITLE].asString());
	CHECK_EQ(std::string("Server SDK"), jModernImpl[MSG_KEY_DESCRIPTION].asString());
	CHECK_EQ(std::string("https://example.com"), jModernImpl[MSG_KEY_WEBSITE_URL].asString());
	CHECK(jModernImpl[MSG_KEY_ICONS].isArray());
	CHECK_EQ(static_cast<Json::ArrayIndex>(1), jModernImpl[MSG_KEY_ICONS].size());
	CHECK_EQ(std::string("https://example.com/icon.png"), jModernImpl[MSG_KEY_ICONS][0][MSG_KEY_SRC].asString());
	CHECK_EQ(std::string("image/png"), jModernImpl[MSG_KEY_ICONS][0][MSG_KEY_MIMETYPE].asString());
	CHECK_EQ(std::string("64x64"), jModernImpl[MSG_KEY_ICONS][0][MSG_KEY_SIZES][0].asString());
	CHECK_EQ(std::string("dark"), jModernImpl[MSG_KEY_ICONS][0][MSG_KEY_THEME].asString());

	// Empty optional fields must not appear in the output.
	Implementation implEmpty;
	implEmpty.strName = "tinymcp";
	implEmpty.strVersion = "1.0.0";
	Json::Value jEmptyImpl(Json::objectValue);
	implEmpty.DoSerialize(jEmptyImpl);
	VersionExtrasAccess::Serialize(implEmpty, jEmptyImpl, ctxModern);
	CHECK(!jEmptyImpl.isMember(MSG_KEY_TITLE));
	CHECK(!jEmptyImpl.isMember(MSG_KEY_DESCRIPTION));
	CHECK(!jEmptyImpl.isMember(MSG_KEY_ICONS));
	CHECK(!jEmptyImpl.isMember(MSG_KEY_WEBSITE_URL));

	// Modern extras round trip.
	Implementation implParsed;
	CHECK_EQ(ERRNO_OK, implParsed.DoDeserialize(jModernImpl));
	CHECK_EQ(ERRNO_OK, VersionExtrasAccess::Deserialize(implParsed, jModernImpl, ctxModern));
	CHECK_EQ(std::string("Tiny MCP"), implParsed.strTitle);
	CHECK_EQ(static_cast<size_t>(1), implParsed.vecIcons.size());

	// Tool: base fields never leak the new keys for the legacy profile.
	Tool tool;
	tool.strName = "echo";
	tool.strDescription = "Echo input";
	Json::Value jInputSchema(Json::objectValue);
	jInputSchema[MSG_KEY_TYPE] = "object";
	tool.jInputSchema = jInputSchema;
	tool.strTitle = "Echo tool";
	Json::Value jOutputSchema(Json::objectValue);
	jOutputSchema[MSG_KEY_TYPE] = "object";
	tool.jOutputSchema = jOutputSchema;
	tool.annotations.strTitle = "Echo annotations";
	tool.annotations.bHasReadOnlyHint = true;
	tool.annotations.bReadOnlyHint = true;
	tool.vecIcons.push_back(MakeTestIcon());
	tool.SetUiResourceUri("ui://echo/app.html");

	Json::Value jLegacyTool(Json::objectValue);
	tool.DoSerialize(jLegacyTool);
	VersionExtrasAccess::Serialize(tool, jLegacyTool, ctxLegacy);
	CHECK(!jLegacyTool.isMember(MSG_KEY_TITLE));
	CHECK(!jLegacyTool.isMember(MSG_KEY_ICONS));
	CHECK(!jLegacyTool.isMember(MSG_KEY_OUTPUT_SCHEMA));
	CHECK(!jLegacyTool.isMember(MSG_KEY_ANNOTATIONS));
	CHECK(!jLegacyTool.isMember(MSG_KEY_META));

	Json::Value jModernTool(Json::objectValue);
	tool.DoSerialize(jModernTool);
	VersionExtrasAccess::Serialize(tool, jModernTool, ctxModern);
	CHECK_EQ(std::string("Echo tool"), jModernTool[MSG_KEY_TITLE].asString());
	CHECK_EQ(static_cast<Json::ArrayIndex>(1), jModernTool[MSG_KEY_ICONS].size());
	CHECK(jModernTool[MSG_KEY_OUTPUT_SCHEMA].isObject());
	CHECK_EQ(std::string("Echo annotations"), jModernTool[MSG_KEY_ANNOTATIONS][MSG_KEY_TITLE].asString());
	CHECK(jModernTool[MSG_KEY_ANNOTATIONS][MSG_KEY_READ_ONLY_HINT].asBool());
	CHECK_EQ(std::string("ui://echo/app.html"),
		jModernTool[MSG_KEY_META][MSG_KEY_UI][MSG_KEY_RESOURCE_URI].asString());

	// Resource, resource template and prompt share the entity extras gates.
	Resource resource;
	resource.strName = "doc";
	resource.strUri = "file:///doc.txt";
	resource.strTitle = "Doc";
	resource.annotations.vecAudience.push_back(CONST_USER);
	resource.jMeta = Json::Value(Json::objectValue);
	resource.jMeta["trace"] = "r-1";

	Json::Value jLegacyResource(Json::objectValue);
	resource.DoSerialize(jLegacyResource);
	VersionExtrasAccess::Serialize(resource, jLegacyResource, ctxLegacy);
	CHECK(!jLegacyResource.isMember(MSG_KEY_TITLE));
	// annotations exist in 2024-11-05; lastModified does not.
	CHECK_EQ(std::string(CONST_USER),
		jLegacyResource[MSG_KEY_ANNOTATIONS][MSG_KEY_AUDIENCE][0].asString());
	CHECK(!jLegacyResource[MSG_KEY_ANNOTATIONS].isMember(MSG_KEY_LAST_MODIFIED));
	CHECK(!jLegacyResource.isMember(MSG_KEY_META));

	Json::Value jModernResource(Json::objectValue);
	resource.DoSerialize(jModernResource);
	VersionExtrasAccess::Serialize(resource, jModernResource, ctxModern);
	CHECK_EQ(std::string("Doc"), jModernResource[MSG_KEY_TITLE].asString());
	CHECK_EQ(std::string(CONST_USER), jModernResource[MSG_KEY_ANNOTATIONS][MSG_KEY_AUDIENCE][0].asString());
	CHECK_EQ(std::string("r-1"), jModernResource[MSG_KEY_META]["trace"].asString());

	ResourceTemplate resourceTemplate;
	resourceTemplate.strName = "doc-template";
	resourceTemplate.strUriTemplate = "file:///{name}";
	resourceTemplate.strTitle = "Doc template";
	Json::Value jModernTemplate(Json::objectValue);
	resourceTemplate.DoSerialize(jModernTemplate);
	VersionExtrasAccess::Serialize(resourceTemplate, jModernTemplate, ctxModern);
	CHECK_EQ(std::string("Doc template"), jModernTemplate[MSG_KEY_TITLE].asString());

	Prompt prompt;
	prompt.strName = "summarize";
	prompt.strTitle = "Summarize";
	prompt.jMeta = Json::Value(Json::objectValue);
	prompt.jMeta["trace"] = "p-1";
	Json::Value jLegacyPrompt(Json::objectValue);
	prompt.DoSerialize(jLegacyPrompt);
	VersionExtrasAccess::Serialize(prompt, jLegacyPrompt, ctxLegacy);
	CHECK(!jLegacyPrompt.isMember(MSG_KEY_TITLE));
	CHECK(!jLegacyPrompt.isMember(MSG_KEY_META));

	Json::Value jModernPrompt(Json::objectValue);
	prompt.DoSerialize(jModernPrompt);
	VersionExtrasAccess::Serialize(prompt, jModernPrompt, ctxModern);
	CHECK_EQ(std::string("Summarize"), jModernPrompt[MSG_KEY_TITLE].asString());
	CHECK_EQ(std::string("p-1"), jModernPrompt[MSG_KEY_META]["trace"].asString());

	// Server capabilities: completions and extensions are feature gated.
	ServerCapabilities capabilities;
	capabilities.bCompletions = true;
	capabilities.mapExtensions[EXTENSION_UI] = Json::Value(Json::objectValue);

	Json::Value jLegacyCapabilities(Json::objectValue);
	capabilities.DoSerialize(jLegacyCapabilities);
	VersionExtrasAccess::Serialize(capabilities, jLegacyCapabilities, ctxLegacy);
	CHECK(!jLegacyCapabilities.isMember(MSG_KEY_COMPLETIONS));
	CHECK(!jLegacyCapabilities.isMember(MSG_KEY_EXTENSIONS));

	Json::Value jModernCapabilities(Json::objectValue);
	capabilities.DoSerialize(jModernCapabilities);
	VersionExtrasAccess::Serialize(capabilities, jModernCapabilities, ctxModern);
	CHECK(jModernCapabilities[MSG_KEY_COMPLETIONS].isObject());
	CHECK(jModernCapabilities[MSG_KEY_EXTENSIONS].isMember(EXTENSION_UI));

	// A legacy handshake never advertises extensions, no matter what the
	// application registered, and an empty extension map stays absent.
	Json::Value jLegacyExtensions(Json::objectValue);
	VersionExtrasAccess::Serialize(capabilities, jLegacyExtensions, ctxLegacy);
	CHECK(!jLegacyExtensions.isMember(MSG_KEY_EXTENSIONS));

	ServerCapabilities emptyCapabilities;
	Json::Value jEmptyCapabilities(Json::objectValue);
	VersionExtrasAccess::Serialize(emptyCapabilities, jEmptyCapabilities, ctxModern);
	CHECK(!jEmptyCapabilities.isMember(MSG_KEY_EXTENSIONS));

	// An explicitly registered modern-only extension never reaches legacy output,
	// while other passthrough extensions keep their legacy advertisement.
	ServerCapabilities passthroughCapabilities;
	passthroughCapabilities.bExtensionPassthrough = true;
	passthroughCapabilities.mapExtensions[EXTENSION_UI] = Json::Value(Json::objectValue);
	passthroughCapabilities.mapExtensions["example/echo"] = Json::Value(Json::objectValue);
	passthroughCapabilities.setModernOnlyExtensions.insert(EXTENSION_UI);

	Json::Value jPassthroughLegacy(Json::objectValue);
	VersionExtrasAccess::Serialize(passthroughCapabilities, jPassthroughLegacy, ctxLegacy);
	CHECK(jPassthroughLegacy[MSG_KEY_EXTENSIONS].isMember("example/echo"));
	CHECK(!jPassthroughLegacy[MSG_KEY_EXTENSIONS].isMember(EXTENSION_UI));

	Json::Value jPassthroughModern(Json::objectValue);
	VersionExtrasAccess::Serialize(passthroughCapabilities, jPassthroughModern, ctxModern);
	CHECK(jPassthroughModern[MSG_KEY_EXTENSIONS].isMember(EXTENSION_UI));

	// Capability deserialization keeps unknown extension payloads intact.
	ServerCapabilities parsedCapabilities;
	Json::Value jParsedCapabilities(Json::objectValue);
	Json::Value jParsedExtensions(Json::objectValue);
	jParsedExtensions[EXTENSION_UI] = Json::Value(Json::objectValue);
	jParsedExtensions[EXTENSION_UI]["mimeTypes"] = Json::Value(Json::arrayValue);
	Json::Value jParsedCompletions(Json::objectValue);
	jParsedCapabilities[MSG_KEY_COMPLETIONS] = jParsedCompletions;
	jParsedCapabilities[MSG_KEY_EXTENSIONS] = jParsedExtensions;
	CHECK_EQ(ERRNO_OK, parsedCapabilities.DoDeserialize(jParsedCapabilities));
	CHECK(parsedCapabilities.bCompletions);
	CHECK_EQ(static_cast<size_t>(1), parsedCapabilities.mapExtensions.size());
	CHECK(parsedCapabilities.mapExtensions[EXTENSION_UI].isMember("mimeTypes"));

	// Message identity fields and serialization stability.
	TextContent text;
	text.strType = CONST_TEXT;
	text.strText = "hello";
	CHECK(text.IsValid());

	std::string strFirst;
	std::string strSecond;
	CHECK_EQ(ERRNO_OK, text.Serialize(strFirst));
	CHECK_EQ(ERRNO_OK, text.Serialize(strSecond));
	CHECK_EQ(strFirst, strSecond);

	// MRTR containers only accept non-empty object maps.
	InputRequests inputRequests;
	CHECK(!inputRequests.IsValid());
	CHECK(!inputRequests.HasValue());
	CHECK(!inputRequests.Get().isObject());
	Json::Value jInputRequests(Json::objectValue);
	jInputRequests["req-1"] = Json::Value(Json::objectValue);
	CHECK_EQ(ERRNO_OK, inputRequests.DoDeserialize(jInputRequests));
	CHECK(inputRequests.IsValid());
	CHECK(inputRequests.Has("req-1"));
	CHECK(!inputRequests.Has("req-2"));
	CHECK(inputRequests.Get()["req-1"].isObject());
	CHECK_EQ(ERRNO_PARSE_ERROR, inputRequests.DoDeserialize(Json::Value(1)));
	inputRequests.Set(Json::Value("not-an-object"));
	CHECK(!inputRequests.HasValue());
	inputRequests.Set(jInputRequests);
	CHECK(inputRequests.HasValue());
	inputRequests.Clear();
	CHECK(!inputRequests.HasValue());

	InputResponses inputResponses;
	CHECK(!inputResponses.IsValid());
	CHECK(!inputResponses.HasValue());
	inputResponses.Set(jInputRequests);
	CHECK(inputResponses.HasValue());
	CHECK(inputResponses.Has("req-1"));
	CHECK_EQ(ERRNO_OK, inputResponses.DoSerialize(jInputRequests));
	inputResponses.Clear();
	CHECK(!inputResponses.HasValue());

	// structuredContent accepts any JSON value defined by 2026-07-28: object,
	// array, string, number, boolean, or null.
	StructuredContent structuredContent;
	CHECK(!structuredContent.HasValue());
	CHECK(structuredContent.IsValid());
	structuredContent.Set(Json::Value(42));
	CHECK(structuredContent.HasValue());
	CHECK(structuredContent.Get().isInt());
	CHECK_EQ(42, structuredContent.Get().asInt());

	structuredContent.Set(Json::Value(Json::objectValue));
	CHECK(structuredContent.HasValue());
	CHECK(structuredContent.Get().isObject());

	structuredContent.Set(Json::Value());
	CHECK(structuredContent.HasValue());
	CHECK(structuredContent.Get().isNull());

	Json::Value jStructuredArray(Json::arrayValue);
	jStructuredArray.append(Json::Value(1));
	jStructuredArray.append(Json::Value(2));
	structuredContent.Set(jStructuredArray);
	CHECK(structuredContent.HasValue());
	CHECK(structuredContent.Get().isArray());
	CHECK_EQ(static_cast<Json::ArrayIndex>(2), structuredContent.Get().size());

	Json::Value jStructuredValue(Json::objectValue);
	jStructuredValue[MSG_KEY_VALUE] = 42;
	structuredContent.Set(jStructuredValue);
	CHECK(structuredContent.HasValue());
	CHECK(structuredContent.IsValid());
	CHECK_EQ(42, structuredContent.Get()[MSG_KEY_VALUE].asInt());

	Json::Value jStructuredRoundTrip;
	CHECK_EQ(ERRNO_OK, structuredContent.DoSerialize(jStructuredRoundTrip));
	CHECK_EQ(42, jStructuredRoundTrip[MSG_KEY_VALUE].asInt());
	CHECK_EQ(ERRNO_OK, structuredContent.DoDeserialize(jStructuredArray));
	CHECK(structuredContent.HasValue());
	CHECK(structuredContent.Get().isArray());
	CHECK_EQ(ERRNO_OK, structuredContent.DoDeserialize(Json::Value("text")));
	CHECK(structuredContent.Get().isString());
	structuredContent.Clear();
	CHECK(!structuredContent.HasValue());

	// Tool visibility keeps the resource URI and other _meta.ui keys.
	Tool visibilityTool;
	visibilityTool.SetUiResourceUri("ui://echo/app.html");
	visibilityTool.jMeta[MSG_KEY_UI]["video"] = true;
	visibilityTool.SetUiVisibility({ UI_VISIBILITY_MODEL, UI_VISIBILITY_APP });
	CHECK_EQ(std::string("ui://echo/app.html"),
		visibilityTool.jMeta[MSG_KEY_UI][MSG_KEY_RESOURCE_URI].asString());
	CHECK(visibilityTool.jMeta[MSG_KEY_UI]["video"].asBool());
	CHECK_EQ(static_cast<Json::ArrayIndex>(2),
		visibilityTool.jMeta[MSG_KEY_UI][MSG_KEY_VISIBILITY].size());
	CHECK(!visibilityTool.jMeta.isMember("ui/resourceUri"));

	// UiResourceMeta models every documented field and passes unknown keys through.
	UiResourceMeta uiMeta;
	uiMeta.vecConnectDomains.push_back("https://api.example.com");
	uiMeta.vecResourceDomains.push_back("https://cdn.example.com");
	uiMeta.vecFrameDomains.push_back("https://frame.example.com");
	uiMeta.vecBaseUriDomains.push_back("https://base.example.com");
	uiMeta.bCamera = true;
	uiMeta.bClipboardWrite = true;
	uiMeta.strDomain = "example.com";
	uiMeta.bHasDomain = true;
	uiMeta.bPrefersBorder = true;
	uiMeta.bHasPrefersBorder = true;
	uiMeta.jExtra[MSG_KEY_PERMISSIONS] = Json::Value(Json::objectValue);
	uiMeta.jExtra[MSG_KEY_PERMISSIONS][MSG_KEY_MICROPHONE] = true;
	uiMeta.jExtra["futureFlag"] = "keep";
	CHECK(uiMeta.HasValue());

	Json::Value jUiMeta(Json::objectValue);
	CHECK_EQ(ERRNO_OK, uiMeta.DoSerialize(jUiMeta));
	CHECK_EQ(std::string("https://api.example.com"),
		jUiMeta[MSG_KEY_CSP][MSG_KEY_CONNECT_DOMAINS][0].asString());
	CHECK_EQ(std::string("https://cdn.example.com"),
		jUiMeta[MSG_KEY_CSP][MSG_KEY_RESOURCE_DOMAINS][0].asString());
	CHECK_EQ(std::string("https://frame.example.com"),
		jUiMeta[MSG_KEY_CSP][MSG_KEY_FRAME_DOMAINS][0].asString());
	CHECK_EQ(std::string("https://base.example.com"),
		jUiMeta[MSG_KEY_CSP][MSG_KEY_BASE_URI_DOMAINS][0].asString());
	CHECK(jUiMeta[MSG_KEY_PERMISSIONS][MSG_KEY_CAMERA].asBool());
	CHECK(jUiMeta[MSG_KEY_PERMISSIONS][MSG_KEY_CLIPBOARD_WRITE].asBool());
	CHECK(jUiMeta[MSG_KEY_PERMISSIONS][MSG_KEY_MICROPHONE].asBool());
	CHECK(!jUiMeta[MSG_KEY_PERMISSIONS].isMember(MSG_KEY_GEOLOCATION));
	CHECK_EQ(std::string("example.com"), jUiMeta[MSG_KEY_DOMAIN].asString());
	CHECK(jUiMeta[MSG_KEY_PREFERS_BORDER].asBool());
	CHECK_EQ(std::string("keep"), jUiMeta["futureFlag"].asString());

	// Resource and contents setters keep unmodeled _meta keys.
	Resource uiResource;
	uiResource.strName = "Echo App";
	uiResource.strUri = "ui://echo/app.html";
	uiResource.jMeta["trace"] = "keep";
	uiResource.SetUiMeta(uiMeta);
	CHECK_EQ(std::string("keep"), uiResource.jMeta["trace"].asString());
	CHECK_EQ(std::string("example.com"), uiResource.jMeta[MSG_KEY_UI][MSG_KEY_DOMAIN].asString());

	TextResourceContents uiContents;
	uiContents.SetUiMeta(uiMeta);
	CHECK_EQ(std::string("example.com"), uiContents.jMeta[MSG_KEY_UI][MSG_KEY_DOMAIN].asString());

	// BuildUiResourceContents copies uri, mime type and the whole _meta.ui tree.
	Resource listedUiResource;
	listedUiResource.strUri = "ui://echo/app.html";
	listedUiResource.strMimeType = MIME_TYPE_MCP_APP;
	listedUiResource.SetUiMeta(uiMeta);
	TextResourceContents builtContents;
	CHECK_EQ(ERRNO_OK, BuildUiResourceContents(listedUiResource, "<html></html>", builtContents));
	CHECK_EQ(std::string("ui://echo/app.html"), builtContents.strUri);
	CHECK_EQ(std::string(MIME_TYPE_MCP_APP), builtContents.strMimeType);
	CHECK_EQ(std::string("<html></html>"), builtContents.strText);
	CHECK_EQ(std::string("example.com"), builtContents.jMeta[MSG_KEY_UI][MSG_KEY_DOMAIN].asString());
	CHECK_EQ(std::string("keep"),
		builtContents.jMeta[MSG_KEY_UI]["futureFlag"].asString());
}

TEST_CASE("unit.content_blocks")
{
	const ProtocolContext ctxLegacy = MakeLegacyContext();
	const ProtocolContext ctxModern = MakeModernContext();

	// Text content: annotations exist in every revision, _meta is modern only.
	TextContent text;
	text.strType = CONST_TEXT;
	text.strText = "hello";
	text.annotations.vecAudience.push_back(CONST_USER);
	text.annotations.bHasPriority = true;
	text.annotations.dPriority = 0.5;
	text.annotations.strLastModified = "2026-01-01T00:00:00Z";
	text.jMeta = Json::Value(Json::objectValue);
	text.jMeta["trace"] = "abc";

	Json::Value jLegacyText(Json::objectValue);
	text.DoSerialize(jLegacyText);
	VersionExtrasAccess::Serialize(text, jLegacyText, ctxLegacy);
	CHECK_EQ(std::string(CONST_USER), jLegacyText[MSG_KEY_ANNOTATIONS][MSG_KEY_AUDIENCE][0].asString());
	CHECK(jLegacyText[MSG_KEY_ANNOTATIONS][MSG_KEY_PRIORITY].asDouble() > 0.49);
	CHECK(!jLegacyText[MSG_KEY_ANNOTATIONS].isMember(MSG_KEY_LAST_MODIFIED));
	CHECK(!jLegacyText.isMember(MSG_KEY_META));

	Json::Value jModernText(Json::objectValue);
	text.DoSerialize(jModernText);
	VersionExtrasAccess::Serialize(text, jModernText, ctxModern);
	CHECK_EQ(std::string(CONST_USER), jModernText[MSG_KEY_ANNOTATIONS][MSG_KEY_AUDIENCE][0].asString());
	CHECK(jModernText[MSG_KEY_ANNOTATIONS][MSG_KEY_PRIORITY].asDouble() > 0.49);
	CHECK_EQ(std::string("2026-01-01T00:00:00Z"),
		jModernText[MSG_KEY_ANNOTATIONS][MSG_KEY_LAST_MODIFIED].asString());
	CHECK_EQ(std::string("abc"), jModernText[MSG_KEY_META]["trace"].asString());

	// Empty text stays invalid.
	TextContent emptyText;
	emptyText.strType = CONST_TEXT;
	CHECK(!emptyText.IsValid());

	ImageContent image;
	image.strType = CONST_IMAGE;
	image.strMimeType = "image/png";
	image.strData = "AAAA";
	CHECK(image.IsValid());

	// Audio content.
	AudioContent audio;
	audio.strType = CONST_AUDIO;
	audio.strData = "AAAA";
	audio.strMimeType = "audio/wav";
	CHECK(audio.IsValid());

	Json::Value jAudio(Json::objectValue);
	CHECK_EQ(ERRNO_OK, audio.DoSerialize(jAudio));
	CHECK_EQ(std::string(CONST_AUDIO), jAudio[MSG_KEY_TYPE].asString());
	CHECK_EQ(std::string("AAAA"), jAudio[MSG_KEY_DATA].asString());
	CHECK_EQ(std::string("audio/wav"), jAudio[MSG_KEY_MIMETYPE].asString());

	AudioContent parsedAudio;
	CHECK_EQ(ERRNO_OK, parsedAudio.DoDeserialize(jAudio));
	CHECK(parsedAudio.IsValid());

	AudioContent invalidAudio;
	invalidAudio.strType = CONST_AUDIO;
	CHECK(!invalidAudio.IsValid());
	CHECK_EQ(ERRNO_PARSE_ERROR, invalidAudio.DoDeserialize(Json::Value(Json::objectValue)));

	// Resource link content.
	ResourceLink link;
	link.strType = CONST_RESOURCE_LINK;
	link.strName = "readme";
	link.strUri = "file:///readme.md";
	link.strTitle = "README";
	link.strDescription = "Project readme";
	link.strMimeType = "text/markdown";
	link.iSize = 12;
	link.vecIcons.push_back(MakeTestIcon());
	CHECK(link.IsValid());

	Json::Value jLink(Json::objectValue);
	CHECK_EQ(ERRNO_OK, link.DoSerialize(jLink));
	CHECK_EQ(std::string(CONST_RESOURCE_LINK), jLink[MSG_KEY_TYPE].asString());
	CHECK_EQ(std::string("readme"), jLink[MSG_KEY_NAME].asString());
	CHECK_EQ(std::string("file:///readme.md"), jLink[MSG_KEY_URI].asString());
	CHECK_EQ(std::string("README"), jLink[MSG_KEY_TITLE].asString());
	CHECK_EQ(std::string("text/markdown"), jLink[MSG_KEY_MIMETYPE].asString());
	CHECK_EQ(12, jLink[MSG_KEY_SIZE].asInt());
	CHECK_EQ(static_cast<Json::ArrayIndex>(1), jLink[MSG_KEY_ICONS].size());

	ResourceLink parsedLink;
	CHECK_EQ(ERRNO_OK, parsedLink.DoDeserialize(jLink));
	CHECK(parsedLink.IsValid());
	CHECK_EQ(std::string("README"), parsedLink.strTitle);

	ResourceLink invalidLink;
	invalidLink.strType = CONST_RESOURCE_LINK;
	CHECK(!invalidLink.IsValid());

	// Tool use content.
	ToolUseContent toolUse;
	toolUse.strType = CONST_TOOL_USE;
	toolUse.strId = "call-1";
	toolUse.strName = "echo";
	toolUse.jInput = Json::Value(Json::objectValue);
	toolUse.jInput["text"] = "hi";
	CHECK(toolUse.IsValid());

	Json::Value jToolUse(Json::objectValue);
	CHECK_EQ(ERRNO_OK, toolUse.DoSerialize(jToolUse));
	CHECK_EQ(std::string(CONST_TOOL_USE), jToolUse[MSG_KEY_TYPE].asString());
	CHECK_EQ(std::string("call-1"), jToolUse[MSG_KEY_ID].asString());
	CHECK_EQ(std::string("echo"), jToolUse[MSG_KEY_NAME].asString());
	CHECK_EQ(std::string("hi"), jToolUse[MSG_KEY_INPUT]["text"].asString());

	ToolUseContent parsedToolUse;
	CHECK_EQ(ERRNO_OK, parsedToolUse.DoDeserialize(jToolUse));
	CHECK(parsedToolUse.IsValid());

	ToolUseContent invalidToolUse;
	invalidToolUse.strType = CONST_TOOL_USE;
	CHECK(!invalidToolUse.IsValid());

	// Tool result content.
	ToolResultContent toolResult;
	toolResult.strType = CONST_TOOL_RESULT;
	toolResult.strToolUseId = "call-1";
	toolResult.jContent = Json::Value(Json::arrayValue);
	Json::Value jTextBlock(Json::objectValue);
	jTextBlock[MSG_KEY_TYPE] = CONST_TEXT;
	jTextBlock[MSG_KEY_TEXT] = "ok";
	toolResult.jContent.append(jTextBlock);
	toolResult.bHasIsError = true;
	toolResult.bIsError = false;
	Json::Value jToolStructured(Json::objectValue);
	jToolStructured[MSG_KEY_TEXT] = "ok";
	toolResult.structuredContent.Set(jToolStructured);
	CHECK(toolResult.IsValid());

	Json::Value jLegacyToolResult(Json::objectValue);
	toolResult.DoSerialize(jLegacyToolResult);
	VersionExtrasAccess::Serialize(toolResult, jLegacyToolResult, ctxLegacy);
	CHECK(!jLegacyToolResult.isMember(MSG_KEY_STRUCTURED_CONTENT));
	CHECK(!jLegacyToolResult.isMember(MSG_KEY_META));
	CHECK(jLegacyToolResult.isMember(MSG_KEY_IS_ERROR));

	Json::Value jModernToolResult(Json::objectValue);
	toolResult.DoSerialize(jModernToolResult);
	VersionExtrasAccess::Serialize(toolResult, jModernToolResult, ctxModern);
	CHECK_EQ(std::string(CONST_TOOL_RESULT), jModernToolResult[MSG_KEY_TYPE].asString());
	CHECK_EQ(std::string("call-1"), jModernToolResult[MSG_KEY_TOOL_USE_ID].asString());
	CHECK_EQ(static_cast<Json::ArrayIndex>(1), jModernToolResult[MSG_KEY_CONTENT].size());
	CHECK_EQ(std::string("ok"), jModernToolResult[MSG_KEY_STRUCTURED_CONTENT][MSG_KEY_TEXT].asString());

	// Resource contents only add _meta.
	TextResourceContents textResource;
	textResource.strText = "x";
	textResource.strUri = "file:///x";
	textResource.jMeta = Json::Value(Json::objectValue);
	textResource.jMeta["k"] = "v";

	Json::Value jLegacyTextResource(Json::objectValue);
	textResource.DoSerialize(jLegacyTextResource);
	VersionExtrasAccess::Serialize(textResource, jLegacyTextResource, ctxLegacy);
	CHECK(!jLegacyTextResource.isMember(MSG_KEY_META));

	Json::Value jModernTextResource(Json::objectValue);
	textResource.DoSerialize(jModernTextResource);
	VersionExtrasAccess::Serialize(textResource, jModernTextResource, ctxModern);
	CHECK_EQ(std::string("v"), jModernTextResource[MSG_KEY_META]["k"].asString());

	BlobResourceContents blobResource;
	blobResource.strBlob = "AAAA";
	blobResource.strUri = "file:///y";
	blobResource.jMeta = Json::Value(Json::objectValue);
	blobResource.jMeta["k"] = "v";

	Json::Value jLegacyBlobResource(Json::objectValue);
	blobResource.DoSerialize(jLegacyBlobResource);
	VersionExtrasAccess::Serialize(blobResource, jLegacyBlobResource, ctxLegacy);
	CHECK(!jLegacyBlobResource.isMember(MSG_KEY_META));

	Json::Value jModernBlobResource(Json::objectValue);
	blobResource.DoSerialize(jModernBlobResource);
	VersionExtrasAccess::Serialize(blobResource, jModernBlobResource, ctxModern);
	CHECK_EQ(std::string("v"), jModernBlobResource[MSG_KEY_META]["k"].asString());

	// Embedded resources propagate extras into the nested resource object.
	EmbeddedResource embedded;
	embedded.strType = CONST_RESOURCE;
	embedded.textResource = textResource;
	CHECK(embedded.IsValid());

	Json::Value jLegacyEmbedded(Json::objectValue);
	embedded.DoSerialize(jLegacyEmbedded);
	VersionExtrasAccess::Serialize(embedded, jLegacyEmbedded, ctxLegacy);
	CHECK(!jLegacyEmbedded[MSG_KEY_RESOURCE].isMember(MSG_KEY_META));

	Json::Value jModernEmbedded(Json::objectValue);
	embedded.DoSerialize(jModernEmbedded);
	VersionExtrasAccess::Serialize(embedded, jModernEmbedded, ctxModern);
	CHECK_EQ(std::string("v"), jModernEmbedded[MSG_KEY_RESOURCE][MSG_KEY_META]["k"].asString());

	// Prompt messages forward content extras to the nested content object.
	PromptMessage promptMessage;
	promptMessage.strRole = CONST_USER;
	promptMessage.strContentType = CONST_TEXT;
	promptMessage.textContent = text;

	Json::Value jPromptMessage(Json::objectValue);
	promptMessage.DoSerialize(jPromptMessage);
	VersionExtrasAccess::Serialize(promptMessage, jPromptMessage, ctxModern);
	CHECK_EQ(std::string("abc"), jPromptMessage[MSG_KEY_CONTENT][MSG_KEY_META]["trace"].asString());

	// An empty text/blob is legal; the embedded resource must still be emitted.
	EmbeddedResource emptyTextEmbedded;
	emptyTextEmbedded.strType = CONST_RESOURCE;
	emptyTextEmbedded.textResource.strUri = "file:///empty.txt";
	CHECK(emptyTextEmbedded.IsValid());
	Json::Value jEmptyTextEmbedded(Json::objectValue);
	CHECK_EQ(ERRNO_OK, emptyTextEmbedded.DoSerialize(jEmptyTextEmbedded));
	CHECK_EQ(std::string("file:///empty.txt"),
		jEmptyTextEmbedded[MSG_KEY_RESOURCE][MSG_KEY_URI].asString());
	CHECK(jEmptyTextEmbedded[MSG_KEY_RESOURCE].isMember(MSG_KEY_TEXT));
	CHECK_EQ(std::string(""), jEmptyTextEmbedded[MSG_KEY_RESOURCE][MSG_KEY_TEXT].asString());

	EmbeddedResource emptyBlobEmbedded;
	emptyBlobEmbedded.strType = CONST_RESOURCE;
	emptyBlobEmbedded.blobResource.strUri = "file:///empty.bin";
	CHECK(emptyBlobEmbedded.IsValid());
	Json::Value jEmptyBlobEmbedded(Json::objectValue);
	CHECK_EQ(ERRNO_OK, emptyBlobEmbedded.DoSerialize(jEmptyBlobEmbedded));
	CHECK(jEmptyBlobEmbedded[MSG_KEY_RESOURCE].isMember(MSG_KEY_BLOB));
	CHECK_EQ(std::string(""), jEmptyBlobEmbedded[MSG_KEY_RESOURCE][MSG_KEY_BLOB].asString());

	// Prompt messages support every ContentBlock of 2026-07-28.
	PromptMessage audioMessage;
	audioMessage.strRole = CONST_USER;
	audioMessage.strContentType = CONST_AUDIO;
	audioMessage.audioContent.strType = CONST_AUDIO;
	audioMessage.audioContent.strData = "AAAA";
	audioMessage.audioContent.strMimeType = "audio/wav";
	CHECK(audioMessage.IsValid());

	Json::Value jAudioMessage(Json::objectValue);
	CHECK_EQ(ERRNO_OK, audioMessage.DoSerialize(jAudioMessage));
	CHECK_EQ(std::string(CONST_AUDIO), jAudioMessage[MSG_KEY_CONTENT][MSG_KEY_TYPE].asString());
	CHECK_EQ(std::string("audio/wav"), jAudioMessage[MSG_KEY_CONTENT][MSG_KEY_MIMETYPE].asString());

	PromptMessage parsedAudioMessage;
	CHECK_EQ(ERRNO_OK, parsedAudioMessage.DoDeserialize(jAudioMessage));
	CHECK_EQ(std::string(CONST_AUDIO), parsedAudioMessage.strContentType);
	CHECK_EQ(std::string("AAAA"), parsedAudioMessage.audioContent.strData);

	PromptMessage linkMessage;
	linkMessage.strRole = CONST_ASSISTANT;
	linkMessage.strContentType = CONST_RESOURCE_LINK;
	linkMessage.resourceLink.strType = CONST_RESOURCE_LINK;
	linkMessage.resourceLink.strName = "readme";
	linkMessage.resourceLink.strUri = "file:///readme.md";
	CHECK(linkMessage.IsValid());

	Json::Value jLinkMessage(Json::objectValue);
	CHECK_EQ(ERRNO_OK, linkMessage.DoSerialize(jLinkMessage));
	CHECK_EQ(std::string(CONST_RESOURCE_LINK), jLinkMessage[MSG_KEY_CONTENT][MSG_KEY_TYPE].asString());
	CHECK_EQ(std::string("file:///readme.md"), jLinkMessage[MSG_KEY_CONTENT][MSG_KEY_URI].asString());

	PromptMessage invalidMessage;
	invalidMessage.strRole = CONST_USER;
	invalidMessage.strContentType = "video";
	CHECK(!invalidMessage.IsValid());

	// The nested content array of a tool result is mandatory, even when empty.
	ToolResultContent emptyToolResult;
	emptyToolResult.strType = CONST_TOOL_RESULT;
	emptyToolResult.strToolUseId = "call-1";
	Json::Value jEmptyToolResult(Json::objectValue);
	CHECK_EQ(ERRNO_OK, emptyToolResult.DoSerialize(jEmptyToolResult));
	CHECK(jEmptyToolResult[MSG_KEY_CONTENT].isArray());
	CHECK_EQ(static_cast<Json::ArrayIndex>(0), jEmptyToolResult[MSG_KEY_CONTENT].size());

	// audience/priority/theme are closed sets; invalid values never leave the SDK.
	Annotations invalidAnnotations;
	invalidAnnotations.vecAudience.push_back("robot");
	invalidAnnotations.bHasPriority = true;
	invalidAnnotations.dPriority = 5.0;
	Json::Value jInvalidAnnotations(Json::objectValue);
	CHECK_EQ(ERRNO_OK, invalidAnnotations.DoSerialize(jInvalidAnnotations));
	CHECK(!jInvalidAnnotations.isMember(MSG_KEY_AUDIENCE));
	CHECK_EQ(1.0, jInvalidAnnotations[MSG_KEY_PRIORITY].asDouble());

	Icon invalidThemeIcon;
	invalidThemeIcon.strSrc = "https://example.com/icon.png";
	invalidThemeIcon.strTheme = "blue";
	Json::Value jInvalidThemeIcon(Json::objectValue);
	CHECK_EQ(ERRNO_OK, invalidThemeIcon.DoSerialize(jInvalidThemeIcon));
	CHECK(!jInvalidThemeIcon.isMember(MSG_KEY_THEME));

	// size is a JSON number: fractional values survive, integral ones keep the
	// legacy integer spelling.
	Resource sizedResource;
	sizedResource.strName = "n";
	sizedResource.strUri = "file:///n";
	sizedResource.iSize = 1.5;
	Json::Value jFractionalSize(Json::objectValue);
	CHECK_EQ(ERRNO_OK, sizedResource.DoSerialize(jFractionalSize));
	CHECK_EQ(1.5, jFractionalSize[MSG_KEY_SIZE].asDouble());

	sizedResource.iSize = 2;
	Json::Value jIntegralSize(Json::objectValue);
	CHECK_EQ(ERRNO_OK, sizedResource.DoSerialize(jIntegralSize));
	CHECK(jIntegralSize[MSG_KEY_SIZE].isInt());
}

TEST_CASE("unit.meta")
{
	const ProtocolContext ctxModern = MakeModernContext();

	// Modern request metadata read from a stateless request.
	Json::Value jCapabilities(Json::objectValue);
	Json::Value jElicitation(Json::objectValue);
	jElicitation[MSG_KEY_FORM] = Json::Value(Json::objectValue);
	jElicitation[MSG_KEY_URL] = Json::Value(Json::objectValue);
	jCapabilities[MSG_KEY_ELICITATION] = jElicitation;
	jCapabilities[MSG_KEY_ROOTS] = Json::Value(Json::objectValue);
	jCapabilities[MSG_KEY_SAMPLING] = Json::Value(Json::objectValue);
	Json::Value jExtensions(Json::objectValue);
	jExtensions[EXTENSION_UI] = Json::Value(Json::objectValue);
	jExtensions[EXTENSION_UI][MSG_KEY_MIME_TYPES].append(MIME_TYPE_MCP_APP);
	jExtensions["example/limited"] = Json::Value(Json::objectValue);
	jCapabilities[MSG_KEY_EXTENSIONS] = jExtensions;

	Json::Value jClientInfo(Json::objectValue);
	jClientInfo[MSG_KEY_NAME] = "test-client";
	jClientInfo[MSG_KEY_VERSION] = "1.2.3";
	jClientInfo[MSG_KEY_TITLE] = "Test client";

	Json::Value jMeta(Json::objectValue);
	jMeta[META_KEY_PROTOCOL_VERSION] = PROTOCOL_VER_2026_07_28;
	jMeta[META_KEY_CLIENT_INFO] = jClientInfo;
	jMeta[META_KEY_CLIENT_CAPABILITIES] = jCapabilities;
	jMeta[META_KEY_LOG_LEVEL] = LOGGING_LEVEL_INFO;
	jMeta[MSG_KEY_PROGRESS_TOKEN] = 7;
	jMeta[MSG_KEY_TOOL_CALL_ID] = "call-9";
	jMeta["otel.trace"] = "trace-1";

	MetaObject meta;
	CHECK_EQ(ERRNO_OK, meta.DoDeserialize(jMeta));
	CHECK_EQ(ERRNO_OK, VersionExtrasAccess::Deserialize(meta, jMeta, ctxModern));
	CHECK(meta.HasValue());
	CHECK_EQ(std::string(PROTOCOL_VER_2026_07_28), meta.strProtocolVersion);
	CHECK_EQ(std::string("test-client"), meta.clientInfo.strName);
	CHECK_EQ(std::string("1.2.3"), meta.clientInfo.strVersion);
	CHECK_EQ(std::string(LOGGING_LEVEL_INFO), meta.strLogLevel);
	CHECK(meta.bHasClientCapabilities);
	CHECK(meta.clientCapabilities.SupportsCapability(MSG_KEY_ELICITATION));
	CHECK(meta.clientCapabilities.SupportsCapability(MSG_KEY_SAMPLING));
	CHECK(meta.clientCapabilities.SupportsCapability(MSG_KEY_ROOTS));
	CHECK(meta.clientCapabilities.SupportsElicitationMode(MSG_KEY_FORM));
	CHECK(meta.clientCapabilities.SupportsElicitationMode(MSG_KEY_URL));
	CHECK(meta.clientCapabilities.SupportsExtension(EXTENSION_UI));
	CHECK(!meta.clientCapabilities.SupportsExtension("io.example/missing"));

	// Extension settings expose the declared MIME types without raw JSON parsing.
	const Json::Value* pUiSettings = meta.clientCapabilities.FindExtensionSettings(EXTENSION_UI);
	CHECK(pUiSettings != nullptr);
	if (pUiSettings)
		CHECK(pUiSettings->isMember(MSG_KEY_MIME_TYPES));

	CHECK(meta.clientCapabilities.SupportsMimeType(EXTENSION_UI, MIME_TYPE_MCP_APP));
	CHECK(!meta.clientCapabilities.SupportsMimeType(EXTENSION_UI, "text/html"));
	CHECK(!meta.clientCapabilities.SupportsMimeType("example/limited", MIME_TYPE_MCP_APP));
	CHECK(meta.clientCapabilities.FindExtensionSettings("io.example/missing") == nullptr);
	CHECK(!meta.clientCapabilities.SupportsMimeType("io.example/missing", MIME_TYPE_MCP_APP));

	// progressToken and toolCallId stay on the Request model.
	CHECK(!meta.jExtra.isMember(MSG_KEY_PROGRESS_TOKEN));
	CHECK(!meta.jExtra.isMember(MSG_KEY_TOOL_CALL_ID));
	CHECK_EQ(std::string("trace-1"), meta.jExtra["otel.trace"].asString());

	Json::Value jMetaOut(Json::objectValue);
	CHECK_EQ(ERRNO_OK, meta.DoSerialize(jMetaOut));
	CHECK(!jMetaOut[META_KEY_CLIENT_INFO].isMember(MSG_KEY_TITLE));
	VersionExtrasAccess::Serialize(meta, jMetaOut, ctxModern);
	CHECK_EQ(std::string(PROTOCOL_VER_2026_07_28), jMetaOut[META_KEY_PROTOCOL_VERSION].asString());
	CHECK_EQ(std::string("test-client"), jMetaOut[META_KEY_CLIENT_INFO][MSG_KEY_NAME].asString());
	CHECK_EQ(std::string("Test client"), jMetaOut[META_KEY_CLIENT_INFO][MSG_KEY_TITLE].asString());
	CHECK(jMetaOut[META_KEY_CLIENT_CAPABILITIES].isMember(MSG_KEY_ELICITATION));
	CHECK_EQ(std::string(LOGGING_LEVEL_INFO), jMetaOut[META_KEY_LOG_LEVEL].asString());
	CHECK_EQ(std::string("trace-1"), jMetaOut["otel.trace"].asString());
	CHECK(!jMetaOut.isMember(MSG_KEY_PROGRESS_TOKEN));

	// An empty metadata object must not create any key.
	MetaObject emptyMeta;
	CHECK(!emptyMeta.HasValue());
	Json::Value jEmptyMetaOut(Json::objectValue);
	CHECK_EQ(ERRNO_OK, emptyMeta.DoSerialize(jEmptyMetaOut));
	CHECK(jEmptyMetaOut.empty());

	// Missing required fields stay missing instead of being defaulted.
	MetaObject missingMeta;
	CHECK_EQ(ERRNO_OK, missingMeta.DoDeserialize(Json::Value(Json::objectValue)));
	CHECK(missingMeta.strProtocolVersion.empty());
	CHECK(!missingMeta.bHasClientCapabilities);
	CHECK(!missingMeta.HasValue());

	// Legacy initialize capabilities are readable through the same container.
	Json::Value jLegacyCapabilities(Json::objectValue);
	jLegacyCapabilities[MSG_KEY_EXPERIMENTAL] = Json::Value(Json::objectValue);
	Json::Value jLegacyExtensions(Json::objectValue);
	jLegacyExtensions[EXTENSION_UI] = Json::Value(Json::objectValue);
	jLegacyCapabilities[MSG_KEY_EXTENSIONS] = jLegacyExtensions;

	ClientCapabilities legacyCapabilities;
	CHECK_EQ(ERRNO_OK, legacyCapabilities.DoDeserialize(jLegacyCapabilities));
	CHECK(legacyCapabilities.HasValue());
	CHECK(legacyCapabilities.SupportsCapability(MSG_KEY_EXPERIMENTAL));
	CHECK(!legacyCapabilities.SupportsCapability(MSG_KEY_ELICITATION));
	CHECK(legacyCapabilities.SupportsExtension(EXTENSION_UI));

	Json::Value jRoundTripCapabilities(Json::objectValue);
	CHECK_EQ(ERRNO_OK, legacyCapabilities.DoSerialize(jRoundTripCapabilities));
	CHECK(jRoundTripCapabilities.isMember(MSG_KEY_EXPERIMENTAL));
	CHECK(jRoundTripCapabilities[MSG_KEY_EXTENSIONS].isMember(EXTENSION_UI));

	CHECK_EQ(ERRNO_PARSE_ERROR, legacyCapabilities.DoDeserialize(Json::Value("not-an-object")));
	CHECK_EQ(ERRNO_PARSE_ERROR, meta.DoDeserialize(Json::Value(Json::arrayValue)));
}
