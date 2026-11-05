#include "TestServer.h"

#include <Public/PublicDef.h>

#include <memory>
#include <string>

namespace MCP
{
	namespace Test
	{
		namespace
		{
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
					if ("confirm" == strAction)
					{
						if (!m_spRequest->jInputResponses.isObject() || m_spRequest->jInputResponses.empty())
						{
							Json::Value jParams(Json::objectValue);
							jParams[MCP::MSG_KEY_MODE] = MCP::MSG_KEY_FORM;
							jParams["message"] = "Confirm the echo?";

							Json::Value jConfirm(Json::objectValue);
							jConfirm[MCP::MSG_KEY_METHOD] = MCP::METHOD_ELICITATION_CREATE;
							jConfirm[MCP::MSG_KEY_PARAMS] = jParams;

							Json::Value jInputRequests(Json::objectValue);
							jInputRequests["confirm"] = jConfirm;

							int iErrCode = NotifyInputRequired(jInputRequests, "echo-confirm-state");
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
						confirmedContent.strText = "confirmed";
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
					m_bCancelled = true;
					return MCP::ERRNO_OK;
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
			RegisterServerTools({ tool }, false);

			MCP::Resource resource;
			resource.strName = "echo.txt";
			resource.strUri = CTestServer::RESOURCE_URI;
			resource.strDescription = "Legacy test resource";
			resource.strMimeType = "text/plain";
			RegisterServerResources({ resource }, false);

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

		int CTestServer::Uninitialize()
		{
			return MCP::ERRNO_OK;
		}

		CTestServer CTestServer::s_Instance;
	}
}
