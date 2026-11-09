#include "TestServer.h"

#include <Public/PublicDef.h>

#include <chrono>
#include <memory>
#include <string>
#include <thread>

namespace MCP
{
	namespace Test
	{
		namespace
		{
			bool s_bRegisterModernFixtures = false;

			Json::Value MakeSingleInputRequest(const std::string& strKey, const std::string& strMethod,
				const Json::Value& jParams)
			{
				Json::Value jInput(Json::objectValue);
				jInput[MCP::MSG_KEY_METHOD] = strMethod;
				jInput[MCP::MSG_KEY_PARAMS] = jParams;

				Json::Value jInputRequests(Json::objectValue);
				jInputRequests[strKey] = jInput;

				return jInputRequests;
			}

			Json::Value MakeElicitationInputRequests()
			{
				Json::Value jParams(Json::objectValue);
				jParams[MCP::MSG_KEY_MODE] = MCP::MSG_KEY_FORM;
				jParams[MCP::MSG_KEY_MESSAGE] = "Confirm the echo?";

				Json::Value jConfirm(Json::objectValue);
				jConfirm[MCP::MSG_KEY_METHOD] = MCP::METHOD_ELICITATION_CREATE;
				jConfirm[MCP::MSG_KEY_PARAMS] = jParams;

				Json::Value jInputRequests(Json::objectValue);
				jInputRequests["confirm"] = jConfirm;

				return jInputRequests;
			}

			class CTestEchoTask : public MCP::ProcessCallToolRequest
			{
			public:
				CTestEchoTask(const std::shared_ptr<MCP::Request>& spRequest)
					: ProcessCallToolRequest(spRequest)
				{
				}

				std::shared_ptr<MCP::CMCPTask> Clone() const override
				{
					auto spClone = std::make_shared<CTestEchoTask>(nullptr);
					if (spClone)
						*spClone = *this;

					return spClone;
				}

