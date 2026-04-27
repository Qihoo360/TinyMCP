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
		resource.strName = "echo_resource";
		resource.strUri = "echo_resource_uri";
		resource.strDescription = "This is a resource for echo server.";
		resource.strMimeType = "text/plain";
        RegisterServerResources({ resource }, false);

        // 4. Register the tasks for implementing the actual capabilities.
        auto spCallToolsTask = std::make_shared<Implementation::CEchoTask>(nullptr);
        if (!spCallToolsTask)
            return MCP::ERRNO_INTERNAL_ERROR;
        RegisterToolsTasks(Implementation::CEchoTask::TOOL_NAME, spCallToolsTask);

        return MCP::ERRNO_OK;
    }

    int CEchoServer::Uninitialize()
    {
        return MCP::ERRNO_OK;
    }

    CEchoServer CEchoServer::s_Instance;
}
