#include "EchoTask.h"
#include <Public/PublicDef.h>
#include <Public/StringHelper.h>
#include <fstream>
#include <chrono>
#include <memory>


namespace Implementation
{
	namespace
	{
		// Illustrative integrity check only. A production server must protect
		// requestState with an AEAD or an HMAC (for example AES-GCM); a checksum
		// such as the one below is not tamper resistant. This example deliberately
		// avoids pulling in a third-party crypto library.
		constexpr const char* REQUEST_STATE_SECRET = "echo-confirm-demo-secret";

		std::string ToHexString(unsigned long long uValue)
		{
			static const char* pszDigits = "0123456789abcdef";
			std::string strResult;
			for (int i = 15; i >= 0; --i)
				strResult.push_back(pszDigits[(uValue >> (i * 4)) & 0xF]);
			return strResult;
		}

		unsigned long long Fnv1a64(const std::string& strInput)
		{
			unsigned long long uHash = 1469598103934665603ULL;
			for (unsigned char ch : strInput)
			{
				uHash ^= static_cast<unsigned long long>(ch);
				uHash *= 1099511628211ULL;
			}
			return uHash;
		}

		std::string BuildRequestState(const std::string& strInput)
		{
			std::string strPayload = std::string("v1:") + strInput;
			return strPayload + ":" + ToHexString(Fnv1a64(strPayload + ":" + REQUEST_STATE_SECRET));
		}