				int Execute() override
				{
					if (!IsValid())
						return MCP::ERRNO_INTERNAL_ERROR;

					auto spCallToolRequest = std::dynamic_pointer_cast<MCP::CallToolRequest>(m_spRequest);
					if (!spCallToolRequest)
						return MCP::ERRNO_INTERNAL_ERROR;

					std::string strAction;
					if (spCallToolRequest->jArguments.isMember("action") && spCallToolRequest->jArguments["action"].isString())
						strAction = spCallToolRequest->jArguments["action"].asString();

					// MRTR probe: an "action":"confirm" call asks the client for an
					// elicitation round trip before echoing. The legacy echo path is
					// untouched because action-less calls keep the old behaviour.
					if ("confirm" == strAction || "confirm_input_only" == strAction)
					{
						if (!m_spRequest->jInputResponses.isObject() || m_spRequest->jInputResponses.empty())
						{
							int iErrCode = NotifyInputRequired(MakeElicitationInputRequests(),
								("confirm" == strAction) ? "echo-confirm-state" : std::string());
							if (MCP::ERRNO_OK == iErrCode)
								return MCP::ERRNO_OK;

							Json::Value jForm(Json::objectValue);
							Json::Value jElicitation(Json::objectValue);
							jElicitation[MCP::MSG_KEY_FORM] = jForm;

							Json::Value jRequiredCapabilities(Json::objectValue);
							jRequiredCapabilities[MCP::MSG_KEY_ELICITATION] = jElicitation;

							return NotifyMissingClientCapability(jRequiredCapabilities);
						}

						auto spConfirmedResult = BuildResult();
						if (!spConfirmedResult)
							return MCP::ERRNO_INTERNAL_ERROR;

						MCP::TextContent confirmedContent;
						confirmedContent.strType = MCP::CONST_TEXT;
						confirmedContent.strText = ("confirm" == strAction) ? "confirmed" : "input-only-confirmed";
						spConfirmedResult->vecTextContent.push_back(confirmedContent);
						spConfirmedResult->bIsError = false;

						return NotifyResult(spConfirmedResult);
					}

					if ("confirm_empty" == strAction)
					{
						// Exercises the SDK guard that refuses an MRTR result with
						// neither inputRequests nor requestState.
						return NotifyInputRequired(Json::Value(), std::string());
					}

					// Load shedding without inputRequests: the client only needs to
					// retry with the opaque requestState.
					if ("confirm_state_only" == strAction || "confirm_tamper" == strAction)
					{
						const bool bTamper = ("confirm_tamper" == strAction);
						const std::string strExpectedState = bTamper ? "echo-tamper-state" : "echo-shed-state";

						if (m_spRequest->strRequestState.empty())
							return NotifyInputRequired(Json::Value(), strExpectedState);

						if (m_spRequest->strRequestState != strExpectedState)
						{
							auto spRejectedResult = BuildResult();
							if (!spRejectedResult)
								return MCP::ERRNO_INTERNAL_ERROR;

							MCP::TextContent rejectedContent;
							rejectedContent.strType = MCP::CONST_TEXT;
							rejectedContent.strText = "tampered state rejected";
							spRejectedResult->vecTextContent.push_back(rejectedContent);
							spRejectedResult->bIsError = true;

							return NotifyResult(spRejectedResult);
						}

						auto spStateResult = BuildResult();
						if (!spStateResult)
							return MCP::ERRNO_INTERNAL_ERROR;

						MCP::TextContent stateContent;
						stateContent.strType = MCP::CONST_TEXT;
						stateContent.strText = bTamper ? "state-verified" : "state-only-ok";
						spStateResult->vecTextContent.push_back(stateContent);
						spStateResult->bIsError = false;

						return NotifyResult(spStateResult);
					}

					// Rich content surface used by integration.modern_core.
					if ("rich" == strAction)
					{
						auto spRichResult = BuildResult();
						if (!spRichResult)
							return MCP::ERRNO_INTERNAL_ERROR;

						MCP::TextContent richText;
						richText.strType = MCP::CONST_TEXT;
						richText.strText = "rich";
						spRichResult->vecTextContent.push_back(richText);

						MCP::ImageContent richImage;
						richImage.strType = MCP::CONST_IMAGE;
						richImage.strMimeType = "image/png";
						richImage.strData = "aW1n";
						spRichResult->vecImageContent.push_back(richImage);

						MCP::AudioContent richAudio;
						richAudio.strType = MCP::CONST_AUDIO;
						richAudio.strMimeType = "audio/wav";
						richAudio.strData = "YXVkaW8=";
						spRichResult->vecAudioContent.push_back(richAudio);

						MCP::ResourceLink richLink;
						richLink.strType = MCP::CONST_RESOURCE_LINK;
						richLink.strName = "echo.txt";
						richLink.strUri = CTestServer::RESOURCE_URI;
						spRichResult->vecResourceLinkContent.push_back(richLink);

						MCP::ToolUseContent richToolUse;
						richToolUse.strType = MCP::CONST_TOOL_USE;
						richToolUse.strId = "call-1";
						richToolUse.strName = CTestServer::TOOL_NAME;
						richToolUse.jInput["input"] = "nested";
						spRichResult->vecToolUseContent.push_back(richToolUse);

						MCP::ToolResultContent richToolResult;
						richToolResult.strType = MCP::CONST_TOOL_RESULT;
						richToolResult.strToolUseId = "call-1";
						Json::Value jNestedText(Json::objectValue);
						jNestedText[MCP::MSG_KEY_TYPE] = MCP::CONST_TEXT;
						jNestedText[MCP::MSG_KEY_TEXT] = "nested";
						richToolResult.jContent.append(jNestedText);
						richToolResult.bHasIsError = true;
						richToolResult.bIsError = false;
						spRichResult->vecToolResultContent.push_back(richToolResult);

						spRichResult->bIsError = false;

						return NotifyResult(spRichResult);
					}

					// Result level _meta reserved key handling.
					if ("meta" == strAction)
					{
						auto spMetaResult = BuildResult();
						if (!spMetaResult)
							return MCP::ERRNO_INTERNAL_ERROR;

						MCP::TextContent metaContent;
						metaContent.strType = MCP::CONST_TEXT;
						metaContent.strText = "meta";
						spMetaResult->vecTextContent.push_back(metaContent);
						spMetaResult->jResultMeta["example.com/custom"] = "keep";
						spMetaResult->jResultMeta[MCP::META_KEY_SERVER_INFO][MCP::MSG_KEY_NAME] = "evil";
						spMetaResult->jResultMeta[MCP::META_KEY_SUBSCRIPTION_ID] = "evil";

						return NotifyResult(spMetaResult);
					}

					// Client capabilities and custom _meta passthrough.
					if ("caps" == strAction)
					{
						bool bHasUiExtension = m_spRequest->meta.clientCapabilities.SupportsExtension(MCP::EXTENSION_MCP_APPS_UI);

						std::string strMimeType;
						const Json::Value& jRawCapabilities = m_spRequest->meta.clientCapabilities.jRaw;
						if (jRawCapabilities.isMember(MCP::MSG_KEY_EXTENSIONS)
							&& jRawCapabilities[MCP::MSG_KEY_EXTENSIONS].isObject()
							&& jRawCapabilities[MCP::MSG_KEY_EXTENSIONS].isMember(MCP::EXTENSION_MCP_APPS_UI)
							&& jRawCapabilities[MCP::MSG_KEY_EXTENSIONS][MCP::EXTENSION_MCP_APPS_UI].isObject())
						{
							const Json::Value& jUi = jRawCapabilities[MCP::MSG_KEY_EXTENSIONS][MCP::EXTENSION_MCP_APPS_UI];
							if (jUi.isMember("mimeTypes") && jUi["mimeTypes"].isArray() && jUi["mimeTypes"].size() > 0)
								strMimeType = jUi["mimeTypes"][0].asString();
						}

						bool bHasCustomKey = m_spRequest->meta.jExtra.isObject()
							&& m_spRequest->meta.jExtra.isMember("example.com/custom");

						auto spCapsResult = BuildResult();
						if (!spCapsResult)
							return MCP::ERRNO_INTERNAL_ERROR;

						MCP::TextContent capsContent;
						capsContent.strType = MCP::CONST_TEXT;
						capsContent.strText = std::string("caps:") + (bHasUiExtension ? "1" : "0")
							+ ";" + strMimeType + ";" + (bHasCustomKey ? "1" : "0");
						spCapsResult->vecTextContent.push_back(capsContent);

						return NotifyResult(spCapsResult);
					}

					// Logging probe: the request level logLevel decides whether the
					// notification is written (see ProcessRequest::NotifyLogMessage).
					if ("log" == strAction)
					{
						Json::Value jLogData(Json::objectValue);
						jLogData[MCP::MSG_KEY_MESSAGE] = "echo log";
						NotifyLogMessage(MCP::LoggingLevelValue::LoggingLevel_Info, "echo", jLogData);

						auto spLogResult = BuildResult();
						if (!spLogResult)
							return MCP::ERRNO_INTERNAL_ERROR;

						MCP::TextContent logContent;
						logContent.strType = MCP::CONST_TEXT;
						logContent.strText = "logged";
						spLogResult->vecTextContent.push_back(logContent);

						return NotifyResult(spLogResult);
					}

					// Subscription probes used by the E2E client: the client cannot call
					// the session directly, so a tool call drives the server-side push.
					if ("notify_resource" == strAction || "notify_list_changed" == strAction
						|| "notify_prompt_list_changed" == strAction)
					{
						if ("notify_resource" == strAction)
						{
							Json::Value jUpdated(Json::objectValue);
							jUpdated[MCP::MSG_KEY_URI] = CTestServer::RESOURCE_URI;
							CTestServer::GetInstance().SendSubscriptionNotification(
								MCP::METHOD_NOTIFICATION_RESOURCES_UPDATED, jUpdated, CTestServer::RESOURCE_URI);
						}
						else if ("notify_list_changed" == strAction)
						{
							CTestServer::GetInstance().SendResourceListChanged();
						}
						else
						{
							CTestServer::GetInstance().SendPromptListChanged();
						}

						auto spNotifyResult = BuildResult();
						if (!spNotifyResult)
							return MCP::ERRNO_INTERNAL_ERROR;

						MCP::TextContent notifyContent;
						notifyContent.strType = MCP::CONST_TEXT;
						notifyContent.strText = "notified";
						spNotifyResult->vecTextContent.push_back(notifyContent);

						return NotifyResult(spNotifyResult);
					}

					// Progress probe: the notification is only written when the
					// request carried a progressToken in params._meta.
					if ("progress" == strAction)
					{
						int iErrCode = NotifyProgress(5, 10);
						if (MCP::ERRNO_OK != iErrCode)
							return iErrCode;

						auto spProgressResult = BuildResult();
						if (!spProgressResult)
							return MCP::ERRNO_INTERNAL_ERROR;

						MCP::TextContent progressContent;
						progressContent.strType = MCP::CONST_TEXT;
						progressContent.strText = "progress-done";
						spProgressResult->vecTextContent.push_back(progressContent);

						return NotifyResult(spProgressResult);
					}

					// MRTR probes for the server-to-client sampling and roots
					// requests. ValidateInputRequired gates them on the client
					// capabilities declared in params._meta.
					if ("input_sampling" == strAction || "input_roots" == strAction)
					{
						const bool bSampling = ("input_sampling" == strAction);

						if (!m_spRequest->jInputResponses.isObject() || m_spRequest->jInputResponses.empty())
						{
							Json::Value jParams(Json::objectValue);
							if (bSampling)
							{
								Json::Value jMessage(Json::objectValue);
								jMessage[MCP::MSG_KEY_ROLE] = MCP::CONST_USER;
								jMessage[MCP::MSG_KEY_CONTENT][MCP::MSG_KEY_TYPE] = MCP::CONST_TEXT;
								jMessage[MCP::MSG_KEY_CONTENT][MCP::MSG_KEY_TEXT] = "sample me";
								jParams[MCP::MSG_KEY_MESSAGES].append(jMessage);
							}

							int iErrCode = NotifyInputRequired(
								MakeSingleInputRequest(bSampling ? "sample" : "roots",
									bSampling ? MCP::METHOD_SAMPLING_CREATE_MESSAGE : MCP::METHOD_ROOTS_LIST,
									jParams),
								bSampling ? "sampling-state" : "roots-state");
							if (MCP::ERRNO_OK == iErrCode)
								return MCP::ERRNO_OK;

							Json::Value jRequiredCapabilities(Json::objectValue);
							jRequiredCapabilities[bSampling ? MCP::MSG_KEY_SAMPLING : MCP::MSG_KEY_ROOTS]
								= Json::Value(Json::objectValue);

							return NotifyMissingClientCapability(jRequiredCapabilities);
						}

						auto spMrtrResult = BuildResult();
						if (!spMrtrResult)
							return MCP::ERRNO_INTERNAL_ERROR;

						MCP::TextContent mrtrContent;
						mrtrContent.strType = MCP::CONST_TEXT;
						mrtrContent.strText = bSampling ? "sampled" : "roots-ok";
						spMrtrResult->vecTextContent.push_back(mrtrContent);
						spMrtrResult->bIsError = false;

						return NotifyResult(spMrtrResult);
					}

					// Gives the E2E client a window to queue a second request and
					// cancel it before the async worker reaches it.
					if ("slow" == strAction)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(800));
					}

