#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <signal.h>
#include "EchoServer.h"


int LaunchEchoServer()
{
    // 1. Configure the Server.
    auto& server = Implementation::CEchoServer::GetInstance();
    int iErrCode = server.Initialize();
    if (MCP::ERRNO_OK == iErrCode)
    {
        // 2. Start the Server.
        iErrCode = server.Start();
        if (MCP::ERRNO_OK == iErrCode)
        {
            // 3. Stop the Server.
            server.Stop();
        }

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
    
    return LaunchEchoServer();
}
