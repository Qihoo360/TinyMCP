#include "EchoTask.h"
#include <Public/PublicDef.h>
#include <Public/StringHelper.h>
#include <fstream>


namespace Implementation
{
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
}