					auto spResult = BuildResult();
					if (!spResult)
						return MCP::ERRNO_INTERNAL_ERROR;

					std::string strInput;
					if (spCallToolRequest->jArguments.isMember("input") && spCallToolRequest->jArguments["input"].isString())
						strInput = spCallToolRequest->jArguments["input"].asString();

					MCP::TextContent textContent;
					textContent.strType = MCP::CONST_TEXT;
					textContent.strText = strInput;
					spResult->vecTextContent.push_back(textContent);
					spResult->bIsError = false;

					return NotifyResult(spResult);
				}

				int Cancel() override
				{
					return MCP::ERRNO_OK;
				}
			};

			class CTestReadResourceTask : public MCP::ProcessReadResourceRequest
			{
			public:
				CTestReadResourceTask(const std::shared_ptr<MCP::Request>& spRequest)
					: ProcessReadResourceRequest(spRequest)
				{
				}

				std::shared_ptr<MCP::CMCPTask> Clone() const override
				{
					auto spClone = std::make_shared<CTestReadResourceTask>(nullptr);
					if (spClone)
						*spClone = *this;

					return spClone;
				}

				int Execute() override
				{
					if (!IsValid())
						return MCP::ERRNO_INTERNAL_ERROR;

					auto spReadResourceRequest = std::dynamic_pointer_cast<MCP::ReadResourceRequest>(m_spRequest);
					if (!spReadResourceRequest)
						return MCP::ERRNO_INTERNAL_ERROR;

					auto spResult = BuildResult();
					if (!spResult)
						return MCP::ERRNO_INTERNAL_ERROR;

					MCP::TextResourceContents textResource;
					textResource.strUri = spReadResourceRequest->strUri;
					textResource.strMimeType = "text/plain";
					textResource.strText = "legacy resource payload";
					spResult->vecTextResourceContents.push_back(textResource);

					return NotifyResult(spResult);
				}

