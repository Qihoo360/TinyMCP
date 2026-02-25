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
        std::vector<MCP::Tool> vecTools;
        vecTools.push_back(tool);
        RegisterServerTools(vecTools, false);

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
