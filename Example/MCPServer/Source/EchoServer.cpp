#include "EchoServer.h"
#include "EchoTask.h"

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

        // 3. Register the descriptions of the Server's actual capabilities and their calling methods.
        MCP::Tool tool;
        tool.strName = Implementation::CEchoTask::TOOL_NAME;
        tool.strDescription = Implementation::CEchoTask::TOOL_DESCRIPTION;
        std::string strInputSchema = Implementation::CEchoTask::TOOL_INPUT_SCHEMA;
        Json::Reader reader;
        Json::Value jInputSchema(Json::objectValue);
        if (!reader.parse(strInputSchema, jInputSchema) || !jInputSchema.isObject())
            return MCP::ERRNO_PARSE_ERROR;
        tool.jInputSchema = jInputSchema;
        RegisterServerTools({ tool }, false);

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

        // 4. Register the tasks for implementing the actual capabilities.
        auto spCallToolsTask = std::make_shared<Implementation::CEchoTask>(nullptr);
        if (!spCallToolsTask)
            return MCP::ERRNO_INTERNAL_ERROR;
        RegisterToolsTasks(Implementation::CEchoTask::TOOL_NAME, spCallToolsTask);
        
		auto spReadResourceTask = std::make_shared<Implementation::CEchoResourceReadTask>(nullptr);
		if (!spReadResourceTask)
            return MCP::ERRNO_INTERNAL_ERROR;
		RegisterReadResourceTasks(resource.strUri, spReadResourceTask);

		auto spSubscribeResourceTask = std::make_shared<Implementation::CEchoResourceSubscribeTask>(nullptr);
        if (!spSubscribeResourceTask)
			return MCP::ERRNO_INTERNAL_ERROR;
		RegisterSubscribeResourceTasks(resource.strUri, spSubscribeResourceTask);

        return MCP::ERRNO_OK;
    }

    int CEchoServer::Uninitialize()
    {
        return MCP::ERRNO_OK;
    }

    CEchoServer CEchoServer::s_Instance;
}