				int Cancel() override
				{
					return MCP::ERRNO_OK;
				}
			};

			// Serves the MCP Apps UI resource declared by the modern fixture set.
			class CTestReadUiResourceTask : public MCP::ProcessReadResourceRequest
			{
			public:
				CTestReadUiResourceTask(const std::shared_ptr<MCP::Request>& spRequest)
					: ProcessReadResourceRequest(spRequest)
				{
				}

				std::shared_ptr<MCP::CMCPTask> Clone() const override
				{
					auto spClone = std::make_shared<CTestReadUiResourceTask>(nullptr);
					if (spClone)
						*spClone = *this;

					return spClone;
				}

				int Execute() override
				{
					if (!IsValid())
						return MCP::ERRNO_INTERNAL_ERROR;

					auto spReadResourceRequest = std::dynamic_pointer_cast<MCP::ReadResourceRequest>(m_spRequest);
					if (!spReadResourceRequest)
						return MCP::ERRNO_INTERNAL_ERROR;

					auto spResult = BuildResult();
					if (!spResult)
						return MCP::ERRNO_INTERNAL_ERROR;

					MCP::TextResourceContents textResource;
					textResource.strUri = spReadResourceRequest->strUri;
					textResource.strMimeType = MCP::MIME_TYPE_MCP_APP;
					textResource.strText = "<html><body>Echo App</body></html>";
					textResource.jMeta[MCP::MSG_KEY_UI][MCP::MSG_KEY_CSP]["connectDomains"].append("https://example.com");
					textResource.jMeta[MCP::MSG_KEY_UI][MCP::MSG_KEY_PERMISSIONS]["clipboardWrite"] = true;
					spResult->vecTextResourceContents.push_back(textResource);

					return NotifyResult(spResult);
				}