		bool VerifyRequestState(const std::string& strRequestState)
		{
			size_t iSeparator = strRequestState.rfind(':');
			if (std::string::npos == iSeparator || 0 == iSeparator)
				return false;

			std::string strPayload = strRequestState.substr(0, iSeparator);
			std::string strChecksum = strRequestState.substr(iSeparator + 1);
			return strChecksum == ToHexString(Fnv1a64(strPayload + ":" + REQUEST_STATE_SECRET));
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// CEchoTask
	std::shared_ptr<MCP::CMCPTask> CEchoTask::Clone() const
	{
		auto spClone = std::make_shared<CEchoTask>(nullptr);
		if (spClone)
		{
			*spClone = *this;
		}

		return spClone;
	}

	int CEchoTask::Cancel()
	{
		return MCP::ERRNO_OK;
	}

	int CEchoTask::Execute()
	{
		NotifyLogMessage(MCP::LoggingLevelValue::LoggingLevel_Info, "CEchoTask", Json::Value("Execution started"));

		int iErrCode = MCP::ERRNO_INTERNAL_ERROR;
		if (!IsValid())
			return iErrCode;

		Json::Value jArgument;
		std::string strInput;
		auto spCallToolRequest = std::dynamic_pointer_cast<MCP::CallToolRequest>(m_spRequest);
		if (!spCallToolRequest)
			goto PROC_END;
		if (spCallToolRequest->strName.compare(TOOL_NAME) != 0)
			goto PROC_END;
		jArgument = spCallToolRequest->jArguments;
		if (!jArgument.isMember(TOOL_ARGUMENT_INPUT) || !jArgument[TOOL_ARGUMENT_INPUT].isString())
			goto PROC_END;
		strInput = jArgument[TOOL_ARGUMENT_INPUT].asString();
		iErrCode = MCP::ERRNO_OK;

	PROC_END:
		auto spExecuteResult = BuildResult();
		if (spExecuteResult)
		{
			MCP::TextContent textContent;
			textContent.strType = MCP::CONST_TEXT;
			if (MCP::ERRNO_OK == iErrCode)
			{
				spExecuteResult->bIsError = false;
				textContent.strText = strInput;
			}
			else
			{
				spExecuteResult->bIsError = true;
				textContent.strText = u8"Unfortunately, the execution failed.";
				textContent.strText += u8"Error code:";
				textContent.strText += std::to_string(iErrCode);
			}
			spExecuteResult->vecTextContent.push_back(textContent);
			iErrCode = NotifyResult(spExecuteResult);
		}

		return iErrCode;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// CEchoConfirmTask
	std::shared_ptr<MCP::CMCPTask> CEchoConfirmTask::Clone() const
	{
		auto spClone = std::make_shared<CEchoConfirmTask>(nullptr);
		if (spClone)
		{
			*spClone = *this;
		}

		return spClone;
	}

	int CEchoConfirmTask::Cancel()
	{
		return MCP::ERRNO_OK;
	}

	int CEchoConfirmTask::Execute()
	{
		if (!IsValid())
			return MCP::ERRNO_INTERNAL_ERROR;

		auto spCallToolRequest = std::dynamic_pointer_cast<MCP::CallToolRequest>(m_spRequest);
		if (!spCallToolRequest || spCallToolRequest->strName != TOOL_NAME)
			return MCP::ERRNO_INTERNAL_ERROR;

		std::string strInput;
		if (spCallToolRequest->jArguments.isMember(TOOL_ARGUMENT_INPUT)
			&& spCallToolRequest->jArguments[TOOL_ARGUMENT_INPUT].isString())
		{
			strInput = spCallToolRequest->jArguments[TOOL_ARGUMENT_INPUT].asString();
		}

		// First round: no inputResponses yet, so ask the client for an
		// elicitation round trip and hand back an opaque requestState.
		if (!m_spRequest->inputResponses.HasValue())
		{
			Json::Value jProperties(Json::objectValue);
			Json::Value jConfirmProperty(Json::objectValue);
			jConfirmProperty["type"] = "boolean";
			jConfirmProperty[MCP::MSG_KEY_TITLE] = "Confirm";
			jProperties["confirm"] = jConfirmProperty;

			Json::Value jRequiredSchema(Json::arrayValue);
			jRequiredSchema.append("confirm");

			Json::Value jRequestedSchema(Json::objectValue);
			jRequestedSchema["type"] = "object";
			jRequestedSchema["properties"] = jProperties;
			jRequestedSchema[MCP::MSG_KEY_REQUIRED] = jRequiredSchema;

			Json::Value jParams(Json::objectValue);
			jParams[MCP::MSG_KEY_MODE] = MCP::MSG_KEY_FORM;
			jParams[MCP::MSG_KEY_MESSAGE] = "Confirm the echo request.";
			jParams[MCP::MSG_KEY_REQUESTED_SCHEMA] = jRequestedSchema;

			Json::Value jConfirmRequest(Json::objectValue);
			jConfirmRequest[MCP::MSG_KEY_METHOD] = MCP::METHOD_ELICITATION_CREATE;
			jConfirmRequest[MCP::MSG_KEY_PARAMS] = jParams;

			Json::Value jInputRequests(Json::objectValue);
			jInputRequests["confirm"] = jConfirmRequest;

			MCP::InputRequests inputRequests;
			inputRequests.Set(jInputRequests);

			int iErrCode = NotifyInputRequired(inputRequests, BuildRequestState(strInput));
			if (MCP::ERRNO_OK == iErrCode)
				return iErrCode;

			// The client did not declare form elicitation; tell it what is needed.
			Json::Value jElicitation(Json::objectValue);
			jElicitation[MCP::MSG_KEY_FORM] = Json::Value(Json::objectValue);

			Json::Value jRequiredCapabilities(Json::objectValue);
			jRequiredCapabilities[MCP::MSG_KEY_ELICITATION] = jElicitation;

			return NotifyMissingClientCapability(jRequiredCapabilities);
		}

		auto spResult = BuildResult();
		if (!spResult)
			return MCP::ERRNO_INTERNAL_ERROR;

		MCP::TextContent textContent;
		textContent.strType = MCP::CONST_TEXT;

		// Retry round: reject a requestState that was tampered with or produced
		// by another server instance.
		if (!VerifyRequestState(m_spRequest->strRequestState))
		{
			spResult->bIsError = true;
			textContent.strText = u8"Invalid requestState; restart the confirmation round.";
		}
		else
		{
			spResult->bIsError = false;
			textContent.strText = u8"confirmed: " + strInput;
		}

		spResult->vecTextContent.push_back(textContent);
		return NotifyResult(spResult);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// CEchoAppTask
	std::shared_ptr<MCP::CMCPTask> CEchoAppTask::Clone() const
	{
		auto spClone = std::make_shared<CEchoAppTask>(nullptr);
		if (spClone)
		{
			*spClone = *this;
		}

		return spClone;
	}

	int CEchoAppTask::Cancel()
	{
		return MCP::ERRNO_OK;
	}

	int CEchoAppTask::Execute()
	{
		if (!IsValid())
			return MCP::ERRNO_INTERNAL_ERROR;

		auto spCallToolRequest = std::dynamic_pointer_cast<MCP::CallToolRequest>(m_spRequest);
		if (!spCallToolRequest || spCallToolRequest->strName != TOOL_NAME)
			return MCP::ERRNO_INTERNAL_ERROR;

		std::string strInput;
		if (spCallToolRequest->jArguments.isMember(TOOL_ARGUMENT_INPUT)
			&& spCallToolRequest->jArguments[TOOL_ARGUMENT_INPUT].isString())
		{
			strInput = spCallToolRequest->jArguments[TOOL_ARGUMENT_INPUT].asString();
		}

		auto spResult = BuildResult();
		if (!spResult)
			return MCP::ERRNO_INTERNAL_ERROR;

		// Text content keeps working for clients that do not render the UI.
		MCP::TextContent textContent;
		textContent.strType = MCP::CONST_TEXT;
		textContent.strText = u8"echo_app received: " + strInput;
		spResult->vecTextContent.push_back(textContent);

		// The Apps view consumes structuredContent; the schema above describes it.
		Json::Value jStructured(Json::objectValue);
		jStructured["echo"] = strInput;
		jStructured["length"] = static_cast<Json::UInt>(strInput.size());
		spResult->structuredContent.Set(jStructured);
		spResult->bIsError = false;

		return NotifyResult(spResult);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// CEchoAppResourceReadTask
	std::shared_ptr<MCP::CMCPTask> CEchoAppResourceReadTask::Clone() const
	{
		auto spClone = std::make_shared<CEchoAppResourceReadTask>(nullptr);
		if (spClone)
		{
			*spClone = *this;
		}

		return spClone;
	}

	int CEchoAppResourceReadTask::Cancel()
	{
		return MCP::ERRNO_OK;
	}

	int CEchoAppResourceReadTask::Execute()
	{
		if (!IsValid())
			return MCP::ERRNO_INTERNAL_ERROR;

		auto spReadResourceRequest = std::dynamic_pointer_cast<MCP::ReadResourceRequest>(m_spRequest);
		if (!spReadResourceRequest)
			return MCP::ERRNO_INTERNAL_ERROR;

		auto spResult = BuildResult();
		if (!spResult)
			return MCP::ERRNO_INTERNAL_ERROR;

		MCP::TextResourceContents textContent;
		textContent.strUri = spReadResourceRequest->strUri;
		textContent.strMimeType = MCP::MIME_TYPE_MCP_APP;
		// The HTML below speaks the MCP Apps View<->Host bridge. It is demo glue,
		// not SDK code: the host owns window.parent and answers ui/initialize,
		// tools/call and the tool-input/tool-result notifications.
		textContent.strText =
			u8R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<title>Echo App</title>
<style>
  :root { color-scheme: light dark; }
  body { font-family: system-ui, sans-serif; margin: 16px; }
  input, button { font: inherit; padding: 6px 10px; }
  pre { padding: 8px; background: rgba(127, 127, 127, 0.15); white-space: pre-wrap; }
</style>
</head>
<body>
<h1>Echo App</h1>
<p id="status">Connecting to the MCP Apps host...</p>
<div>
  <input id="input" value="hello from the app" size="32">
  <button id="call">Call echo tool</button>
</div>
<pre id="output">No tool result yet.</pre>
<script>
(function () {
  'use strict';

  var pending = {};
  var nextId = 1;
  var statusNode = document.getElementById('status');
  var inputNode = document.getElementById('input');
  var outputNode = document.getElementById('output');

  function post(message) {
    window.parent.postMessage(message, '*');
  }

  function request(method, params) {
    return new Promise(function (resolve, reject) {
      var id = nextId++;
      pending[id] = { resolve: resolve, reject: reject };
      post({ jsonrpc: '2.0', id: id, method: method, params: params || {} });
    });
  }

  function notify(method, params) {
    post({ jsonrpc: '2.0', method: method, params: params || {} });
  }

  function show(value) {
    outputNode.textContent = JSON.stringify(value, null, 2);
  }

  window.addEventListener('message', function (event) {
    var message = event.data;
    if (!message || message.jsonrpc !== '2.0') {
      return;
    }

    if (message.id !== undefined && pending[message.id]) {
      var entry = pending[message.id];
      delete pending[message.id];
      if (message.error) {
        entry.reject(message.error);
      } else {
        entry.resolve(message.result);
      }
      return;
    }

    if (message.method === 'ui/notifications/tool-input') {
      var args = message.params && message.params.arguments;
      if (args && typeof args.input === 'string') {
        inputNode.value = args.input;
      }
      statusNode.textContent = 'Tool input received.';
      return;
    }

    if (message.method === 'ui/notifications/tool-result') {
      var result = message.params || {};
      show(result.structuredContent !== undefined ? result.structuredContent : result);
      statusNode.textContent = result.isError ? 'Tool reported an error.' : 'Tool result received.';
    }
  });

  document.getElementById('call').addEventListener('click', function () {
    statusNode.textContent = 'Calling the echo tool...';
    request('tools/call', { name: 'echo', arguments: { input: inputNode.value } })
      .then(function (result) {
        show(result.structuredContent !== undefined ? result.structuredContent : result);
        statusNode.textContent = 'Echo tool returned.';
      })
      .catch(function (error) {
        show(error);
        statusNode.textContent = 'Echo tool call failed.';
      });
  });

  if (window.parent === window) {
    statusNode.textContent = 'Standalone preview: no MCP Apps host detected.';
    return;
  }

  var settled = false;
  request('ui/initialize', {
    protocolVersion: '2026-07-28',
    appInfo: { name: 'echo-app', version: '1.0.0' },
    appCapabilities: {}
  }).then(function (result) {
    settled = true;
    var hostName = result && result.hostInfo && result.hostInfo.name ? result.hostInfo.name : 'host';
    statusNode.textContent = 'Connected to ' + hostName + '.';
    notify('ui/notifications/initialized', {});
  }).catch(function () {
    settled = true;
    statusNode.textContent = 'The host did not accept ui/initialize.';
  });

  window.setTimeout(function () {
    if (!settled) {
      statusNode.textContent = 'No host response; running as a preview.';
    }
  }, 2000);

  function sendSize() {
    notify('ui/notifications/size-changed', {
      width: Math.ceil(document.documentElement.scrollWidth),
      height: Math.ceil(document.documentElement.scrollHeight)
    });
  }

  if (window.ResizeObserver) {
    new ResizeObserver(sendSize).observe(document.body);
  }
  window.addEventListener('load', sendSize);
})();
</script>
</body>
</html>
)";
		spResult->vecTextResourceContents.push_back(textContent);

		return NotifyResult(spResult);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// CEchoResourceReadTask
	std::shared_ptr<MCP::CMCPTask> CEchoResourceReadTask::Clone() const
	{
		auto spClone = std::make_shared<CEchoResourceReadTask>(nullptr);
		if (spClone)
		{
			*spClone = *this;
		}

		return spClone;
	}

	int CEchoResourceReadTask::Cancel()
	{
		return MCP::ERRNO_OK;
	}

	int CEchoResourceReadTask::Execute()
	{
		int iErrCode = MCP::ERRNO_INTERNAL_ERROR;
		if (!IsValid())
			return iErrCode;

		auto spReadResourceRequest = std::dynamic_pointer_cast<MCP::ReadResourceRequest>(m_spRequest);
		if (!spReadResourceRequest)
			goto PROC_END;
		
		iErrCode = MCP::ERRNO_OK;

	PROC_END:
		if (MCP::ERRNO_OK == iErrCode)
		{
			auto spExecuteResult = BuildResult();
			if (spExecuteResult)
			{
				MCP::TextResourceContents textContent;
				textContent.strMimeType = u8"text/x-rust";
				textContent.strUri = spReadResourceRequest->strUri;
				textContent.strText = u8"fn main() {\n    println!(\"Hello, world!\");\n}";
				spExecuteResult->vecTextResourceContents.push_back(textContent);
				iErrCode = NotifyResult(spExecuteResult);
			}
		}
		else
		{
			Json::Value jErrData(Json::objectValue);
			jErrData[MCP::MSG_KEY_URI] = spReadResourceRequest->strUri;
			iErrCode = NotifyError(iErrCode, u8"Failed to read resource", jErrData);
		}		

		return iErrCode;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// CEchoResourceSubscribeTask
	std::shared_ptr<MCP::CMCPTask> CEchoResourceSubscribeTask::Clone() const
	{
		auto spClone = std::make_shared<CEchoResourceSubscribeTask>(nullptr);
		if (spClone)
		{
			// The clone only carries the request; the session binds its services
			// after cloning and a fresh task starts with idle state flags.
			spClone->SetRequest(GetRequest());
		}

		return spClone;
	}

	int CEchoResourceSubscribeTask::Cancel()
	{
		m_bNeedCancel = true;
		return MCP::ERRNO_OK;
	}

	int CEchoResourceSubscribeTask::Execute()
	{
		int iErrCode = MCP::ERRNO_INTERNAL_ERROR;
		if (!IsValid())
			return iErrCode;

		auto spSubscribeResourceRequest = std::dynamic_pointer_cast<MCP::SubscribeResourceRequest>(m_spRequest);
		if (!spSubscribeResourceRequest)
			goto PROC_END;

		iErrCode = MCP::ERRNO_OK;

	PROC_END:
		if (MCP::ERRNO_OK == iErrCode)
		{
			NotifyResult();

			m_upSubscribeThread = std::make_unique<std::thread>([this]() {
					do
					{
						// Simulate resource update notification
						std::this_thread::sleep_for(std::chrono::seconds(2));
						NotifyUpdated();
					} while (!m_bNeedCancel);

					NotifyCancelled();
				});
			if (m_upSubscribeThread)
			{
				m_upSubscribeThread->detach();
			}
		}
		else
		{
			Json::Value jErrData(Json::objectValue);
			jErrData[MCP::MSG_KEY_URI] = spSubscribeResourceRequest->strUri;
			iErrCode = NotifyError(iErrCode, u8"Failed to subscribe resource", jErrData);
		}

		return iErrCode;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// CEchoGetPromptTask
	std::shared_ptr<MCP::CMCPTask> CEchoGetPromptTask::Clone() const
	{
		auto spClone = std::make_shared<CEchoGetPromptTask>(nullptr);
		if (spClone)
		{
			*spClone = *this;
		}

		return spClone;
	}

	int CEchoGetPromptTask::Cancel()
	{
		return MCP::ERRNO_OK;
	}

	int CEchoGetPromptTask::Execute()
	{
		int iErrCode = MCP::ERRNO_INTERNAL_ERROR;
		if (!IsValid())
			return iErrCode;

		auto spGetPromptRequest = std::dynamic_pointer_cast<MCP::GetPromptRequest>(m_spRequest);
		if (!spGetPromptRequest)
			goto PROC_END;

		iErrCode = MCP::ERRNO_OK;

	PROC_END:
		if (MCP::ERRNO_OK == iErrCode)
		{
			auto spResult = BuildResult();
			if (!spResult)
				return MCP::ERRNO_INTERNAL_ERROR;

			if (spGetPromptRequest->strName == PROMPT_CODE_REVIEW)
			{
				spResult->strDescription = "Code review prompt template";

				MCP::PromptMessage msg;
				msg.strRole = MCP::CONST_USER;
				msg.strContentType = MCP::CONST_TEXT;

				std::string strCode;
				if (spGetPromptRequest->jArguments.isMember("code") && 
					spGetPromptRequest->jArguments["code"].isString())
				{
					strCode = spGetPromptRequest->jArguments["code"].asString();
				}

				msg.textContent.strType = MCP::CONST_TEXT;
				msg.textContent.strText = "Please review the following code and provide suggestions:\n\n";
				msg.textContent.strText += strCode;

				spResult->vecMessages.push_back(msg);
			}
			else if (spGetPromptRequest->strName == PROMPT_EXPLAIN_CODE)
			{
				spResult->strDescription = "Code explanation prompt template";

				MCP::PromptMessage msg;
				msg.strRole = MCP::CONST_USER;
				msg.strContentType = MCP::CONST_TEXT;

				std::string strCode;
				if (spGetPromptRequest->jArguments.isMember("code") && 
					spGetPromptRequest->jArguments["code"].isString())
				{
					strCode = spGetPromptRequest->jArguments["code"].asString();
				}

				msg.textContent.strType = MCP::CONST_TEXT;
				msg.textContent.strText = "Please explain the following code in detail:\n\n";
				msg.textContent.strText += strCode;

				spResult->vecMessages.push_back(msg);
			}
			else
			{
				return NotifyError(MCP::ERRNO_INVALID_PARAMS, "Unknown prompt name", Json::Value());
			}

			iErrCode = NotifyResult(spResult);
		}
		else
		{
			Json::Value jErrData(Json::objectValue);
			jErrData[MCP::MSG_KEY_NAME] = spGetPromptRequest->strName;
			iErrCode = NotifyError(iErrCode, "Failed to get prompt", jErrData);
		}

		return iErrCode;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// CEchoCompleteTask
	std::shared_ptr<MCP::CMCPTask> CEchoCompleteTask::Clone() const
	{
		auto spClone = std::make_shared<CEchoCompleteTask>(nullptr);
		if (spClone)
		{
			*spClone = *this;
		}

		return spClone;
	}

	int CEchoCompleteTask::Cancel()
	{
		return MCP::ERRNO_OK;
	}

	int CEchoCompleteTask::Execute()
	{
		int iErrCode = MCP::ERRNO_INTERNAL_ERROR;
		if (!IsValid())
			return iErrCode;

		auto spCompleteRequest = std::dynamic_pointer_cast<MCP::CompleteRequest>(m_spRequest);
		if (!spCompleteRequest)
			goto PROC_END;

		iErrCode = MCP::ERRNO_OK;

	PROC_END:
		if (MCP::ERRNO_OK == iErrCode)
		{
			auto spResult = BuildResult();
			if (!spResult)
				return MCP::ERRNO_INTERNAL_ERROR;

			if (spCompleteRequest->strRefType == MCP::CONST_REF_PROMPT)
			{
				if (spCompleteRequest->promptRef.strName == CEchoGetPromptTask::PROMPT_CODE_REVIEW)
				{
					if (spCompleteRequest->strArgumentName == "language")
					{
						spResult->vecValues = {
							"python",
							"javascript",
							"typescript",
							"rust",
							"go",
							"c++",
							"java",
							"c#"
						};
						spResult->iTotal = static_cast<int>(spResult->vecValues.size());
						spResult->bHasMore = false;
					}
				}
				else if (spCompleteRequest->promptRef.strName == CEchoGetPromptTask::PROMPT_EXPLAIN_CODE)
				{
					if (spCompleteRequest->strArgumentName == "code")
					{
						spResult->vecValues = {
							"fn main() { ... }",
							"class Example { ... }"
						};
						spResult->iTotal = static_cast<int>(spResult->vecValues.size());
						spResult->bHasMore = false;
					}
				}
			}
			else if (spCompleteRequest->strRefType == MCP::CONST_REF_RESOURCE)
			{
				spResult->vecValues = {
					"file:///project/src/main.rs",
					"file:///project/src/lib.rs",
					"file:///project/src/utils.rs"
				};
				spResult->iTotal = static_cast<int>(spResult->vecValues.size());
				spResult->bHasMore = false;
			}

			iErrCode = NotifyResult(spResult);
		}
		else
		{
			Json::Value jErrData(Json::objectValue);
			jErrData[MCP::MSG_KEY_REF] = spCompleteRequest->strRefType;
			iErrCode = NotifyError(iErrCode, "Failed to complete", jErrData);
		}

		return iErrCode;
	}
}