#include "BasicTask.h"
#include "../Message/Notification.h"
#include "../Internal/MessageAccess.h"
#include "../Internal/TaskServices.h"
#include "../Internal/TaskAccess.h"

namespace MCP
{
	namespace
	{
		// A paged list request with an invalid cursor is answered by an error only in
		// the stateless era, where every request must carry its own id and context.
		// Legacy callers keep the legacy code path, so the shared helper preserves
		// that split in one place instead of repeating it per list task.
		int WriteInvalidCursorErrorResponse(const std::shared_ptr<MCP::Request>& spRequest, std::string& strResponse)
		{
			auto spErrorResponse = std::make_shared<ErrorResponse>(true);
			if (!spErrorResponse)
				return ERRNO_INTERNAL_ERROR;

			spErrorResponse->iCode = ERRNO_INVALID_PARAMS;
			spErrorResponse->strMesage = ERROR_MESSAGE_INVALID_PARAMS;
			if (MessageContextAccess::Get(*spRequest).IsStateless())
			{
				spErrorResponse->requestId = spRequest->requestId;
				MessageContextAccess::Copy(*spErrorResponse, *spRequest);
			}

			return spErrorResponse->Serialize(strResponse);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessRequest
	bool ProcessRequest::IsValid() const
	{
		if (!m_spRequest || !m_spServices)
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

	ProcessRequest::ProcessRequest(const ProcessRequest& other)
		: CMCPTask(other)
		, m_spRequest(other.m_spRequest)
		, m_spServices(other.m_spServices)
		, m_nStateFlags(other.m_nStateFlags.load())
		, m_nAsyncSequence(other.m_nAsyncSequence)
	{
	}

	ProcessRequest& ProcessRequest::operator=(const ProcessRequest& other)
	{
		if (this != &other)
		{
			m_spRequest = other.m_spRequest;
			m_spServices = other.m_spServices;
			m_nStateFlags.store(other.m_nStateFlags.load());
			m_nAsyncSequence = other.m_nAsyncSequence;
		}

		return *this;
	}

	void ProcessRequest::SetTaskServices(const std::shared_ptr<MCP::TaskServices>& spServices)
	{
		m_spServices = spServices;
	}

	const std::shared_ptr<MCP::TaskServices>& ProcessRequest::GetTaskServices() const
	{
		return m_spServices;
	}

	uint64_t ProcessRequest::GetAsyncSequence() const
	{
		return m_nAsyncSequence;
	}

	void ProcessRequest::SetAsyncSequence(uint64_t nSequence)
	{
		m_nAsyncSequence = nSequence;
	}

	bool ProcessRequest::IsStale() const
	{
		return 0 != (m_nStateFlags.load() & StateFlag_Stale);
	}

	void ProcessRequest::MarkStale()
	{
		m_nStateFlags.fetch_or(StateFlag_Stale);
	}

	void ProcessRequest::MarkFinished()
	{
		m_nStateFlags.fetch_or(StateFlag_Finished);
	}

	bool ProcessRequest::IsFinishedFlag() const
	{
		return 0 != (m_nStateFlags.load() & StateFlag_Finished);
	}

	void ProcessRequest::MarkCancelledFlag()
	{
		m_nStateFlags.fetch_or(StateFlag_Cancelled);
	}

	bool ProcessRequest::IsCancelledFlag() const
	{
		return 0 != (m_nStateFlags.load() & StateFlag_Cancelled);
	}

	int ProcessRequest::WriteToClient(const std::string& strMessage)
	{
		// A task retired by a session reset must stay silent: its client is gone and
		// the transport now belongs to someone else.
		if (IsStale())
			return ERRNO_OK;

		if (!m_spServices)
			return ERRNO_INTERNAL_ERROR;

		auto spTransport = m_spServices->GetTransport();
		if (!spTransport)
			return ERRNO_INTERNAL_ERROR;

		return spTransport->Write(strMessage);
	}

	int ProcessRequest::NotifyLogMessage(LoggingLevelValue eLevel, const std::string& strLogger, const Json::Value& jData)
	{
		if (!m_spServices)
			return ERRNO_INTERNAL_ERROR;

		if (IsStale())
			return ERRNO_OK;

		MCP::LoggingLevel level;
		level.eLevel = eLevel;

		// Stateless requests carry their own threshold; without an explicit
		// logLevel the request must not receive notifications/message at all.
		if (m_spRequest && MessageContextAccess::Get(*m_spRequest).Supports(Feature_LogLevelInMeta))
		{
			if (m_spRequest->meta.strLogLevel.empty())
				return ERRNO_OK;

			MCP::LoggingLevel requestLevel;
			Json::Value jLevel(Json::objectValue);
			jLevel[MSG_KEY_LEVEL] = m_spRequest->meta.strLogLevel;
			if (ERRNO_OK != requestLevel.DoDeserialize(jLevel))
				return ERRNO_OK;

			if (static_cast<int>(eLevel) < static_cast<int>(requestLevel.eLevel))
				return ERRNO_OK;

			return GetTaskServices()->WriteLogNotification(level, strLogger, jData);
		}

		return GetTaskServices()->SendLogMessage(level, strLogger, jData);
	}

	int ProcessRequest::SerializeResult(const std::shared_ptr<MCP::Response>& spResult, std::string& strResponse)
	{
		if (!spResult || !m_spServices || IsStale())
			return ERRNO_INTERNAL_ERROR;

		GetTaskServices()->PrepareResult(m_spRequest, spResult);

		return spResult->Serialize(strResponse);
	}

	int ProcessRequest::ValidateInputRequired(const Json::Value& jInputRequests, const std::string& strRequestState) const
	{
		if (!m_spRequest || !MessageContextAccess::Get(*m_spRequest).Supports(Feature_Mrtr))
			return ERRNO_INVALID_PARAMS;

		bool bHasInputRequests = jInputRequests.isObject() && !jInputRequests.empty();
		if (!bHasInputRequests && strRequestState.empty())
			return ERRNO_INVALID_PARAMS;

		if (!bHasInputRequests)
			return ERRNO_OK;

		const MCP::ClientCapabilities& clientCapabilities = m_spRequest->meta.clientCapabilities;
		for (auto it = jInputRequests.begin(); it != jInputRequests.end(); ++it)
		{
			const Json::Value& jInputRequest = *it;
			if (!jInputRequest.isObject() || !jInputRequest.isMember(MSG_KEY_METHOD))
				return ERRNO_INVALID_PARAMS;

			std::string strMethod = jInputRequest[MSG_KEY_METHOD].asString();
			if (METHOD_SAMPLING_CREATE_MESSAGE == strMethod)
			{
				if (!clientCapabilities.SupportsCapability(MSG_KEY_SAMPLING))
					return ERRNO_MISSING_REQUIRED_CLIENT_CAPABILITY;
			}
			else if (METHOD_ROOTS_LIST == strMethod)
			{
				if (!clientCapabilities.SupportsCapability(MSG_KEY_ROOTS))
					return ERRNO_MISSING_REQUIRED_CLIENT_CAPABILITY;
			}
			else if (METHOD_ELICITATION_CREATE == strMethod)
			{
				std::string strMode = MSG_KEY_FORM;
				if (jInputRequest.isMember(MSG_KEY_PARAMS) && jInputRequest[MSG_KEY_PARAMS].isObject()
					&& jInputRequest[MSG_KEY_PARAMS].isMember(MSG_KEY_MODE)
					&& jInputRequest[MSG_KEY_PARAMS][MSG_KEY_MODE].isString())
				{
					strMode = jInputRequest[MSG_KEY_PARAMS][MSG_KEY_MODE].asString();
				}

				if (!clientCapabilities.SupportsElicitationMode(strMode))
					return ERRNO_MISSING_REQUIRED_CLIENT_CAPABILITY;
			}
			else
			{
				return ERRNO_INVALID_PARAMS;
			}
		}

		return ERRNO_OK;
	}

	int ProcessRequest::SendInputRequiredResult(const std::shared_ptr<MCP::Response>& spResult,
		const Json::Value& jInputRequests, const std::string& strRequestState)
	{
		if (!spResult || !m_spServices)
			return ERRNO_INTERNAL_ERROR;

		if (IsStale())
			return ERRNO_OK;

		spResult->strResultType = RESULT_TYPE_INPUT_REQUIRED;
		spResult->jInputRequests = jInputRequests;
		spResult->strRequestState = strRequestState;

		std::string strResponse;
		if (ERRNO_OK != SerializeResult(spResult, strResponse))
			return ERRNO_INTERNAL_ERROR;

		return WriteToClient(strResponse);
	}

	int ProcessRequest::SendMissingClientCapability(const Json::Value& jRequiredCapabilities)
	{
		if (!m_spRequest || !MessageContextAccess::Get(*m_spRequest).Supports(Feature_Mrtr))
			return ERRNO_INVALID_PARAMS;

		if (!jRequiredCapabilities.isObject() || jRequiredCapabilities.empty())
			return ERRNO_INVALID_PARAMS;

		if (IsStale())
			return ERRNO_OK;

		Json::Value jErrorData(Json::objectValue);
		jErrorData[MSG_KEY_REQUIRED_CAPABILITIES] = jRequiredCapabilities;

		auto spTask = std::make_shared<ProcessErrorRequest>(m_spRequest);
		if (!spTask)
			return ERRNO_INTERNAL_ERROR;

		TaskServicesAccess::Set(*spTask, m_spServices);

		spTask->SetErrorCode(ERRNO_MISSING_REQUIRED_CLIENT_CAPABILITY);
		spTask->SetErrorMessage(ERROR_MESSAGE_MISSING_REQUIRED_CLIENT_CAPABILITY);
		spTask->SetErrorData(jErrorData);

		return spTask->Execute();
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

		return WriteToClient(strResponse);
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
		spErrorResponse->jErrorData = m_jErrorData;

		std::string strResponse;
		if (ERRNO_OK != spErrorResponse->Serialize(strResponse))
			return ERRNO_INTERNAL_ERROR;

		return WriteToClient(strResponse);
	}

	void ProcessErrorRequest::SetErrorCode(int iCode)
	{
		m_iCode = iCode;
	}

	void ProcessErrorRequest::SetErrorMessage(const std::string& strMessage)
	{
		m_strMessage = strMessage;
	}

	void ProcessErrorRequest::SetErrorData(const Json::Value& jErrorData)
	{
		m_jErrorData = jErrorData;
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
		// The context carries the negotiated handshake revision, so the response
		// reports the version the session will actually speak.
		std::shared_ptr<const ProtocolProfile> pHandshakeProfile = MessageContextAccess::Get(*m_spRequest).pProfile;
		if (!pHandshakeProfile)
			pHandshakeProfile = GetTaskServices()->GetDefaultHandshakeProfile();
		spInitializeResult->strProtocolVersion = pHandshakeProfile ? pHandshakeProfile->strVersion : PROTOCOL_VER;
		spInitializeResult->capabilities = GetTaskServices()->GetServerCapabilities();
		spInitializeResult->implServerInfo = GetTaskServices()->GetServerInfo();
		std::string strResponse;
		if (ERRNO_OK != SerializeResult(spInitializeResult, strResponse))
			return ERRNO_INTERNAL_ERROR;

		return WriteToClient(strResponse);
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

		bool bPagination = GetTaskServices()->GetServerToolsPagination();		
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

				auto vecServerTools = GetTaskServices()->GetServerTools();
				if (bValidCursor)
				{
					if (nCursor >= vecServerTools.size())
					{
						bValidCursor = false;
					}
				}

				if (!bValidCursor)
				{
					if (ERRNO_OK != WriteInvalidCursorErrorResponse(spListToolRequest, strResponse))
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
				auto vecServerTools = GetTaskServices()->GetServerTools();
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
			spListToolsResult->vecTools = GetTaskServices()->GetServerTools();
		}

		if (spListToolsResult)
		{
			if (ERRNO_OK != SerializeResult(spListToolsResult, strResponse))
				return ERRNO_INTERNAL_ERROR;
		}

		if (!strResponse.empty())
		{
			if (ERRNO_OK != WriteToClient(strResponse))
				return ERRNO_INTERNAL_ERROR;
		}

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessCallToolRequest
	bool ProcessCallToolRequest::IsFinished() const
	{
		return IsFinishedFlag();
	}

	bool ProcessCallToolRequest::IsCancelled() const
	{
		return IsCancelledFlag();
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
			if (!m_spServices)
				return ERRNO_INTERNAL_ERROR;

			if (IsStale())
				return ERRNO_OK;

			MCP::ProgressNotification progressNotification(false);
			progressNotification.strMethod = METHOD_NOTIFICATION_PROGRESS;
			progressNotification.progressToken = m_spRequest->progressToken;
			progressNotification.iProgress = iProgress;
			progressNotification.iTotal = iTotal;

			std::string strNotification;
			if (ERRNO_OK != progressNotification.Serialize(strNotification))
				return ERRNO_INTERNAL_ERROR;

			return WriteToClient(strNotification);
		}

		return ERRNO_OK;
	}

	int ProcessCallToolRequest::NotifyResult(std::shared_ptr<MCP::CallToolResult> spResult)
	{
		if (!spResult)
		{
			MarkFinished();
			return ERRNO_INTERNAL_ERROR;
		}

		int iErrCode = ERRNO_INTERNAL_ERROR;
		std::string strResponse;
		if (ERRNO_OK == SerializeResult(spResult, strResponse))
			iErrCode = WriteToClient(strResponse);

		// Mark finished only after the write attempt, so the worker never reaps a
		// task that is still producing its response.
		MarkFinished();
		return iErrCode;
	}

	int ProcessCallToolRequest::NotifyCancelled()
	{
		MarkCancelledFlag();
		return ERRNO_OK;
	}

	int ProcessCallToolRequest::NotifyInputRequired(const Json::Value& jInputRequests, const std::string& strRequestState)
	{
		int iErrCode = ValidateInputRequired(jInputRequests, strRequestState);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		auto spResult = BuildResult();
		if (!spResult)
			return ERRNO_INTERNAL_ERROR;

		int iSendErrCode = SendInputRequiredResult(spResult, jInputRequests, strRequestState);
		MarkFinished();
		return iSendErrCode;
	}

	int ProcessCallToolRequest::NotifyMissingClientCapability(const Json::Value& jRequiredCapabilities)
	{
		int iErrCode = SendMissingClientCapability(jRequiredCapabilities);
		MarkFinished();
		return iErrCode;
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

		auto spListResourcesRequest = std::dynamic_pointer_cast<ListResourcesRequest>(m_spRequest);
		if (!spListResourcesRequest)
			return ERRNO_INTERNAL_ERROR;
		
		std::shared_ptr<ListResourcesResult> spListResourcesResult = nullptr;
		std::string strResponse;

		bool bPagination = GetTaskServices()->GetServerResourcesPagination();
		if (bPagination)
		{
			if (!spListResourcesRequest->strCursor.empty())
			{
				bool bValidCursor = true;
				unsigned nCursor = 0;
				try
				{
					nCursor = std::stoul(spListResourcesRequest->strCursor);
				}
				catch (const std::invalid_argument&)
				{
					bValidCursor = false;
				}
				catch (const std::out_of_range&)
				{
					bValidCursor = false;
				}

				auto vecServerResources = GetTaskServices()->GetServerResources();
				if (bValidCursor)
				{
					if (nCursor >= vecServerResources.size())
					{
						bValidCursor = false;
					}
				}

				if (!bValidCursor)
				{
					if (ERRNO_OK != WriteInvalidCursorErrorResponse(spListResourcesRequest, strResponse))
						return ERRNO_INTERNAL_ERROR;
				}
				else
				{
					spListResourcesResult = std::make_shared<ListResourcesResult>(true);
					if (!spListResourcesResult)
						return ERRNO_INTERNAL_ERROR;
					spListResourcesResult->requestId = spListResourcesRequest->requestId;
					spListResourcesResult->vecResources.clear();
					spListResourcesResult->vecResources.push_back(vecServerResources[nCursor]);
					if (nCursor < vecServerResources.size() - 1)
					{
						spListResourcesResult->strNextCursor = std::to_string(nCursor + 1);
					}
				}
			}
			else
			{
				spListResourcesResult = std::make_shared<ListResourcesResult>(true);
				if (!spListResourcesResult)
					return ERRNO_INTERNAL_ERROR;
				spListResourcesResult->requestId = spListResourcesRequest->requestId;
				auto vecServerResources = GetTaskServices()->GetServerResources();
				spListResourcesResult->vecResources.clear();
				if (vecServerResources.size() > 0)
					spListResourcesResult->vecResources.push_back(vecServerResources[0]);
				if (vecServerResources.size() > 1)
					spListResourcesResult->strNextCursor = std::to_string(1);
			}
		}
		else
		{
			spListResourcesResult = std::make_shared<ListResourcesResult>(true);
			if (!spListResourcesResult)
				return ERRNO_INTERNAL_ERROR;
			spListResourcesResult->requestId = spListResourcesRequest->requestId;
			spListResourcesResult->vecResources = GetTaskServices()->GetServerResources();
		}

		if (spListResourcesResult)
		{
			if (ERRNO_OK != SerializeResult(spListResourcesResult, strResponse))
				return ERRNO_INTERNAL_ERROR;
		}

		if (!strResponse.empty())
		{
			if (ERRNO_OK != WriteToClient(strResponse))
				return ERRNO_INTERNAL_ERROR;
		}

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessReadResourceRequest
	bool ProcessReadResourceRequest::IsFinished() const
	{
		return IsFinishedFlag();
	}

	bool ProcessReadResourceRequest::IsCancelled() const
	{
		return IsCancelledFlag();
	}

	std::shared_ptr<MCP::ReadResourceResult> ProcessReadResourceRequest::BuildResult()
	{
		if (!IsValid())
			return nullptr;

		auto spReadResourceResult = std::make_shared<ReadResourceResult>(true);
		if (!spReadResourceResult)
			return nullptr;
		spReadResourceResult->requestId = m_spRequest->requestId;

		return spReadResourceResult;
	}

	int ProcessReadResourceRequest::NotifyProgress(int iProgress, int iTotal)
	{
		if (!m_spRequest)
			return ERRNO_INTERNAL_ERROR;

		if (m_spRequest->progressToken.IsValid())
		{
			if (!m_spServices)
				return ERRNO_INTERNAL_ERROR;

			if (IsStale())
				return ERRNO_OK;

			MCP::ProgressNotification progressNotification(false);
			progressNotification.strMethod = METHOD_NOTIFICATION_PROGRESS;
			progressNotification.progressToken = m_spRequest->progressToken;
			progressNotification.iProgress = iProgress;
			progressNotification.iTotal = iTotal;

			std::string strNotification;
			if (ERRNO_OK != progressNotification.Serialize(strNotification))
				return ERRNO_INTERNAL_ERROR;

			return WriteToClient(strNotification);
		}

		return ERRNO_OK;
	}

	int ProcessReadResourceRequest::NotifyResult(std::shared_ptr<MCP::ReadResourceResult> spResult)
	{
		if (!spResult)
		{
			MarkFinished();
			return ERRNO_INTERNAL_ERROR;
		}

		int iErrCode = ERRNO_INTERNAL_ERROR;
		std::string strResponse;
		if (ERRNO_OK == SerializeResult(spResult, strResponse))
			iErrCode = WriteToClient(strResponse);

		MarkFinished();
		return iErrCode;
	}

	int ProcessReadResourceRequest::NotifyError(int iCode, const std::string& strMessage, const Json::Value& jErrData)
	{
		if (IsStale())
		{
			MarkFinished();
			return ERRNO_OK;
		}

		// The error code for a missing resource is owned by the active profile.
		if (ERRNO_RESOURCE_NOT_FOUND == iCode && m_spRequest && MessageContextAccess::Get(*m_spRequest).pProfile)
			iCode = MessageContextAccess::Get(*m_spRequest).pProfile->iResourceNotFoundCode;

		auto spTask = std::make_shared<ProcessErrorRequest>(m_spRequest);
		if (!spTask)
			return ERRNO_INTERNAL_ERROR;

		TaskServicesAccess::Set(*spTask, GetTaskServices());
		spTask->SetErrorCode(iCode);
		spTask->SetErrorMessage(strMessage);
		spTask->SetErrorData(jErrData);
		int iErrCode = spTask->Execute();
		MarkFinished();
		return iErrCode;
	}

	int ProcessReadResourceRequest::NotifyCancelled()
	{
		MarkCancelledFlag();
		return ERRNO_OK;
	}

	int ProcessReadResourceRequest::NotifyInputRequired(const Json::Value& jInputRequests, const std::string& strRequestState)
	{
		int iErrCode = ValidateInputRequired(jInputRequests, strRequestState);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		auto spResult = BuildResult();
		if (!spResult)
			return ERRNO_INTERNAL_ERROR;

		int iSendErrCode = SendInputRequiredResult(spResult, jInputRequests, strRequestState);
		MarkFinished();
		return iSendErrCode;
	}

	int ProcessReadResourceRequest::NotifyMissingClientCapability(const Json::Value& jRequiredCapabilities)
	{
		int iErrCode = SendMissingClientCapability(jRequiredCapabilities);
		MarkFinished();
		return iErrCode;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessSubscribeResourceRequest
	bool ProcessSubscribeResourceRequest::IsFinished() const
	{
		return IsFinishedFlag();
	}

	bool ProcessSubscribeResourceRequest::IsCancelled() const
	{
		return IsCancelledFlag();
	}

	int ProcessSubscribeResourceRequest::NotifyResult()
	{
		if (!m_spRequest || !m_spServices)
			return ERRNO_INTERNAL_ERROR;

		if (IsStale())
			return ERRNO_OK;

		auto spEmptyResponse = std::make_shared<EmptyResponse>(true);
		if (!spEmptyResponse)
			return ERRNO_INTERNAL_ERROR;
		spEmptyResponse->requestId = m_spRequest->requestId;

		std::string strResponse;
		if (ERRNO_OK != spEmptyResponse->Serialize(strResponse))
			return ERRNO_INTERNAL_ERROR;

		return WriteToClient(strResponse);
	}

	int ProcessSubscribeResourceRequest::NotifyError(int iCode, const std::string& strMessage, const Json::Value& jErrData)
	{
		if (!m_spRequest || !m_spServices)
		{
			MarkFinished();
			return ERRNO_INTERNAL_ERROR;
		}

		if (IsStale())
		{
			MarkFinished();
			return ERRNO_OK;
		}

		auto spErrorResponse = std::make_shared<ErrorResponse>(true);
		if (!spErrorResponse)
		{
			MarkFinished();
			return ERRNO_INTERNAL_ERROR;
		}
		spErrorResponse->requestId = m_spRequest->requestId;
		spErrorResponse->iCode = iCode;
		spErrorResponse->strMesage = strMessage;
		spErrorResponse->jErrorData = jErrData;

		int iErrCode = ERRNO_INTERNAL_ERROR;
		std::string strResponse;
		if (ERRNO_OK == spErrorResponse->Serialize(strResponse))
			iErrCode = WriteToClient(strResponse);

		MarkFinished();
		return iErrCode;
	}

	int ProcessSubscribeResourceRequest::NotifyUpdated()
	{
		if (!m_spRequest || !m_spServices)
			return ERRNO_INTERNAL_ERROR;

		if (IsStale())
			return ERRNO_OK;

		auto spResourceUpdatedNotification = std::make_shared<ResourceUpdatedNotification>(false);
		if (!spResourceUpdatedNotification)
			return ERRNO_INTERNAL_ERROR;
		auto spSubscribeResourceRequest = std::dynamic_pointer_cast<MCP::SubscribeResourceRequest>(m_spRequest);
		if (!spSubscribeResourceRequest)
			return ERRNO_INTERNAL_ERROR;
		spResourceUpdatedNotification->strMethod = MCP::METHOD_NOTIFICATION_RESOURCES_UPDATED;
		spResourceUpdatedNotification->strUri = spSubscribeResourceRequest->strUri;

		std::string strNotification;
		if (ERRNO_OK != spResourceUpdatedNotification->Serialize(strNotification))
			return ERRNO_INTERNAL_ERROR;

		return WriteToClient(strNotification);
	}

	int ProcessSubscribeResourceRequest::NotifyCancelled()
	{
		MarkCancelledFlag();
		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessUnsubscribeResourceRequest
	void ProcessUnsubscribeResourceRequest::SetRequestIdToUnsubscribe(const MCP::RequestId& requestId)
	{
		m_requestIdToUnsubscribe = requestId;
	}

	int ProcessUnsubscribeResourceRequest::NotifyResult()
	{
		if (!m_spRequest || !m_spServices)
			return ERRNO_INTERNAL_ERROR;

		if (IsStale())
			return ERRNO_OK;

		auto spEmptyResponse = std::make_shared<EmptyResponse>(true);
		if (!spEmptyResponse)
			return ERRNO_INTERNAL_ERROR;
		spEmptyResponse->requestId = m_spRequest->requestId;

		std::string strResponse;
		if (ERRNO_OK != spEmptyResponse->Serialize(strResponse))
			return ERRNO_INTERNAL_ERROR;

		return WriteToClient(strResponse);
	}

	int ProcessUnsubscribeResourceRequest::NotifyError(int iCode, const std::string& strMessage, const Json::Value& jErrData)
	{
		if (!m_spRequest || !m_spServices)
			return ERRNO_INTERNAL_ERROR;

		if (IsStale())
			return ERRNO_OK;

		auto spErrorResponse = std::make_shared<ErrorResponse>(true);
		if (!spErrorResponse)
			return ERRNO_INTERNAL_ERROR;
		spErrorResponse->requestId = m_spRequest->requestId;
		spErrorResponse->iCode = iCode;
		spErrorResponse->strMesage = strMessage;
		spErrorResponse->jErrorData = jErrData;

		std::string strResponse;
		if (ERRNO_OK != spErrorResponse->Serialize(strResponse))
			return ERRNO_INTERNAL_ERROR;

		return WriteToClient(strResponse);
	}

	std::shared_ptr<CMCPTask> ProcessUnsubscribeResourceRequest::Clone() const
	{
		return nullptr;
	}

	int ProcessUnsubscribeResourceRequest::Execute()
	{
		if (!IsValid())
			return ERRNO_INTERNAL_ERROR;

		int iErrCode = GetTaskServices()->CancelTask(m_requestIdToUnsubscribe);
		if (MCP::ERRNO_OK == iErrCode)
		{
			NotifyResult();
		}
		else
		{
			Json::Value jErrData(Json::objectValue);
			auto spUnsubscribeResourceRequest = std::dynamic_pointer_cast<MCP::UnsubscribeResourceRequest>(m_spRequest);
			if (spUnsubscribeResourceRequest)
			{
				jErrData[MCP::MSG_KEY_DATA] = spUnsubscribeResourceRequest->strUri;
			}
			NotifyError(iErrCode, u8"Failed to unsubscribe resource", jErrData);
		}

		return iErrCode;
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

		auto spListResourceTemplatesRequest = std::dynamic_pointer_cast<ListResourceTemplatesRequest>(m_spRequest);
		if (!spListResourceTemplatesRequest)
			return ERRNO_INTERNAL_ERROR;
		
		std::shared_ptr<ListResourceTemplatesResult> spListResourceTemplatesResult = nullptr;
		std::string strResponse;

		bool bPagination = GetTaskServices()->GetServerResourceTemplatesPagination();
		if (bPagination)
		{
			if (!spListResourceTemplatesRequest->strCursor.empty())
			{
				bool bValidCursor = true;
				unsigned nCursor = 0;
				try
				{
					nCursor = std::stoul(spListResourceTemplatesRequest->strCursor);
				}
				catch (const std::invalid_argument&)
				{
					bValidCursor = false;
				}
				catch (const std::out_of_range&)
				{
					bValidCursor = false;
				}

				auto vecServerResourceTemplates = GetTaskServices()->GetServerResourceTemplates();
				if (bValidCursor)
				{
					if (nCursor >= vecServerResourceTemplates.size())
					{
						bValidCursor = false;
					}
				}

				if (!bValidCursor)
				{
					if (ERRNO_OK != WriteInvalidCursorErrorResponse(spListResourceTemplatesRequest, strResponse))
						return ERRNO_INTERNAL_ERROR;
				}
				else
				{
					spListResourceTemplatesResult = std::make_shared<ListResourceTemplatesResult>(true);
					if (!spListResourceTemplatesResult)
						return ERRNO_INTERNAL_ERROR;
					spListResourceTemplatesResult->requestId = spListResourceTemplatesRequest->requestId;
					spListResourceTemplatesResult->vecResourceTemplates.clear();
					spListResourceTemplatesResult->vecResourceTemplates.push_back(vecServerResourceTemplates[nCursor]);
					if (nCursor < vecServerResourceTemplates.size() - 1)
					{
						spListResourceTemplatesResult->strNextCursor = std::to_string(nCursor + 1);
					}
				}
			}
			else
			{
				spListResourceTemplatesResult = std::make_shared<ListResourceTemplatesResult>(true);
				if (!spListResourceTemplatesResult)
					return ERRNO_INTERNAL_ERROR;
				spListResourceTemplatesResult->requestId = spListResourceTemplatesRequest->requestId;
				auto vecServerResourceTemplates = GetTaskServices()->GetServerResourceTemplates();
				spListResourceTemplatesResult->vecResourceTemplates.clear();
				if (vecServerResourceTemplates.size() > 0)
					spListResourceTemplatesResult->vecResourceTemplates.push_back(vecServerResourceTemplates[0]);
				if (vecServerResourceTemplates.size() > 1)
					spListResourceTemplatesResult->strNextCursor = std::to_string(1);
			}
		}
		else
		{
			spListResourceTemplatesResult = std::make_shared<ListResourceTemplatesResult>(true);
			if (!spListResourceTemplatesResult)
				return ERRNO_INTERNAL_ERROR;
			spListResourceTemplatesResult->requestId = spListResourceTemplatesRequest->requestId;
			spListResourceTemplatesResult->vecResourceTemplates = GetTaskServices()->GetServerResourceTemplates();
		}

		if (spListResourceTemplatesResult)
		{
			if (ERRNO_OK != SerializeResult(spListResourceTemplatesResult, strResponse))
				return ERRNO_INTERNAL_ERROR;
		}

		if (!strResponse.empty())
		{
			if (ERRNO_OK != WriteToClient(strResponse))
				return ERRNO_INTERNAL_ERROR;
		}

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessListPromptsRequest
	std::shared_ptr<CMCPTask> ProcessListPromptsRequest::Clone() const
	{
		return nullptr;
	}

	int ProcessListPromptsRequest::Execute()
	{
		if (!IsValid())
			return ERRNO_INTERNAL_ERROR;

		auto spListPromptsRequest = std::dynamic_pointer_cast<ListPromptsRequest>(m_spRequest);
		if (!spListPromptsRequest)
			return ERRNO_INTERNAL_ERROR;
		
		std::shared_ptr<ListPromptsResult> spListPromptsResult = nullptr;
		std::string strResponse;

		bool bPagination = GetTaskServices()->GetServerPromptsPagination();
		if (bPagination)
		{
			if (!spListPromptsRequest->strCursor.empty())
			{
				bool bValidCursor = true;
				unsigned nCursor = 0;
				try
				{
					nCursor = std::stoul(spListPromptsRequest->strCursor);
				}
				catch (const std::invalid_argument&)
				{
					bValidCursor = false;
				}
				catch (const std::out_of_range&)
				{
					bValidCursor = false;
				}

				auto vecServerPrompts = GetTaskServices()->GetServerPrompts();
				if (bValidCursor)
				{
					if (nCursor >= vecServerPrompts.size())
					{
						bValidCursor = false;
					}
				}

				if (!bValidCursor)
				{
					if (ERRNO_OK != WriteInvalidCursorErrorResponse(spListPromptsRequest, strResponse))
						return ERRNO_INTERNAL_ERROR;
				}
				else
				{
					spListPromptsResult = std::make_shared<ListPromptsResult>(true);
					if (!spListPromptsResult)
						return ERRNO_INTERNAL_ERROR;
					spListPromptsResult->requestId = spListPromptsRequest->requestId;
					spListPromptsResult->vecPrompts.clear();
					spListPromptsResult->vecPrompts.push_back(vecServerPrompts[nCursor]);
					if (nCursor < vecServerPrompts.size() - 1)
					{
						spListPromptsResult->strNextCursor = std::to_string(nCursor + 1);
					}
				}
			}
			else
			{
				spListPromptsResult = std::make_shared<ListPromptsResult>(true);
				if (!spListPromptsResult)
					return ERRNO_INTERNAL_ERROR;
				spListPromptsResult->requestId = spListPromptsRequest->requestId;
				auto vecServerPrompts = GetTaskServices()->GetServerPrompts();
				spListPromptsResult->vecPrompts.clear();
				if (vecServerPrompts.size() > 0)
					spListPromptsResult->vecPrompts.push_back(vecServerPrompts[0]);
				if (vecServerPrompts.size() > 1)
					spListPromptsResult->strNextCursor = std::to_string(1);
			}
		}
		else
		{
			spListPromptsResult = std::make_shared<ListPromptsResult>(true);
			if (!spListPromptsResult)
				return ERRNO_INTERNAL_ERROR;
			spListPromptsResult->requestId = spListPromptsRequest->requestId;
			spListPromptsResult->vecPrompts = GetTaskServices()->GetServerPrompts();
		}

		if (spListPromptsResult)
		{
			if (ERRNO_OK != SerializeResult(spListPromptsResult, strResponse))
				return ERRNO_INTERNAL_ERROR;
		}

		if (!strResponse.empty())
		{
			if (ERRNO_OK != WriteToClient(strResponse))
				return ERRNO_INTERNAL_ERROR;
		}

		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessGetPromptRequest
	bool ProcessGetPromptRequest::IsFinished() const
	{
		return IsFinishedFlag();
	}

	bool ProcessGetPromptRequest::IsCancelled() const
	{
		return IsCancelledFlag();
	}

	std::shared_ptr<MCP::GetPromptResult> ProcessGetPromptRequest::BuildResult()
	{
		auto spResult = std::make_shared<GetPromptResult>(true);
		if (!spResult)
			return nullptr;

		auto spRequest = std::dynamic_pointer_cast<GetPromptRequest>(m_spRequest);
		if (!spRequest)
			return nullptr;

		spResult->requestId = spRequest->requestId;
		return spResult;
	}

	int ProcessGetPromptRequest::NotifyProgress(int iProgress, int iTotal)
	{
		auto spRequest = std::dynamic_pointer_cast<GetPromptRequest>(m_spRequest);
		if (!spRequest)
			return ERRNO_INTERNAL_ERROR;

		if (!spRequest->progressToken.IsValid())
			return ERRNO_OK;

		if (!m_spServices)
			return ERRNO_INTERNAL_ERROR;

		if (IsStale())
			return ERRNO_OK;

		auto spProgressNotification = std::make_shared<ProgressNotification>(false);
		if (!spProgressNotification)
			return ERRNO_INTERNAL_ERROR;

		spProgressNotification->progressToken = spRequest->progressToken;
		spProgressNotification->iProgress = iProgress;
		spProgressNotification->iTotal = iTotal;

		std::string strNotification;
		if (ERRNO_OK != spProgressNotification->Serialize(strNotification))
			return ERRNO_INTERNAL_ERROR;

		return WriteToClient(strNotification);
	}

	int ProcessGetPromptRequest::NotifyResult(std::shared_ptr<MCP::GetPromptResult> spResult)
	{
		if (!spResult)
		{
			MarkFinished();
			return ERRNO_INTERNAL_ERROR;
		}

		auto spRequest = std::dynamic_pointer_cast<GetPromptRequest>(m_spRequest);
		if (!spRequest)
		{
			MarkFinished();
			return ERRNO_INTERNAL_ERROR;
		}

		spResult->requestId = spRequest->requestId;

		int iErrCode = ERRNO_INTERNAL_ERROR;
		std::string strResponse;
		if (ERRNO_OK == SerializeResult(spResult, strResponse))
			iErrCode = WriteToClient(strResponse);

		MarkFinished();
		return iErrCode;
	}

	int ProcessGetPromptRequest::NotifyError(int iCode, const std::string& strMessage, const Json::Value& jErrData)
	{
		auto spRequest = std::dynamic_pointer_cast<GetPromptRequest>(m_spRequest);
		if (!spRequest)
		{
			MarkFinished();
			return ERRNO_INTERNAL_ERROR;
		}

		if (IsStale())
		{
			MarkFinished();
			return ERRNO_OK;
		}

		auto spErrorResponse = std::make_shared<ErrorResponse>(true);
		if (!spErrorResponse)
		{
			MarkFinished();
			return ERRNO_INTERNAL_ERROR;
		}

		spErrorResponse->requestId = spRequest->requestId;
		spErrorResponse->iCode = iCode;
		spErrorResponse->strMesage = strMessage;
		spErrorResponse->jErrorData = jErrData;

		int iErrCode = ERRNO_INTERNAL_ERROR;
		std::string strResponse;
		if (ERRNO_OK == spErrorResponse->Serialize(strResponse))
			iErrCode = WriteToClient(strResponse);

		MarkFinished();
		return iErrCode;
	}

	int ProcessGetPromptRequest::NotifyCancelled()
	{
		MarkCancelledFlag();
		return ERRNO_OK;
	}

	int ProcessGetPromptRequest::NotifyInputRequired(const Json::Value& jInputRequests, const std::string& strRequestState)
	{
		int iErrCode = ValidateInputRequired(jInputRequests, strRequestState);
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		auto spResult = BuildResult();
		if (!spResult)
			return ERRNO_INTERNAL_ERROR;

		int iSendErrCode = SendInputRequiredResult(spResult, jInputRequests, strRequestState);
		MarkFinished();
		return iSendErrCode;
	}

	int ProcessGetPromptRequest::NotifyMissingClientCapability(const Json::Value& jRequiredCapabilities)
	{
		int iErrCode = SendMissingClientCapability(jRequiredCapabilities);
		MarkFinished();
		return iErrCode;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessCompleteRequest
	bool ProcessCompleteRequest::IsFinished() const
	{
		return IsFinishedFlag();
	}

	bool ProcessCompleteRequest::IsCancelled() const
	{
		return IsCancelledFlag();
	}

	std::shared_ptr<MCP::CompleteResult> ProcessCompleteRequest::BuildResult()
	{
		auto spResult = std::make_shared<CompleteResult>(true);
		if (!spResult)
			return nullptr;

		auto spRequest = std::dynamic_pointer_cast<CompleteRequest>(m_spRequest);
		if (!spRequest)
			return nullptr;

		spResult->requestId = spRequest->requestId;
		return spResult;
	}

	int ProcessCompleteRequest::NotifyProgress(int iProgress, int iTotal)
	{
		auto spRequest = std::dynamic_pointer_cast<CompleteRequest>(m_spRequest);
		if (!spRequest)
			return ERRNO_INTERNAL_ERROR;

		if (!spRequest->progressToken.IsValid())
			return ERRNO_OK;

		if (!m_spServices)
			return ERRNO_INTERNAL_ERROR;

		if (IsStale())
			return ERRNO_OK;

		auto spProgressNotification = std::make_shared<ProgressNotification>(false);
		if (!spProgressNotification)
			return ERRNO_INTERNAL_ERROR;

		spProgressNotification->progressToken = spRequest->progressToken;
		spProgressNotification->iProgress = iProgress;
		spProgressNotification->iTotal = iTotal;

		std::string strNotification;
		if (ERRNO_OK != spProgressNotification->Serialize(strNotification))
			return ERRNO_INTERNAL_ERROR;

		return WriteToClient(strNotification);
	}

	int ProcessCompleteRequest::NotifyResult(std::shared_ptr<MCP::CompleteResult> spResult)
	{
		if (!spResult)
		{
			MarkFinished();
			return ERRNO_INTERNAL_ERROR;
		}

		auto spRequest = std::dynamic_pointer_cast<CompleteRequest>(m_spRequest);
		if (!spRequest)
		{
			MarkFinished();
			return ERRNO_INTERNAL_ERROR;
		}

		spResult->requestId = spRequest->requestId;

		int iErrCode = ERRNO_INTERNAL_ERROR;
		std::string strResponse;
		if (ERRNO_OK == SerializeResult(spResult, strResponse))
			iErrCode = WriteToClient(strResponse);

		MarkFinished();
		return iErrCode;
	}

	int ProcessCompleteRequest::NotifyError(int iCode, const std::string& strMessage, const Json::Value& jErrData)
	{
		auto spRequest = std::dynamic_pointer_cast<CompleteRequest>(m_spRequest);
		if (!spRequest)
		{
			MarkFinished();
			return ERRNO_INTERNAL_ERROR;
		}

		if (IsStale())
		{
			MarkFinished();
			return ERRNO_OK;
		}

		auto spErrorResponse = std::make_shared<ErrorResponse>(true);
		if (!spErrorResponse)
		{
			MarkFinished();
			return ERRNO_INTERNAL_ERROR;
		}

		spErrorResponse->requestId = spRequest->requestId;
		spErrorResponse->iCode = iCode;
		spErrorResponse->strMesage = strMessage;
		spErrorResponse->jErrorData = jErrData;

		int iErrCode = ERRNO_INTERNAL_ERROR;
		std::string strResponse;
		if (ERRNO_OK == spErrorResponse->Serialize(strResponse))
			iErrCode = WriteToClient(strResponse);

		MarkFinished();
		return iErrCode;
	}

	int ProcessCompleteRequest::NotifyCancelled()
	{
		MarkCancelledFlag();
		return ERRNO_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessDiscoverRequest
	std::shared_ptr<CMCPTask> ProcessDiscoverRequest::Clone() const
	{
		// Discovery is handled synchronously on the reader thread.
		return nullptr;
	}

	int ProcessDiscoverRequest::Execute()
	{
		if (!IsValid())
			return ERRNO_INTERNAL_ERROR;

		auto spDiscoverResult = std::make_shared<DiscoverResult>(true);
		if (!spDiscoverResult)
			return ERRNO_INTERNAL_ERROR;

		spDiscoverResult->requestId = m_spRequest->requestId;
		spDiscoverResult->vecSupportedVersions = GetTaskServices()->GetSupportedProtocolVersions();
		spDiscoverResult->capabilities = GetTaskServices()->GetServerCapabilities();
		spDiscoverResult->strInstructions = GetTaskServices()->GetServerInstructions();

		std::string strResponse;
		if (ERRNO_OK != SerializeResult(spDiscoverResult, strResponse))
			return ERRNO_INTERNAL_ERROR;

		return WriteToClient(strResponse);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// ProcessSetLevelRequest
	std::shared_ptr<CMCPTask> ProcessSetLevelRequest::Clone() const
	{
		return nullptr;
	}

	int ProcessSetLevelRequest::Execute()
	{
		if (!IsValid())
			return ERRNO_INTERNAL_ERROR;

		auto spSetLevelRequest = std::dynamic_pointer_cast<SetLevelRequest>(m_spRequest);
		if (!spSetLevelRequest)
			return ERRNO_INTERNAL_ERROR;

		GetTaskServices()->SetLoggingLevel(spSetLevelRequest->level);

		auto spEmptyResponse = std::make_shared<EmptyResponse>(true);
		if (!spEmptyResponse)
			return ERRNO_INTERNAL_ERROR;
		spEmptyResponse->requestId = m_spRequest->requestId;

		std::string strResponse;
		if (ERRNO_OK != spEmptyResponse->Serialize(strResponse))
			return ERRNO_INTERNAL_ERROR;

		return WriteToClient(strResponse);
	}
}