				int Cancel() override
				{
					return MCP::ERRNO_OK;
				}
			};

			// MRTR probe for a cacheable resources/read result.
			class CTestReadMrtrResourceTask : public MCP::ProcessReadResourceRequest
			{
			public:
				CTestReadMrtrResourceTask(const std::shared_ptr<MCP::Request>& spRequest)
					: ProcessReadResourceRequest(spRequest)
				{
				}

				std::shared_ptr<MCP::CMCPTask> Clone() const override
				{
					auto spClone = std::make_shared<CTestReadMrtrResourceTask>(nullptr);
					if (spClone)
						*spClone = *this;

					return spClone;
				}

				int Execute() override
				{
					if (!IsValid())
						return MCP::ERRNO_INTERNAL_ERROR;

					auto spReadResourceRequest = std::dynamic_pointer_cast<MCP::ReadResourceRequest>(m_spRequest);
					if (!spReadResourceRequest)
						return MCP::ERRNO_INTERNAL_ERROR;

					bool bHasInputResponses = m_spRequest->jInputResponses.isObject() && !m_spRequest->jInputResponses.empty();
					if (!bHasInputResponses && m_spRequest->strRequestState.empty())
						return NotifyInputRequired(Json::Value(), "echo-read-state");

					auto spResult = BuildResult();
					if (!spResult)
						return MCP::ERRNO_INTERNAL_ERROR;

					MCP::TextResourceContents textResource;
					textResource.strUri = spReadResourceRequest->strUri;
					textResource.strMimeType = "text/plain";
					textResource.strText = "mrtr-read-complete";
					spResult->vecTextResourceContents.push_back(textResource);

					return NotifyResult(spResult);
				}

