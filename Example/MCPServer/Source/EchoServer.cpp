#include "EchoServer.h"
#include "EchoTask.h"
#include <chrono>
#include <cstdio>
#include <fstream>

namespace Implementation
{
    int CEchoServer::Initialize()
    {
        // 1. Set the basic information of the Server.
        MCP::Implementation serverInfo;
        serverInfo.strName = Implementation::CEchoServer::SERVER_NAME;
        serverInfo.strVersion = Implementation::CEchoServer::SERVER_VERSION;
        SetServerInfo(serverInfo);

        // 2. Register the Server's capability declaration.
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

		// 2026-07-28 capabilities. The legacy experimental flag above stays
		// untouched so that 2024-11-05 clients receive the same payload as before.
		RegisterServerCompletionsCapability();
		// MCP Apps is declared by RegisterServerApp below, which also registers the
		// app tool and its UI resource.

        // 3. Register the descriptions of the Server's actual capabilities and their calling methods.
		auto ParseInputSchema = [](const char* pszSchema, Json::Value& jSchemaValue) -> bool
		{
			Json::Reader reader;
			Json::Value jSchema;
			if (!reader.parse(pszSchema, jSchema) || !jSchema.isObject())
				return false;

			jSchemaValue = jSchema;
			return true;
		};

		std::vector<MCP::Tool> vecTools;

        MCP::Tool tool;
        tool.strName = Implementation::CEchoTask::TOOL_NAME;
        tool.strDescription = Implementation::CEchoTask::TOOL_DESCRIPTION;
        if (!ParseInputSchema(Implementation::CEchoTask::TOOL_INPUT_SCHEMA, tool.jInputSchema))
            return MCP::ERRNO_PARSE_ERROR;
		vecTools.push_back(tool);

		// MRTR example: the first call returns input_required.
		MCP::Tool confirmTool;
		confirmTool.strName = Implementation::CEchoConfirmTask::TOOL_NAME;
		confirmTool.strDescription = Implementation::CEchoConfirmTask::TOOL_DESCRIPTION;
		if (!ParseInputSchema(Implementation::CEchoConfirmTask::TOOL_INPUT_SCHEMA, confirmTool.jInputSchema))
			return MCP::ERRNO_PARSE_ERROR;
		vecTools.push_back(confirmTool);

        RegisterServerTools(vecTools, false);

		MCP::Resource resource;
		resource.strName = "main.rs";
		resource.strUri = "file:///project/src/main.rs";
		resource.strDescription = "Primary application entry point";
		resource.strMimeType = "text/x-rust";

        RegisterServerResources({ resource }, false);

		MCP::ResourceTemplate resourceTemplate;
		resourceTemplate.strName = "Project Files";
		resourceTemplate.strUriTemplate = "file:///{path}";
		resourceTemplate.strDescription = "Access files in the project directory";
		resourceTemplate.strMimeType = "application/octet-stream";
        RegisterServerResourceTemplates({ resourceTemplate }, false);

        MCP::Prompt codeReviewPrompt;
        codeReviewPrompt.strName = Implementation::CEchoGetPromptTask::PROMPT_CODE_REVIEW;
        codeReviewPrompt.strDescription = "Asks the LLM to analyze code quality and suggest improvements";        
        MCP::PromptArgument codeArg;
        codeArg.strName = "code";
        codeArg.strDescription = "The code to review";
        codeArg.bRequired = true;
        codeReviewPrompt.vecArguments.push_back(codeArg);
        MCP::Prompt explainCodePrompt;
        explainCodePrompt.strName = Implementation::CEchoGetPromptTask::PROMPT_EXPLAIN_CODE;
        explainCodePrompt.strDescription = "Asks the LLM to explain code in detail";        
        MCP::PromptArgument explainCodeArg;
        explainCodeArg.strName = "code";
        explainCodeArg.strDescription = "The code to explain";
        explainCodeArg.bRequired = true;
        explainCodePrompt.vecArguments.push_back(explainCodeArg);
        RegisterServerPrompts({ codeReviewPrompt, explainCodePrompt }, false);

        // 4. MCP Apps: one call advertises the extension, registers the tool and the
        // UI resource, binds the tool task and a built-in resources/read task that
        // serves the HTML below. Visibility defaults to {model, app}.
        MCP::AppDefinition appDef;
        appDef.tool.strName = Implementation::CEchoAppTask::TOOL_NAME;
        appDef.tool.strDescription = Implementation::CEchoAppTask::TOOL_DESCRIPTION;
        if (!ParseInputSchema(Implementation::CEchoAppTask::TOOL_INPUT_SCHEMA, appDef.tool.jInputSchema))
            return MCP::ERRNO_PARSE_ERROR;
        if (!ParseInputSchema(Implementation::CEchoAppTask::TOOL_OUTPUT_SCHEMA, appDef.tool.jOutputSchema))
            return MCP::ERRNO_PARSE_ERROR;
        appDef.resource.strName = "echo app";
        appDef.resource.strUri = Implementation::ECHO_APP_RESOURCE_URI;
        appDef.resource.strDescription = "MCP Apps UI for the echo server";
        appDef.resource.strMimeType = MCP::MIME_TYPE_MCP_APP;
        appDef.uiResourceMeta.bClipboardWrite = true;
        appDef.strHtml = Implementation::GetEchoAppHtml();
        appDef.spToolTask = std::make_shared<Implementation::CEchoAppTask>(nullptr);
        if (!appDef.spToolTask)
            return MCP::ERRNO_INTERNAL_ERROR;

        int iErrCode = RegisterServerApp(appDef);
        if (MCP::ERRNO_OK != iErrCode)
            return iErrCode;

        // The same view is also served through RegisterServerAppFromFile. The HTML
        // is written to a local file for the registration and removed right after,
        // so the sample does not depend on the working directory.
        MCP::AppDefinition fileAppDef = appDef;
        fileAppDef.tool.strName = Implementation::ECHO_FILE_APP_TOOL_NAME;
        fileAppDef.tool.strDescription = Implementation::ECHO_FILE_APP_TOOL_DESCRIPTION;
        fileAppDef.resource.strName = "echo file app";
        fileAppDef.resource.strUri = Implementation::ECHO_FILE_APP_RESOURCE_URI;
        fileAppDef.resource.strDescription = "MCP Apps UI loaded from a local HTML file";
        fileAppDef.spToolTask = std::make_shared<Implementation::CEchoAppTask>(
            nullptr, Implementation::ECHO_FILE_APP_TOOL_NAME);
        if (!fileAppDef.spToolTask)
            return MCP::ERRNO_INTERNAL_ERROR;

        const std::string strHtmlPath = "echo_file_app.html";
        {
            std::ofstream htmlFile(strHtmlPath.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
            if (!htmlFile.is_open())
                return MCP::ERRNO_INTERNAL_ERROR;
            htmlFile << Implementation::GetEchoAppHtml();
        }
        iErrCode = RegisterServerAppFromFile(fileAppDef, strHtmlPath);
        std::remove(strHtmlPath.c_str());
        if (MCP::ERRNO_OK != iErrCode)
            return iErrCode;

        // 5. Register the tasks for implementing the actual capabilities.
        auto spCallToolsTask = std::make_shared<Implementation::CEchoTask>(nullptr);
        if (!spCallToolsTask)
            return MCP::ERRNO_INTERNAL_ERROR;
        RegisterToolsTasks(Implementation::CEchoTask::TOOL_NAME, spCallToolsTask);

		auto spConfirmToolTask = std::make_shared<Implementation::CEchoConfirmTask>(nullptr);
		if (!spConfirmToolTask)
			return MCP::ERRNO_INTERNAL_ERROR;
		RegisterToolsTasks(Implementation::CEchoConfirmTask::TOOL_NAME, spConfirmToolTask);
        
		auto spReadResourceTask = std::make_shared<Implementation::CEchoResourceReadTask>(nullptr);
		if (!spReadResourceTask)
            return MCP::ERRNO_INTERNAL_ERROR;
		RegisterReadResourceTasks(resource.strUri, spReadResourceTask);

		auto spSubscribeResourceTask = std::make_shared<Implementation::CEchoResourceSubscribeTask>(nullptr);
        if (!spSubscribeResourceTask)
			return MCP::ERRNO_INTERNAL_ERROR;
		RegisterSubscribeResourceTasks(resource.strUri, spSubscribeResourceTask);

        auto spGetPromptTask = std::make_shared<Implementation::CEchoGetPromptTask>(nullptr);
        if (!spGetPromptTask)
            return MCP::ERRNO_INTERNAL_ERROR;
        RegisterGetPromptTasks(Implementation::CEchoGetPromptTask::PROMPT_CODE_REVIEW, spGetPromptTask);        
        RegisterGetPromptTasks(Implementation::CEchoGetPromptTask::PROMPT_EXPLAIN_CODE, spGetPromptTask);

        auto spCompleteTask = std::make_shared<Implementation::CEchoCompleteTask>(nullptr);
        if (!spCompleteTask)
            return MCP::ERRNO_INTERNAL_ERROR;
        RegisterCompleteTasks(std::string(MCP::CONST_REF_PROMPT) + ":" + Implementation::CEchoGetPromptTask::PROMPT_CODE_REVIEW, spCompleteTask);
        RegisterCompleteTasks(std::string(MCP::CONST_REF_PROMPT) + ":" + Implementation::CEchoGetPromptTask::PROMPT_EXPLAIN_CODE, spCompleteTask);
        RegisterCompleteTasks(std::string(MCP::CONST_REF_RESOURCE) + ":" + resource.strUri, spCompleteTask);

        // 6. Start the subscriptions/listen demo.
        StartSubscriptionDemo();

        return MCP::ERRNO_OK;
    }

    int CEchoServer::Uninitialize()
    {
        StopSubscriptionDemo();
        return MCP::ERRNO_OK;
    }

    void CEchoServer::StartSubscriptionDemo()
    {
        m_bStopSubscriptionDemo = false;
        m_upSubscriptionDemoThread = std::make_unique<std::thread>([this]()
        {
            int iTicks = 0;
            while (!m_bStopSubscriptionDemo)
            {
                // Wake up once per second so that shutdown stays responsive.
                std::this_thread::sleep_for(std::chrono::seconds(1));
                if (m_bStopSubscriptionDemo)
                    break;

                if (++iTicks < 5)
                    continue;
                iTicks = 0;

                if (!CEchoServer::GetInstance().GetTransport())
                    continue;

                MCP::ResourceUpdatedNotification updated(false);
                updated.strUri = Implementation::ECHO_APP_RESOURCE_URI;

                // Only active subscriptions/listen registrations receive this
                // notification; a server without listeners sends nothing.
                CEchoServer::GetInstance().SendSubscriptionNotification(
                    updated, Implementation::ECHO_APP_RESOURCE_URI);
            }
        });
    }

    void CEchoServer::StopSubscriptionDemo()
    {
        m_bStopSubscriptionDemo = true;
        if (m_upSubscriptionDemoThread && m_upSubscriptionDemoThread->joinable())
            m_upSubscriptionDemoThread->join();
        m_upSubscriptionDemoThread.reset();
    }

    CEchoServer CEchoServer::s_Instance;
}
