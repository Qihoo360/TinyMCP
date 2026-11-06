#pragma once
// Internal to the SDK. A dispatched task receives the session-side services it
// needs through this interface instead of reaching for the process-wide
// CMCPSession singleton, which keeps the task layer free of hidden globals.

#include "../Public/PublicDef.h"
#include "../Message/BasicMessage.h"
#include "../Message/Request.h"
#include "../Message/Response.h"
#include "../Transport/Transport.h"

#include <memory>
#include <string>
#include <vector>

namespace MCP
{
	struct ProtocolProfile;
	struct TaskServices;

	struct TaskServices
	{
		virtual ~TaskServices() {}

		virtual std::shared_ptr<CMCPTransport> GetTransport() const = 0;

		virtual void PrepareResult(const std::shared_ptr<MCP::Request>& spRequest, const std::shared_ptr<MCP::Response>& spResponse) = 0;
		virtual int SendLogMessage(const MCP::LoggingLevel& level, const std::string& strLogger, const Json::Value& jData) = 0;
		virtual int WriteLogNotification(const MCP::LoggingLevel& level, const std::string& strLogger, const Json::Value& jData) = 0;
		virtual int CancelTask(const MCP::RequestId& requestId) = 0;

		virtual MCP::Implementation GetServerInfo() const = 0;
		virtual MCP::ServerCapabilities GetServerCapabilities() const = 0;
		virtual std::string GetServerInstructions() const = 0;

		virtual bool GetServerToolsPagination() const = 0;
		virtual std::vector<MCP::Tool> GetServerTools() const = 0;
		virtual bool GetServerResourcesPagination() const = 0;
		virtual std::vector<MCP::Resource> GetServerResources() const = 0;
		virtual bool GetServerResourceTemplatesPagination() const = 0;
		virtual std::vector<MCP::ResourceTemplate> GetServerResourceTemplates() const = 0;
		virtual bool GetServerPromptsPagination() const = 0;
		virtual std::vector<MCP::Prompt> GetServerPrompts() const = 0;

		virtual void SetLoggingLevel(const MCP::LoggingLevel& level) = 0;

		virtual std::shared_ptr<const ProtocolProfile> GetDefaultHandshakeProfile() const = 0;
		virtual std::vector<std::string> GetSupportedProtocolVersions() const = 0;
	};
}