				int Cancel() override
				{
					return MCP::ERRNO_OK;
				}
			};

			// Simulates an application that owns the URI space but has no payload for
			// the requested resource, so it keeps the legacy -32002 error code.
			class CTestMissingResourceTask : public MCP::ProcessReadResourceRequest
			{
			public:
				CTestMissingResourceTask(const std::shared_ptr<MCP::Request>& spRequest)
					: ProcessReadResourceRequest(spRequest)
				{
				}

				std::shared_ptr<MCP::CMCPTask> Clone() const override
				{
					auto spClone = std::make_shared<CTestMissingResourceTask>(nullptr);
					if (spClone)
						*spClone = *this;

					return spClone;
				}

				int Execute() override
				{
					if (!IsValid())
						return MCP::ERRNO_INTERNAL_ERROR;

					auto spReadResourceRequest = std::dynamic_pointer_cast<MCP::ReadResourceRequest>(m_spRequest);
					if (!spReadResourceRequest)
						return MCP::ERRNO_INTERNAL_ERROR;

					Json::Value jErrorData(Json::objectValue);
					jErrorData[MCP::MSG_KEY_URI] = spReadResourceRequest->strUri;

					return NotifyError(MCP::ERRNO_RESOURCE_NOT_FOUND, "Failed to read resource", jErrorData);
				}

				int Cancel() override
				{
					return MCP::ERRNO_OK;
				}
			};

			class CTestSubscribeResourceTask : public MCP::ProcessSubscribeResourceRequest
			{
			public:
				CTestSubscribeResourceTask(const std::shared_ptr<MCP::Request>& spRequest)
					: ProcessSubscribeResourceRequest(spRequest)
				{
				}

				std::shared_ptr<MCP::CMCPTask> Clone() const override
				{
					auto spClone = std::make_shared<CTestSubscribeResourceTask>(nullptr);
					if (spClone)
						*spClone = *this;

					return spClone;
				}

				int Execute() override
				{
					if (!IsValid())
						return MCP::ERRNO_INTERNAL_ERROR;

					return NotifyResult();
				}

				int Cancel() override
				{
					return NotifyCancelled();
				}
			};

			class CTestGetPromptTask : public MCP::ProcessGetPromptRequest
			{
			public:
				CTestGetPromptTask(const std::shared_ptr<MCP::Request>& spRequest)
					: ProcessGetPromptRequest(spRequest)
				{
				}

				std::shared_ptr<MCP::CMCPTask> Clone() const override
				{
					auto spClone = std::make_shared<CTestGetPromptTask>(nullptr);
					if (spClone)
						*spClone = *this;

					return spClone;
				}

				int Execute() override
				{
					if (!IsValid())
						return MCP::ERRNO_INTERNAL_ERROR;

					auto spResult = BuildResult();
					if (!spResult)
						return MCP::ERRNO_INTERNAL_ERROR;

					spResult->strDescription = "Echo test prompt";

					MCP::PromptMessage message;
					message.strRole = MCP::CONST_USER;
					message.strContentType = MCP::CONST_TEXT;
					message.textContent.strType = MCP::CONST_TEXT;
					message.textContent.strText = "hello from prompt";
					spResult->vecMessages.push_back(message);

					return NotifyResult(spResult);
				}

				int Cancel() override
				{
					return MCP::ERRNO_OK;
				}
			};

			class CTestCompleteTask : public MCP::ProcessCompleteRequest
			{
			public:
				CTestCompleteTask(const std::shared_ptr<MCP::Request>& spRequest)
					: ProcessCompleteRequest(spRequest)
				{
				}

				std::shared_ptr<MCP::CMCPTask> Clone() const override
				{
					auto spClone = std::make_shared<CTestCompleteTask>(nullptr);
					if (spClone)
						*spClone = *this;

					return spClone;
				}

