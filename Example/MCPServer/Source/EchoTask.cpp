#include "EchoTask.h"
#include <Public/PublicDef.h>
#include <Public/StringHelper.h>
#include <fstream>
#include <thread>
#include <chrono>


namespace Implementation
{
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
			spClone->m_bCancelled = m_bCancelled;
			spClone->m_bFinished = m_bFinished;
			spClone->m_bNeedCancel = m_bNeedCancel ? true : false;
			spClone->m_spRequest = m_spRequest;
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

			do
			{
				// Simulate resource update notification
				std::this_thread::sleep_for(std::chrono::seconds(2));
				NotifyUpdated();
			} while (!m_bNeedCancel);

			NotifyCancelled();
		}
		else
		{
			Json::Value jErrData(Json::objectValue);
			jErrData[MCP::MSG_KEY_URI] = spSubscribeResourceRequest->strUri;
			iErrCode = NotifyError(iErrCode, u8"Failed to subscribe resource", jErrData);
		}

		return iErrCode;
	}
}