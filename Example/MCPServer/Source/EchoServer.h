#pragma once

#include <Entity/Server.h>


namespace Implementation
{
	// A server class for business operations is declared. 
	// It is used to customize unique logic, but it must be a singleton.
	class CEchoServer : public MCP::CMCPServer<CEchoServer>
	{
	public:
		static constexpr const char* SERVER_NAME = "echo_server";
		static constexpr const char* SERVER_VERSION = "1.0.0.1";

		// This is the initialization method, which is used to configure the Server. 
		// The Server can be started only after the configuration is successful.
		int Initialize() override;

		// This is the deinitialization method, where you can perform any necessary cleanup tasks.
		int Uninitialize() override;

	private:
		friend class MCP::CMCPServer<CEchoServer>;
		CEchoServer() = default;
		static CEchoServer s_Instance;
	};
}