				int Execute() override
				{
					if (!IsValid())
						return MCP::ERRNO_INTERNAL_ERROR;

					auto spCompleteRequest = std::dynamic_pointer_cast<MCP::CompleteRequest>(m_spRequest);
					if (!spCompleteRequest)
						return MCP::ERRNO_INTERNAL_ERROR;

					auto spResult = BuildResult();
					if (!spResult)
						return MCP::ERRNO_INTERNAL_ERROR;

					spResult->vecValues.push_back("alpha");
					spResult->vecValues.push_back("beta");
					spResult->iTotal = static_cast<int>(spResult->vecValues.size());
					spResult->bHasMore = false;

					// The resource reference branch reports a truncated page so the
					// client side hasMore handling is observable. Legacy callers only
					// use ref/prompt, so the golden payload stays unchanged.
					if (MCP::CONST_REF_RESOURCE == spCompleteRequest->strRefType)
					{
						spResult->vecValues.clear();
						spResult->vecValues.push_back("gamma");
						spResult->vecValues.push_back("delta");
						spResult->iTotal = 5;
						spResult->bHasMore = true;
					}

					return NotifyResult(spResult);
				}

				int Cancel() override
				{
					return MCP::ERRNO_OK;
				}
			};
		}

		int CTestServer::Initialize()
		{
			// 1. Basic server information.
			MCP::Implementation serverInfo;
			serverInfo.strName = CTestServer::SERVER_NAME;
			serverInfo.strVersion = CTestServer::SERVER_VERSION;
			SetServerInfo(serverInfo);

			// 2. Legacy capability declaration.
			MCP::Tools tools;
			RegisterServerToolsCapabilities(tools);

			MCP::Resources resources;
			resources.bListChanged = true;
			resources.bSubscribe = true;
			RegisterServerResourcesCapabilities(resources);

			MCP::Prompts prompts;
			prompts.bListChanged = true;
			RegisterServerPromptsCapabilities(prompts);

			MCP::Experimental experimental;
			experimental.bCompletion = true;
			RegisterServerExperimentalCapabilities(experimental);

			RegisterServerLoggingCapabilities();

			// 3. Legacy tool/resource/prompt declarations.
			MCP::Tool tool;
			tool.strName = CTestServer::TOOL_NAME;
			tool.strDescription = "Receive the input and return it unchanged.";
			Json::Reader reader;
			Json::Value jInputSchema(Json::objectValue);
			if (!reader.parse("{\"type\":\"object\",\"properties\":{\"input\":{\"type\":\"string\"}},\"required\":[\"input\"]}", jInputSchema) || !jInputSchema.isObject())
				return MCP::ERRNO_PARSE_ERROR;
			tool.jInputSchema = jInputSchema;

			std::vector<MCP::Tool> vecTools;
			vecTools.push_back(tool);

			if (s_bRegisterModernFixtures)
			{
				MCP::Tool appTool;
				appTool.strName = CTestServer::APP_TOOL_NAME;
				appTool.strDescription = "MCP App entry point.";
				appTool.jInputSchema = jInputSchema;
				appTool.SetUiResourceUri(CTestServer::UI_RESOURCE_URI);
				vecTools.push_back(appTool);
			}

			RegisterServerTools(vecTools, false);

			MCP::Resource resource;
			resource.strName = "echo.txt";
			resource.strUri = CTestServer::RESOURCE_URI;
			resource.strDescription = "Legacy test resource";
			resource.strMimeType = "text/plain";

			std::vector<MCP::Resource> vecResources;
			vecResources.push_back(resource);

			if (s_bRegisterModernFixtures)
			{
				MCP::Resource uiResource;
				uiResource.strName = "Echo App";
				uiResource.strUri = CTestServer::UI_RESOURCE_URI;
				uiResource.strDescription = "MCP App UI resource";
				uiResource.strMimeType = MCP::MIME_TYPE_MCP_APP;
				uiResource.jMeta[MCP::MSG_KEY_UI][MCP::MSG_KEY_CSP]["connectDomains"].append("https://example.com");
				uiResource.jMeta[MCP::MSG_KEY_UI][MCP::MSG_KEY_PERMISSIONS]["clipboardWrite"] = true;
				vecResources.push_back(uiResource);
			}

			RegisterServerResources(vecResources, false);

			MCP::ResourceTemplate resourceTemplate;
			resourceTemplate.strName = "Echo files";
			resourceTemplate.strUriTemplate = "test://echo/{name}";
			resourceTemplate.strDescription = "Legacy test resource template";
			resourceTemplate.strMimeType = "text/plain";
			RegisterServerResourceTemplates({ resourceTemplate }, false);

			MCP::Prompt prompt;
			prompt.strName = CTestServer::PROMPT_NAME;
			prompt.strDescription = "Echo test prompt";
			MCP::PromptArgument promptArgument;
			promptArgument.strName = "text";
			promptArgument.strDescription = "Text to echo";
			promptArgument.bRequired = true;
			prompt.vecArguments.push_back(promptArgument);
			RegisterServerPrompts({ prompt }, false);

			// 4. Legacy tasks.
			auto spEchoTask = std::make_shared<CTestEchoTask>(nullptr);
			if (!spEchoTask)
				return MCP::ERRNO_INTERNAL_ERROR;
			RegisterToolsTasks(CTestServer::TOOL_NAME, spEchoTask);

			if (s_bRegisterModernFixtures)
			{
				RegisterToolsTasks(CTestServer::APP_TOOL_NAME, spEchoTask);

				auto spReadUiResourceTask = std::make_shared<CTestReadUiResourceTask>(nullptr);
				if (!spReadUiResourceTask)
					return MCP::ERRNO_INTERNAL_ERROR;
				RegisterReadResourceTasks(CTestServer::UI_RESOURCE_URI, spReadUiResourceTask);

				auto spReadMrtrResourceTask = std::make_shared<CTestReadMrtrResourceTask>(nullptr);
				if (!spReadMrtrResourceTask)
					return MCP::ERRNO_INTERNAL_ERROR;
				RegisterReadResourceTasks(CTestServer::MRTR_RESOURCE_URI, spReadMrtrResourceTask);
			}

			auto spReadResourceTask = std::make_shared<CTestReadResourceTask>(nullptr);
			if (!spReadResourceTask)
				return MCP::ERRNO_INTERNAL_ERROR;
			RegisterReadResourceTasks(CTestServer::RESOURCE_URI, spReadResourceTask);

			auto spMissingResourceTask = std::make_shared<CTestMissingResourceTask>(nullptr);
			if (!spMissingResourceTask)
				return MCP::ERRNO_INTERNAL_ERROR;
			RegisterReadResourceTasks(CTestServer::MISSING_RESOURCE_URI, spMissingResourceTask);

			auto spSubscribeResourceTask = std::make_shared<CTestSubscribeResourceTask>(nullptr);
			if (!spSubscribeResourceTask)
				return MCP::ERRNO_INTERNAL_ERROR;
			RegisterSubscribeResourceTasks(CTestServer::RESOURCE_URI, spSubscribeResourceTask);

			auto spGetPromptTask = std::make_shared<CTestGetPromptTask>(nullptr);
			if (!spGetPromptTask)
				return MCP::ERRNO_INTERNAL_ERROR;
			RegisterGetPromptTasks(CTestServer::PROMPT_NAME, spGetPromptTask);

			auto spCompleteTask = std::make_shared<CTestCompleteTask>(nullptr);
			if (!spCompleteTask)
				return MCP::ERRNO_INTERNAL_ERROR;
			RegisterCompleteTasks(std::string(MCP::CONST_REF_PROMPT) + ":" + CTestServer::PROMPT_NAME, spCompleteTask);
			RegisterCompleteTasks(std::string(MCP::CONST_REF_RESOURCE) + ":" + CTestServer::RESOURCE_URI, spCompleteTask);

			return MCP::ERRNO_OK;
		}

		void CTestServer::SetRegisterModernFixtures(bool bRegister)
		{
			s_bRegisterModernFixtures = bRegister;
		}

		int CTestServer::Uninitialize()
		{
			return MCP::ERRNO_OK;
		}

		CTestServer CTestServer::s_Instance;
	}
}
