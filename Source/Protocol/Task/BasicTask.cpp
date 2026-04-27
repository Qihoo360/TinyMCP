#include "BasicTask.h"
#include "../Session/Session.h"
#include "../Message/Notification.h"

namespace MCP
{
	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessRequest
	bool ProcessRequest::IsValid() const
	{
		if (!m_spRequest)
			return false;

		return m_spRequest->IsValid();
	}

	bool ProcessRequest::IsFinished() const
	{
		return true;
	}

	bool ProcessRequest::IsCancelled() const
	{
		return false;
	}

	int ProcessRequest::Execute()
	{
		return ERRNO_OK;
	}

	int ProcessRequest::Cancel()
	{
		return ERRNO_OK;
	}

	void ProcessRequest::SetRequest(const std::shared_ptr<MCP::Request>& spRequest)
	{
		m_spRequest = spRequest;
	}

	std::shared_ptr<MCP::Request> ProcessRequest::GetRequest() const
	{
		return m_spRequest;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessPingRequest
	std::shared_ptr<CMCPTask> ProcessPingRequest::Clone() const
	{
		return nullptr;
	}

	int ProcessPingRequest::Execute()
	{
		if (!IsValid())
			return ERRNO_INTERNAL_ERROR;

		auto spEmptyResponse = std::make_shared<EmptyResponse>(true);
		if (!spEmptyResponse)
			return ERRNO_INTERNAL_ERROR;
		spEmptyResponse->requestId = m_spRequest->requestId;
		std::string strResponse;
		if (ERRNO_OK != spEmptyResponse->Serialize(strResponse))
			return ERRNO_INTERNAL_ERROR;
		auto spTransport = CMCPSession::GetInstance().GetTransport();
		if (!spTransport)
			return ERRNO_INTERNAL_ERROR;
#ifdef _WIN32
		strResponse += "\r\n";
#else
		strResponse += "\n";
#endif // _WIN32
		if (ERRNO_OK != spTransport->Write(strResponse))
			return ERRNO_INTERNAL_ERROR;

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessErrorRequest
	std::shared_ptr<CMCPTask> ProcessErrorRequest::Clone() const
	{
		return nullptr;
	}

	int ProcessErrorRequest::Execute()
	{
		if (!IsValid())
			return ERRNO_INTERNAL_ERROR;

		auto spErrorResponse = std::make_shared<ErrorResponse>(true);
		if (!spErrorResponse)
			return ERRNO_INTERNAL_ERROR;
		spErrorResponse->requestId = m_spRequest->requestId;
		if (m_strMessage.empty())
		{
			switch (m_iCode)
			{
				case ERRNO_PARSE_ERROR:
				{
					m_strMessage = ERROR_MESSAGE_PARSE_ERROR;
				} break;
				case ERRNO_INVALID_REQUEST:
				{
					m_strMessage = ERROR_MESSAGE_INVALID_REQUEST;
				} break;
				case ERRNO_METHOD_NOT_FOUND:
				{
					m_strMessage = ERROR_MESSAGE_METHOD_NOT_FOUND;
				} break;
				case ERRNO_INVALID_PARAMS:
				{
					m_strMessage = ERROR_MESSAGE_INVALID_PARAMS;
				} break;
				case ERRNO_INTERNAL_ERROR:
				{
					m_strMessage = ERROR_MESSAGE_INTERNAL_ERROR;
				} break;
				default: break;
			}
		}
		spErrorResponse->iCode = m_iCode;
		spErrorResponse->strMesage = m_strMessage;

		std::string strResponse;
		if (ERRNO_OK != spErrorResponse->Serialize(strResponse))
			return ERRNO_INTERNAL_ERROR;
		auto spTransport = CMCPSession::GetInstance().GetTransport();
		if (!spTransport)
			return ERRNO_INTERNAL_ERROR;
#ifdef _WIN32
		strResponse += "\r\n";
#else
		strResponse += "\n";
#endif // _WIN32
		if (ERRNO_OK != spTransport->Write(strResponse))
			return ERRNO_INTERNAL_ERROR;

		return ERRNO_OK;
	}

	void ProcessErrorRequest::SetErrorCode(int iCode)
	{
		m_iCode = iCode;
	}

	void ProcessErrorRequest::SetErrorMessage(const std::string& strMessage)
	{
		m_strMessage = strMessage;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessInitializeRequest
	std::shared_ptr<CMCPTask> ProcessInitializeRequest::Clone() const
	{
		return nullptr;
	}

	int ProcessInitializeRequest::Execute()
	{
		if (!IsValid())
			return ERRNO_INTERNAL_ERROR;

		auto spInitializeResult = std::make_shared<InitializeResult>(true);
		if (!spInitializeResult)
			return ERRNO_INTERNAL_ERROR;
		spInitializeResult->requestId = m_spRequest->requestId;
		spInitializeResult->strProtocolVersion = PROTOCOL_VER;
		spInitializeResult->capabilities = CMCPSession::GetInstance().GetServerCapabilities();
		spInitializeResult->implServerInfo = CMCPSession::GetInstance().GetServerInfo();
		std::string strResponse;
		if (ERRNO_OK != spInitializeResult->Serialize(strResponse))
			return ERRNO_INTERNAL_ERROR;
		auto spTransport = CMCPSession::GetInstance().GetTransport();
		if (!spTransport)
			return ERRNO_INTERNAL_ERROR;
#ifdef _WIN32
		strResponse += "\r\n";
#else
		strResponse += "\n";
#endif // _WIN32
		if (ERRNO_OK != spTransport->Write(strResponse))
			return ERRNO_INTERNAL_ERROR;

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessListToolsRequest
	std::shared_ptr<CMCPTask> ProcessListToolsRequest::Clone() const
	{
		return nullptr;
	}

	int ProcessListToolsRequest::Execute()
	{
		if (!IsValid())
			return ERRNO_INTERNAL_ERROR;

		auto spListToolRequest = std::dynamic_pointer_cast<ListToolsRequest>(m_spRequest);
		if (!spListToolRequest)
			return ERRNO_INTERNAL_ERROR;
		
		std::shared_ptr<ListToolsResult> spListToolsResult = nullptr;
		std::string strResponse;

		bool bPagination = CMCPSession::GetInstance().GetServerToolsPagination();		
		if (bPagination)
		{
			if (!spListToolRequest->strCursor.empty())
			{
				bool bValidCursor = true;
				unsigned nCursor = 0;
				try
				{
					nCursor = std::stoul(spListToolRequest->strCursor);
				}
				catch (const std::invalid_argument&)
				{
					bValidCursor = false;
				}
				catch (const std::out_of_range&)
				{
					bValidCursor = false;
				}

				auto vecServerTools = CMCPSession::GetInstance().GetServerTools();
				if (bValidCursor)
				{
					if (nCursor >= vecServerTools.size())
					{
						bValidCursor = false;
					}
				}

				if (!bValidCursor)
				{
					auto spErrorResponse = std::make_shared<ErrorResponse>(true);
					if (!spErrorResponse)
						return ERRNO_INTERNAL_ERROR;
					spErrorResponse->iCode = ERRNO_INVALID_PARAMS;
					spErrorResponse->strMesage = "invalid params";
					if (ERRNO_OK != spErrorResponse->Serialize(strResponse))
						return ERRNO_INTERNAL_ERROR;
				}
				else
				{
					spListToolsResult = std::make_shared<ListToolsResult>(true);
					if (!spListToolsResult)
						return ERRNO_INTERNAL_ERROR;
					spListToolsResult->requestId = spListToolRequest->requestId;
					spListToolsResult->vecTools.clear();
					spListToolsResult->vecTools.push_back(vecServerTools[nCursor]);
					if (nCursor < vecServerTools.size() - 1)
					{
						spListToolsResult->strNextCursor = std::to_string(nCursor + 1);
					}
				}
			}
			else
			{
				spListToolsResult = std::make_shared<ListToolsResult>(true);
				if (!spListToolsResult)
					return ERRNO_INTERNAL_ERROR;
				spListToolsResult->requestId = spListToolRequest->requestId;
				auto vecServerTools = CMCPSession::GetInstance().GetServerTools();
				spListToolsResult->vecTools.clear();
				if (vecServerTools.size() > 0)
					spListToolsResult->vecTools.push_back(vecServerTools[0]);
				if (vecServerTools.size() > 1)
					spListToolsResult->strNextCursor = std::to_string(1);
			}
		}
		else
		{
			spListToolsResult = std::make_shared<ListToolsResult>(true);
			if (!spListToolsResult)
				return ERRNO_INTERNAL_ERROR;
			spListToolsResult->requestId = spListToolRequest->requestId;
			spListToolsResult->vecTools = CMCPSession::GetInstance().GetServerTools();
		}

		if (spListToolsResult)
		{
			if (ERRNO_OK != spListToolsResult->Serialize(strResponse))
				return ERRNO_INTERNAL_ERROR;
		}

		if (!strResponse.empty())
		{
			auto spTransport = CMCPSession::GetInstance().GetTransport();
			if (!spTransport)
				return ERRNO_INTERNAL_ERROR;
#ifdef _WIN32
			strResponse += "\r\n";
#else
			strResponse += "\n";
#endif // _WIN32
			if (ERRNO_OK != spTransport->Write(strResponse))
				return ERRNO_INTERNAL_ERROR;
		}

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessCallToolRequest
	bool ProcessCallToolRequest::IsFinished() const
	{
		return m_bFinished;
	}

	bool ProcessCallToolRequest::IsCancelled() const
	{
		return m_bCancelled;
	}

	std::shared_ptr<MCP::CallToolResult> ProcessCallToolRequest::BuildResult()
	{
		if (!IsValid())
			return nullptr;

		auto spCallToolResult = std::make_shared<CallToolResult>(true);
		if (!spCallToolResult)
			return nullptr;
		spCallToolResult->requestId = m_spRequest->requestId;

		return spCallToolResult;
	}

	int ProcessCallToolRequest::NotifyProgress(int iProgress, int iTotal)
	{
		if (!m_spRequest)
			return ERRNO_INTERNAL_ERROR;

		if (m_spRequest->progressToken.IsValid())
		{
			MCP::ProgressNotification progressNotification(false);
			progressNotification.strMethod = METHOD_NOTIFICATION_PROGRESS;
			progressNotification.progressToken = m_spRequest->progressToken;
			progressNotification.iProgress = iProgress;
			progressNotification.iTotal = iTotal;

			std::string strNotification;
			if (ERRNO_OK != progressNotification.Serialize(strNotification))
				return ERRNO_INTERNAL_ERROR;
			auto spTransport = CMCPSession::GetInstance().GetTransport();
			if (!spTransport)
				return ERRNO_INTERNAL_ERROR;
#ifdef _WIN32
			strNotification += "\r\n";
#else
			strNotification += "\n";
#endif // _WIN32
			if (ERRNO_OK != spTransport->Write(strNotification))
				return ERRNO_INTERNAL_ERROR;
		}

		return ERRNO_OK;
	}

	int ProcessCallToolRequest::NotifyResult(std::shared_ptr<MCP::CallToolResult> spResult)
	{
		m_bFinished = true;

		if (!spResult)
			return ERRNO_INTERNAL_ERROR;

		std::string strResponse;
		if (ERRNO_OK != spResult->Serialize(strResponse))
			return ERRNO_INTERNAL_ERROR;
		auto spTransport = CMCPSession::GetInstance().GetTransport();
		if (!spTransport)
			return ERRNO_INTERNAL_ERROR;
#ifdef _WIN32
		strResponse += "\r\n";
#else
		strResponse += "\n";
#endif // _WIN32
		if (ERRNO_OK != spTransport->Write(strResponse))
			return ERRNO_INTERNAL_ERROR;

		return ERRNO_OK;
	}

	int ProcessCallToolRequest::NotifyCancelled()
	{
		m_bCancelled = true;
		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessListResourcesRequest
	std::shared_ptr<CMCPTask> ProcessListResourcesRequest::Clone() const
	{
		return nullptr;
	}

	int ProcessListResourcesRequest::Execute()
	{
		if (!IsValid())
			return ERRNO_INTERNAL_ERROR;

		auto spListResourcesResult = std::make_shared<ListResourcesResult>(true);
		if (!spListResourcesResult)
			return ERRNO_INTERNAL_ERROR;
		spListResourcesResult->requestId = m_spRequest->requestId;

		// TODO: 实现资源列表的获取逻辑

		std::string strResponse;
		if (ERRNO_OK != spListResourcesResult->Serialize(strResponse))
			return ERRNO_INTERNAL_ERROR;
		auto spTransport = CMCPSession::GetInstance().GetTransport();
		if (!spTransport)
			return ERRNO_INTERNAL_ERROR;
#ifdef _WIN32
		strResponse += "\r\n";
#else
		strResponse += "\n";
#endif // _WIN32
		if (ERRNO_OK != spTransport->Write(strResponse))
			return ERRNO_INTERNAL_ERROR;

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessReadResourceRequest
	std::shared_ptr<CMCPTask> ProcessReadResourceRequest::Clone() const
	{
		return nullptr;
	}

	int ProcessReadResourceRequest::Execute()
	{
		if (!IsValid())
			return ERRNO_INTERNAL_ERROR;

		auto spReadResourceRequest = std::dynamic_pointer_cast<ReadResourceRequest>(m_spRequest);
		if (!spReadResourceRequest)
			return ERRNO_INTERNAL_ERROR;

		auto spReadResourceResult = std::make_shared<ReadResourceResult>(true);
		if (!spReadResourceResult)
			return ERRNO_INTERNAL_ERROR;
		spReadResourceResult->requestId = m_spRequest->requestId;

		// TODO: 实现资源读取逻辑

		std::string strResponse;
		if (ERRNO_OK != spReadResourceResult->Serialize(strResponse))
			return ERRNO_INTERNAL_ERROR;
		auto spTransport = CMCPSession::GetInstance().GetTransport();
		if (!spTransport)
			return ERRNO_INTERNAL_ERROR;
#ifdef _WIN32
		strResponse += "\r\n";
#else
		strResponse += "\n";
#endif // _WIN32
		if (ERRNO_OK != spTransport->Write(strResponse))
			return ERRNO_INTERNAL_ERROR;

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessSubscribeRequest
	std::shared_ptr<CMCPTask> ProcessSubscribeRequest::Clone() const
	{
		return nullptr;
	}

	int ProcessSubscribeRequest::Execute()
	{
		if (!IsValid())
			return ERRNO_INTERNAL_ERROR;

		auto spEmptyResponse = std::make_shared<EmptyResponse>(true);
		if (!spEmptyResponse)
			return ERRNO_INTERNAL_ERROR;
		spEmptyResponse->requestId = m_spRequest->requestId;

		// TODO: 实现资源订阅逻辑

		std::string strResponse;
		if (ERRNO_OK != spEmptyResponse->Serialize(strResponse))
			return ERRNO_INTERNAL_ERROR;
		auto spTransport = CMCPSession::GetInstance().GetTransport();
		if (!spTransport)
			return ERRNO_INTERNAL_ERROR;
#ifdef _WIN32
		strResponse += "\r\n";
#else
		strResponse += "\n";
#endif // _WIN32
		if (ERRNO_OK != spTransport->Write(strResponse))
			return ERRNO_INTERNAL_ERROR;

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessUnsubscribeRequest
	std::shared_ptr<CMCPTask> ProcessUnsubscribeRequest::Clone() const
	{
		return nullptr;
	}

	int ProcessUnsubscribeRequest::Execute()
	{
		if (!IsValid())
			return ERRNO_INTERNAL_ERROR;

		auto spEmptyResponse = std::make_shared<EmptyResponse>(true);
		if (!spEmptyResponse)
			return ERRNO_INTERNAL_ERROR;
		spEmptyResponse->requestId = m_spRequest->requestId;

		// TODO: 实现资源取消订阅逻辑

		std::string strResponse;
		if (ERRNO_OK != spEmptyResponse->Serialize(strResponse))
			return ERRNO_INTERNAL_ERROR;
		auto spTransport = CMCPSession::GetInstance().GetTransport();
		if (!spTransport)
			return ERRNO_INTERNAL_ERROR;
#ifdef _WIN32
		strResponse += "\r\n";
#else
		strResponse += "\n";
#endif // _WIN32
		if (ERRNO_OK != spTransport->Write(strResponse))
			return ERRNO_INTERNAL_ERROR;

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessListResourceTemplatesRequest
	std::shared_ptr<CMCPTask> ProcessListResourceTemplatesRequest::Clone() const
	{
		return nullptr;
	}

	int ProcessListResourceTemplatesRequest::Execute()
	{
		if (!IsValid())
			return ERRNO_INTERNAL_ERROR;

		auto spListResourceTemplatesResult = std::make_shared<ListResourceTemplatesResult>(true);
		if (!spListResourceTemplatesResult)
			return ERRNO_INTERNAL_ERROR;
		spListResourceTemplatesResult->requestId = m_spRequest->requestId;

		// TODO: 实现资源模板列表的获取逻辑

		std::string strResponse;
		if (ERRNO_OK != spListResourceTemplatesResult->Serialize(strResponse))
			return ERRNO_INTERNAL_ERROR;
		auto spTransport = CMCPSession::GetInstance().GetTransport();
		if (!spTransport)
			return ERRNO_INTERNAL_ERROR;
#ifdef _WIN32
		strResponse += "\r\n";
#else
		strResponse += "\n";
#endif // _WIN32
		if (ERRNO_OK != spTransport->Write(strResponse))
			return ERRNO_INTERNAL_ERROR;

		return ERRNO_OK;
	}
}