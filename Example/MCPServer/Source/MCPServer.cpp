#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <string>
#include <cstdlib>
#include <signal.h>
#include "EchoServer.h"
#include "Transport/SSETransport.h"


static void PrintUsage(const char* pszProgName)
{
    std::cerr << "Usage: " << pszProgName
              << " [--transport stdio|sse] [--host <ip>] [--port <port>]" << std::endl;
    std::cerr << "  default: --transport stdio" << std::endl;
    std::cerr << "  sse default endpoint: http://127.0.0.1:8080  (GET /sse, POST /messages)" << std::endl;
}


int LaunchEchoServer(const std::string& strTransport, const std::string& strHost, int iPort)
{
    // 1. Configure the Server.
    auto& server = Implementation::CEchoServer::GetInstance();
    int iErrCode = server.Initialize();
    if (MCP::ERRNO_OK == iErrCode)
    {
        if (strTransport == "sse")
        {
            // 2026-07-28 needs a stateless transport, so the SSE entry serves the
            // 2024-11-05 handshake model only.
            server.SetTransport(std::make_shared<MCP::CSSETransport>(strHost, iPort));
            server.SetSupportedProtocolVersions({ MCP::PROTOCOL_VER_2024_11_05 });
        }
        else
        {
            // stdio carries stateless revisions, so both eras are offered. The
            // declaration is explicit because modern support is opt-in.
            server.SetSupportedProtocolVersions({ MCP::PROTOCOL_VER_2024_11_05,
                MCP::PROTOCOL_VER_2026_07_28 });
        }
        // 2. Start the Server.
        server.Start();
        // 3. Stop the Server.
        server.Stop();
        // 4. Deinitialization
        server.Uninitialize();
    }

    return iErrCode;
}


std::atomic_bool g_bStop{ false };
void signal_handler(int signal) { g_bStop = true; }

int main(int argc, char* argv[])
{
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    std::string strTransport = "stdio";
    std::string strHost = "127.0.0.1";
    int iPort = 8080;

    for (int i = 1; i < argc; ++i)
    {
        std::string strArg = argv[i];
        if (strArg == "--transport" && i + 1 < argc)
        {
            strTransport = argv[++i];
        }
        else if (strArg == "--host" && i + 1 < argc)
        {
            strHost = argv[++i];
        }
        else if (strArg == "--port" && i + 1 < argc)
        {
            iPort = std::atoi(argv[++i]);
        }
        else if (strArg == "--help" || strArg == "-h")
        {
            PrintUsage(argv[0]);
            return 0;
        }
        else
        {
            std::cerr << "Unknown argument: " << strArg << std::endl;
            PrintUsage(argv[0]);
            return 1;
        }
    }

    if (strTransport != "stdio" && strTransport != "sse")
    {
        std::cerr << "Invalid --transport value: " << strTransport << std::endl;
        PrintUsage(argv[0]);
        return 1;
    }

    return LaunchEchoServer(strTransport, strHost, iPort);
}
