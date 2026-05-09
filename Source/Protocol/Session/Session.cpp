#include "Session.h"
#include "../Public/PublicDef.h"
#include "../Public/StringHelper.h"
#include "../Message/BasicMessage.h"
#include "../Message/Notification.h"
#include "../Message/Request.h"
#include "../Message/Response.h"
#include "../Task/BasicTask.h"

#include <memory>
#include <algorithm>
#include <json/json.h>

namespace MCP
{
	CMCPSession CMCPSession::s_Instance;

	CMCPSession& CMCPSession::GetInstance()
	{
		return s_Instance;
	}

	int CMCPSession::Ready()
	{
		if (!m_spTransport)
			return ERRNO_INTERNAL_ERROR;

		int iErrCode = ERRNO_OK;
		iErrCode = m_spTransport->Connect();
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		return iErrCode;
	}

	int CMCPSession::Run()
	{
		if (!m_spTransport)
			return ERRNO_INTERNAL_ERROR;

		int iErrCode = ERRNO_OK;

		while (true)
		{
			std::string strIncomingMsg;
			iErrCode = m_spTransport->Read(strIncomingMsg);
			if (ERRNO_OK == iErrCode)
			{
				auto vecStrObj = StringHelper::split_json_objects_string(strIncomingMsg);
				for (auto& strObj : vecStrObj)
				{
					std::shared_ptr<MCP::Message> spMsg;
					iErrCode = ParseMessage(strObj, spMsg);
					iErrCode = ProcessMessage(iErrCode, spMsg);
				}
			}
			else
			{
				break;
			}
		}

		return iErrCode;
	}

	int CMCPSession::Terminate()
	{
		StopAsyncTaskThread();
		if (m_upTaskThread && m_upTaskThread->joinable())
			m_upTaskThread->join();

		if (!m_spTransport)
			return ERRNO_INTERNAL_ERROR;

		int iErrCode = ERRNO_OK;
		iErrCode = m_spTransport->Disconnect();
		if (ERRNO_OK != iErrCode)
			return iErrCode;

		return iErrCode;
	}

	int CMCPSession::CancelTask(const MCP::RequestId& requestId)
	{
		return CancelAsyncTask(requestId);
	}

	int CMCPSession::ProcessMessage(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg)
	{
		if (!spMsg || !spMsg->IsValid())
			return ERRNO_INTERNAL_ERROR;

		switch (spMsg->eMessageCategory)
		{
			case MessageCategory_Request:
			{
				return ProcessRequest(iErrCode, spMsg);
			} break;
			case MessageCategory_Response:
			{
				return ProcessResponse(iErrCode, spMsg);
			} break;
			case MessageCategory_Notification:
			{
				return ProcessNotification(iErrCode, spMsg);
			} break;
			default: break;
		}

		return ERRNO_INTERNAL_ERROR;
	}

	int CMCPSession::ProcessRequest(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg)
	{
		std::shared_ptr<MCP::Request> spRequest{ nullptr };
		std::string strMessage;

		if (ERRNO_OK != iErrCode)
		{
			goto PROC_END;
		}
		if (!spMsg || !spMsg->IsValid())
		{
			iErrCode = ERRNO_INTERNAL_ERROR;
			goto PROC_END;
		}
		spRequest = std::dynamic_pointer_cast<MCP::Request>(spMsg);
		if (!spRequest)
		{
			iErrCode = ERRNO_INTERNAL_ERROR;
			goto PROC_END;
		}
		m_hashMessage[MessageCategory_Request].push_back(spMsg);

		switch (spRequest->eMessageType)
		{
		case MessageType_InitializeRequest:
		{
			if (CMCPSession::SessionState_Original != CMCPSession::GetInstance().GetSessionState())
			{
				strMessage = ERROR_MESSAGE_INVALID_REQUEST;
				iErrCode = ERRNO_INVALID_REQUEST;
				goto PROC_END;
			}

			auto spTask = std::make_shared<ProcessInitializeRequest>(spRequest);
			if (!spTask)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			iErrCode = spTask->Execute();
			if (ERRNO_OK != iErrCode)
			{
				goto PROC_END;
			}

			iErrCode = SwitchState(SessionState_Initializing);

		} break;
		case MessageType_ListToolsRequest:
		{
			if (CMCPSession::SessionState_Initialized != CMCPSession::GetInstance().GetSessionState())
			{
				strMessage = ERROR_MESSAGE_INVALID_REQUEST;
				iErrCode = ERRNO_INVALID_REQUEST;
				goto PROC_END;
			}

			auto spTask = std::make_shared<ProcessListToolsRequest>(spRequest);
			if (!spTask)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}

			iErrCode = spTask->Execute();

		} break;
		case MessageType_CallToolRequest:
		{
			if (CMCPSession::SessionState_Initialized != CMCPSession::GetInstance().GetSessionState())
			{
				iErrCode = ERRNO_INVALID_REQUEST;
				goto PROC_END;
			}

			auto spCallToolRequest = std::dynamic_pointer_cast<MCP::CallToolRequest>(spRequest);
			if (!spCallToolRequest)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			auto spProcessCallToolRequest = CMCPSession::GetInstance().GetServerCallToolsTask(spCallToolRequest->strName);
			if (!spProcessCallToolRequest)
			{
				strMessage = ERROR_MESSAGE_INVALID_PARAMS;
				iErrCode = ERRNO_INVALID_PARAMS;
				goto PROC_END;
			}
			auto spNewTask = spProcessCallToolRequest->Clone();
			if (!spNewTask)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			auto spNewProcessCallToolRequest = std::dynamic_pointer_cast<MCP::ProcessCallToolRequest>(spNewTask);
			if (!spNewProcessCallToolRequest)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			spNewProcessCallToolRequest->SetRequest(spRequest);
			iErrCode = CommitAsyncTask(spNewProcessCallToolRequest);

		} break;
		case MessageType_PingRequest:
		{
			if (CMCPSession::SessionState_Initialized != CMCPSession::GetInstance().GetSessionState())
			{
				iErrCode = ERRNO_INVALID_REQUEST;
				goto PROC_END;
			}

			auto spTask = std::make_shared<ProcessPingRequest>(spRequest);
			if (!spTask)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}

			iErrCode = spTask->Execute();

		} break;
		case MessageType_ListResourcesRequest:
		{
			if (CMCPSession::SessionState_Initialized != CMCPSession::GetInstance().GetSessionState())
			{
				iErrCode = ERRNO_INVALID_REQUEST;
				goto PROC_END;
			}

			auto spTask = std::make_shared<ProcessListResourcesRequest>(spRequest);
			if (!spTask)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}

			iErrCode = spTask->Execute();

		} break;
		case MessageType_ReadResourceRequest:
		{
			if (CMCPSession::SessionState_Initialized != CMCPSession::GetInstance().GetSessionState())
			{
				iErrCode = ERRNO_INVALID_REQUEST;
				goto PROC_END;
			}

			auto spReadResourceRequest = std::dynamic_pointer_cast<MCP::ReadResourceRequest>(spRequest);
			if (!spReadResourceRequest)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			auto spProcessReadResourceRequest = CMCPSession::GetInstance().GetServerReadResourceTask(spReadResourceRequest->strUri);
			if (!spProcessReadResourceRequest)
			{
				strMessage = ERROR_MESSAGE_INVALID_PARAMS;
				iErrCode = ERRNO_INVALID_PARAMS;
				goto PROC_END;
			}
			auto spNewTask = spProcessReadResourceRequest->Clone();
			if (!spNewTask)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			auto spNewProcessReadResourceRequest = std::dynamic_pointer_cast<MCP::ProcessReadResourceRequest>(spNewTask);
			if (!spNewProcessReadResourceRequest)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			spNewProcessReadResourceRequest->SetRequest(spRequest);
			iErrCode = CommitAsyncTask(spNewProcessReadResourceRequest);

		} break;
		case MessageType_SubscribeResourceRequest:
		{
			if (CMCPSession::SessionState_Initialized != CMCPSession::GetInstance().GetSessionState())
			{
				iErrCode = ERRNO_INVALID_REQUEST;
				goto PROC_END;
			}

			auto spSubscribeRequest = std::dynamic_pointer_cast<MCP::SubscribeResourceRequest>(spRequest);
			if (!spSubscribeRequest)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			auto spProcessSubscribeResourceRequest = CMCPSession::GetInstance().GetServerSubscribeResourceTask(spSubscribeRequest->strUri);
			if (!spProcessSubscribeResourceRequest)
			{
				strMessage = ERROR_MESSAGE_INVALID_PARAMS;
				iErrCode = ERRNO_INVALID_PARAMS;
				goto PROC_END;
			}
			auto spNewTask = spProcessSubscribeResourceRequest->Clone();
			if (!spNewTask)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			auto spNewProcessSubscribeResourceRequest = std::dynamic_pointer_cast<MCP::ProcessSubscribeResourceRequest>(spNewTask);
			if (!spNewProcessSubscribeResourceRequest)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			spNewProcessSubscribeResourceRequest->SetRequest(spRequest);
			iErrCode = CommitAsyncTask(spNewProcessSubscribeResourceRequest);
			if (MCP::ERRNO_OK == iErrCode)
			{
				m_hashUri2SubscribeResourceTaskId[spSubscribeRequest->strUri] = spSubscribeRequest->requestId;
			}

		} break;
		case MessageType_UnsubscribeResourceRequest:
		{
			if (CMCPSession::SessionState_Initialized != CMCPSession::GetInstance().GetSessionState())
			{
				iErrCode = ERRNO_INVALID_REQUEST;
				goto PROC_END;
			}

			auto spUnsubscribeRequest = std::dynamic_pointer_cast<MCP::UnsubscribeResourceRequest>(spRequest);
			if (!spUnsubscribeRequest)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			auto spTask = std::make_shared<ProcessUnsubscribeResourceRequest>(spRequest);
			if (!spTask)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			if (m_hashUri2SubscribeResourceTaskId.find(spUnsubscribeRequest->strUri) == m_hashUri2SubscribeResourceTaskId.end())
			{
				iErrCode = ERRNO_INVALID_PARAMS;
				goto PROC_END;
			}
			spTask->SetRequestIdToUnsubscribe(m_hashUri2SubscribeResourceTaskId[spUnsubscribeRequest->strUri]);
			iErrCode = spTask->Execute();
			if (MCP::ERRNO_OK == iErrCode)
			{
				m_hashUri2SubscribeResourceTaskId.erase(spUnsubscribeRequest->strUri);
			}

		} break;
		case MessageType_ListResourceTemplatesRequest:
		{
			if (CMCPSession::SessionState_Initialized != CMCPSession::GetInstance().GetSessionState())
			{
				iErrCode = ERRNO_INVALID_REQUEST;
				goto PROC_END;
			}

			auto spTask = std::make_shared<ProcessListResourceTemplatesRequest>(spRequest);
			if (!spTask)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}

			iErrCode = spTask->Execute();

		} break;
		case MessageType_ListPromptsRequest:
		{
			if (CMCPSession::SessionState_Initialized != CMCPSession::GetInstance().GetSessionState())
			{
				iErrCode = ERRNO_INVALID_REQUEST;
				goto PROC_END;
			}

			auto spTask = std::make_shared<ProcessListPromptsRequest>(spRequest);
			if (!spTask)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}

			iErrCode = spTask->Execute();

		} break;
		case MessageType_GetPromptRequest:
		{
			if (CMCPSession::SessionState_Initialized != CMCPSession::GetInstance().GetSessionState())
			{
				iErrCode = ERRNO_INVALID_REQUEST;
				goto PROC_END;
			}

			auto spGetPromptRequest = std::dynamic_pointer_cast<MCP::GetPromptRequest>(spRequest);
			if (!spGetPromptRequest)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			auto spProcessGetPromptRequest = CMCPSession::GetInstance().GetServerGetPromptTask(spGetPromptRequest->strName);
			if (!spProcessGetPromptRequest)
			{
				strMessage = ERROR_MESSAGE_INVALID_PARAMS;
				iErrCode = ERRNO_INVALID_PARAMS;
				goto PROC_END;
			}
			auto spNewTask = spProcessGetPromptRequest->Clone();
			if (!spNewTask)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			auto spNewProcessGetPromptRequest = std::dynamic_pointer_cast<MCP::ProcessGetPromptRequest>(spNewTask);
			if (!spNewProcessGetPromptRequest)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			spNewProcessGetPromptRequest->SetRequest(spRequest);
			iErrCode = CommitAsyncTask(spNewProcessGetPromptRequest);

		} break;
		case MessageType_CompleteRequest:
		{
			if (CMCPSession::SessionState_Initialized != CMCPSession::GetInstance().GetSessionState())
			{
				iErrCode = ERRNO_INVALID_REQUEST;
				goto PROC_END;
			}

			auto spCompleteRequest = std::dynamic_pointer_cast<MCP::CompleteRequest>(spRequest);
			if (!spCompleteRequest)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			std::string strTaskKey = spCompleteRequest->strRefType + ":";
			if (spCompleteRequest->strRefType == CONST_REF_PROMPT)
			{
				strTaskKey += spCompleteRequest->promptRef.strName;
			}
			else if (spCompleteRequest->strRefType == CONST_REF_RESOURCE)
			{
				strTaskKey += spCompleteRequest->resourceRef.strUri;
			}
			else
			{
				strMessage = ERROR_MESSAGE_INVALID_PARAMS;
				iErrCode = ERRNO_INVALID_PARAMS;
				goto PROC_END;
			}
			auto spProcessCompleteRequest = CMCPSession::GetInstance().GetServerCompleteTask(strTaskKey);
			if (!spProcessCompleteRequest)
			{
				strMessage = ERROR_MESSAGE_INVALID_PARAMS;
				iErrCode = ERRNO_INVALID_PARAMS;
				goto PROC_END;
			}
			auto spNewTask = spProcessCompleteRequest->Clone();
			if (!spNewTask)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			auto spNewProcessCompleteRequest = std::dynamic_pointer_cast<MCP::ProcessCompleteRequest>(spNewTask);
			if (!spNewProcessCompleteRequest)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}
			spNewProcessCompleteRequest->SetRequest(spRequest);
			iErrCode = CommitAsyncTask(spNewProcessCompleteRequest);

		} break;
		case MessageType_SetLevelRequest:
		{
			if (CMCPSession::SessionState_Initialized != CMCPSession::GetInstance().GetSessionState())
			{
				iErrCode = ERRNO_INVALID_REQUEST;
				goto PROC_END;
			}

			auto spTask = std::make_shared<ProcessSetLevelRequest>(spRequest);
			if (!spTask)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}

			iErrCode = spTask->Execute();

		} break;
		default: break;
		}

	PROC_END:
		if (ERRNO_OK != iErrCode)
		{
			auto spTask = std::make_shared<ProcessErrorRequest>(spRequest);
			if (spTask)
			{
				spTask->SetErrorCode(iErrCode);
				spTask->SetErrorMessage(strMessage);
				spTask->Execute();
			}
		}

		return iErrCode;
	}

	int CMCPSession::ProcessResponse(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg)
	{
		if (!spMsg || !spMsg->IsValid())
			return ERRNO_INTERNAL_ERROR;
		auto spResponse = std::dynamic_pointer_cast<MCP::Response>(spMsg);
		if (!spResponse)
			return ERRNO_INTERNAL_ERROR;
		m_hashMessage[MessageCategory_Response].push_back(spMsg);

		return ERRNO_INTERNAL_ERROR;
	}

	int CMCPSession::ProcessNotification(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg)
	{
		if (!spMsg || !spMsg->IsValid())
			return ERRNO_INTERNAL_ERROR;
		auto spNotification = std::dynamic_pointer_cast<MCP::Notification>(spMsg);
		if (!spNotification)
			return ERRNO_INTERNAL_ERROR;
		m_hashMessage[MessageCategory_Notification].push_back(spMsg);

		if (ERRNO_OK != iErrCode)
		{
			return ERRNO_OK;
		}

		switch (spNotification->eMessageType)
		{
			case MessageType_InitializedNotification:
			{
				int iErrCode = SwitchState(SessionState_Initialized);
				if (ERRNO_OK == iErrCode)
				{
					return StartAsyncTaskThread();
				}
				return iErrCode;

			} break;
			case MessageType_CancelledNotification:
			{
				auto spCancelledNotification = std::dynamic_pointer_cast<MCP::CancelledNotification>(spNotification);
				if (spCancelledNotification && spCancelledNotification->IsValid())
				{
					return CancelAsyncTask(spCancelledNotification->requestId);
				}

			} break;
			default: break;
		}

		return ERRNO_INTERNAL_ERROR;
	}

	int CMCPSession::ParseMessage(const std::string& strMsg, std::shared_ptr<MCP::Message>& spMsg)
	{
		if (strMsg.empty())
			return ERRNO_PARSE_ERROR;

		Json::Reader reader;
		Json::Value jVal;
		if (!reader.parse(strMsg, jVal) || !jVal.isObject())
			return ERRNO_PARSE_ERROR;

		MessageCategory eCategory{ MessageCategory_Unknown };
		if (jVal.isMember(MSG_KEY_ID))
		{
			if (jVal.isMember(MSG_KEY_METHOD))
			{
				eCategory = MessageCategory_Request;
			}
			else
			{
				eCategory = MessageCategory_Response;
			}
		}
		else
		{
			if (jVal.isMember(MSG_KEY_METHOD))
			{
				eCategory = MessageCategory_Notification;
			}
		}

		if (MessageCategory_Unknown == eCategory)
			return ERRNO_PARSE_ERROR;

		switch (eCategory)
		{
			case MessageCategory_Request:
			{
				return ParseRequest(strMsg, spMsg);
			} break;
			case MessageCategory_Response:
			{
				return ParseResponse(strMsg, spMsg);
			} break;
			case MessageCategory_Notification:
			{
				return ParseNotification(strMsg, spMsg);
			} break;
			default: break;
		}

		return ERRNO_INTERNAL_ERROR;
	}

	int CMCPSession::ParseRequest(const std::string& strMsg, std::shared_ptr<MCP::Message>& spMsg)
	{
		auto spRequest = std::make_shared<MCP::Request>(MessageType_Unknown, false);
		if (!spRequest)
			return ERRNO_PARSE_ERROR;

		int iErrCode = ERRNO_OK;
		iErrCode = spRequest->Deserialize(strMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;
		if (!spRequest->IsValid())
			return ERRNO_INVALID_REQUEST;

		if (spRequest->strMethod.compare(METHOD_INITIALIZE) == 0)
		{
			auto spInitializeRequest = std::make_shared<MCP::InitializeRequest>(true);
			if (!spInitializeRequest)
				return ERRNO_PARSE_ERROR;

			iErrCode = spInitializeRequest->Deserialize(strMsg);
			if (ERRNO_OK != iErrCode)
				return ERRNO_INVALID_REQUEST;

			spMsg = spInitializeRequest;

			return ERRNO_OK;
		}
		else if (spRequest->strMethod.compare(METHOD_TOOLS_LIST) == 0)
		{
			auto spListToolsRequest = std::make_shared<MCP::ListToolsRequest>(true);
			if (!spListToolsRequest)
				return ERRNO_PARSE_ERROR;

			iErrCode = spListToolsRequest->Deserialize(strMsg);
			if (ERRNO_OK != iErrCode)
				return ERRNO_INVALID_REQUEST;

			spMsg = spListToolsRequest;

			return ERRNO_OK;
		}
		else if (spRequest->strMethod.compare(METHOD_TOOLS_CALL) == 0)
		{
			auto spCallToolRequest = std::make_shared<MCP::CallToolRequest>(true);
			if (!spCallToolRequest)
				return ERRNO_PARSE_ERROR;

			iErrCode = spCallToolRequest->Deserialize(strMsg);
			if (ERRNO_OK != iErrCode)
				return ERRNO_INVALID_REQUEST;

			spMsg = spCallToolRequest;

			return ERRNO_OK;
		}
		else if (spRequest->strMethod.compare(METHOD_PING) == 0)
		{
			auto spPingRequest = std::make_shared<MCP::PingRequest>(true);
			if (!spPingRequest)
				return ERRNO_PARSE_ERROR;

			iErrCode = spPingRequest->Deserialize(strMsg);
			if (ERRNO_OK != iErrCode)
				return ERRNO_INVALID_REQUEST;

			spMsg = spPingRequest;

			return ERRNO_OK;
		}
		else if (spRequest->strMethod.compare(METHOD_RESOURCES_LIST) == 0)
		{
			auto spListResourcesRequest = std::make_shared<MCP::ListResourcesRequest>(true);
			if (!spListResourcesRequest)
				return ERRNO_PARSE_ERROR;

			iErrCode = spListResourcesRequest->Deserialize(strMsg);
			if (ERRNO_OK != iErrCode)
				return ERRNO_INVALID_REQUEST;

			spMsg = spListResourcesRequest;

			return ERRNO_OK;
		}
		else if (spRequest->strMethod.compare(METHOD_RESOURCES_READ) == 0)
		{
			auto spReadResourceRequest = std::make_shared<MCP::ReadResourceRequest>(true);
			if (!spReadResourceRequest)
				return ERRNO_PARSE_ERROR;

			iErrCode = spReadResourceRequest->Deserialize(strMsg);
			if (ERRNO_OK != iErrCode)
				return ERRNO_INVALID_REQUEST;

			spMsg = spReadResourceRequest;

			return ERRNO_OK;
		}
		else if (spRequest->strMethod.compare(METHOD_RESOURCES_SUBSCRIBE) == 0)
		{
			auto spSubscribeResourceRequest = std::make_shared<MCP::SubscribeResourceRequest>(true);
			if (!spSubscribeResourceRequest)
				return ERRNO_PARSE_ERROR;

			iErrCode = spSubscribeResourceRequest->Deserialize(strMsg);
			if (ERRNO_OK != iErrCode)
				return ERRNO_INVALID_REQUEST;

			spMsg = spSubscribeResourceRequest;
			return ERRNO_OK;
		}
		else if (spRequest->strMethod.compare(METHOD_RESOURCES_UNSUBSCRIBE) == 0)
		{
			auto spUnsubscribeResourceRequest = std::make_shared<MCP::UnsubscribeResourceRequest>(true);
			if (!spUnsubscribeResourceRequest)
				return ERRNO_PARSE_ERROR;

			iErrCode = spUnsubscribeResourceRequest->Deserialize(strMsg);
			if (ERRNO_OK != iErrCode)
				return ERRNO_INVALID_REQUEST;

			spMsg = spUnsubscribeResourceRequest;
			return ERRNO_OK;
		}
		else if (spRequest->strMethod.compare(METHOD_RESOURCES_TEMPLATES_LIST) == 0)
		{
			auto spListResourceTemplatesRequest = std::make_shared<MCP::ListResourceTemplatesRequest>(true);
			if (!spListResourceTemplatesRequest)
				return ERRNO_PARSE_ERROR;

			iErrCode = spListResourceTemplatesRequest->Deserialize(strMsg);
			if (ERRNO_OK != iErrCode)
				return ERRNO_INVALID_REQUEST;

			spMsg = spListResourceTemplatesRequest;

			return ERRNO_OK;
		}
		else if (spRequest->strMethod.compare(METHOD_PROMPTS_LIST) == 0)
		{
			auto spListPromptsRequest = std::make_shared<MCP::ListPromptsRequest>(true);
			if (!spListPromptsRequest)
				return ERRNO_PARSE_ERROR;

			iErrCode = spListPromptsRequest->Deserialize(strMsg);
			if (ERRNO_OK != iErrCode)
				return ERRNO_INVALID_REQUEST;

			spMsg = spListPromptsRequest;

			return ERRNO_OK;
		}
		else if (spRequest->strMethod.compare(METHOD_PROMPTS_GET) == 0)
		{
			auto spGetPromptRequest = std::make_shared<MCP::GetPromptRequest>(true);
			if (!spGetPromptRequest)
				return ERRNO_PARSE_ERROR;

			iErrCode = spGetPromptRequest->Deserialize(strMsg);
			if (ERRNO_OK != iErrCode)
				return ERRNO_INVALID_REQUEST;

			spMsg = spGetPromptRequest;

			return ERRNO_OK;
		}
		else if (spRequest->strMethod.compare(METHOD_COMPLETION_COMPLETE) == 0)
		{
			auto spCompleteRequest = std::make_shared<MCP::CompleteRequest>(true);
			if (!spCompleteRequest)
				return ERRNO_PARSE_ERROR;

			iErrCode = spCompleteRequest->Deserialize(strMsg);
			if (ERRNO_OK != iErrCode)
				return ERRNO_INVALID_REQUEST;

			spMsg = spCompleteRequest;

			return ERRNO_OK;
		}
		else if (spRequest->strMethod.compare(METHOD_LOGGING_SET_LEVEL) == 0)
		{
			auto spSetLevelRequest = std::make_shared<MCP::SetLevelRequest>(true);
			if (!spSetLevelRequest)
				return ERRNO_PARSE_ERROR;

			iErrCode = spSetLevelRequest->Deserialize(strMsg);
			if (ERRNO_OK != iErrCode)
				return ERRNO_INVALID_REQUEST;

			spMsg = spSetLevelRequest;

			return ERRNO_OK;
		}

		return ERRNO_INTERNAL_ERROR;
	}

	int CMCPSession::ParseResponse(const std::string& strMsg, std::shared_ptr<MCP::Message>& spMsg)
	{
		return ERRNO_INTERNAL_ERROR;
	}

	int CMCPSession::ParseNotification(const std::string& strMsg, std::shared_ptr<MCP::Message>& spMsg)
	{
		auto spNotification = std::make_shared<MCP::Notification>(MessageType_Unknown, false);
		if (!spNotification)
			return ERRNO_PARSE_ERROR;

		int iErrCode = ERRNO_OK;
		iErrCode = spNotification->Deserialize(strMsg);
		if (ERRNO_OK != iErrCode)
			return iErrCode;
		if (!spNotification->IsValid())
			return ERRNO_INVALID_NOTIFICATION;

		if (spNotification->strMethod.compare(METHOD_NOTIFICATION_INITIALIZED) == 0)
		{
			auto spInitializedNotification = std::make_shared<MCP::InitializedNotification>(true);
			if (!spInitializedNotification)
				return ERRNO_PARSE_ERROR;

			iErrCode = spInitializedNotification->Deserialize(strMsg);
			if (ERRNO_OK != iErrCode)
				return ERRNO_INVALID_NOTIFICATION;

			spMsg = spInitializedNotification;

			return ERRNO_OK;
		}
		else if (spNotification->strMethod.compare(METHOD_NOTIFICATION_CANCELLED) == 0)
		{
			auto spCancelledNotification = std::make_shared<MCP::CancelledNotification>(true);
			if (!spCancelledNotification)
				return ERRNO_PARSE_ERROR;

			iErrCode = spCancelledNotification->Deserialize(strMsg);
			if (ERRNO_OK != iErrCode)
				return ERRNO_INVALID_NOTIFICATION;

			spMsg = spCancelledNotification;

			return ERRNO_OK;
		}

		return ERRNO_INTERNAL_ERROR;
	}

	void CMCPSession::SetTransport(const std::shared_ptr<CMCPTransport>& spTransport)
	{
		m_spTransport = spTransport;
	}

	void CMCPSession::SetServerInfo(const MCP::Implementation& impl)
	{
		m_serverInfo = impl;
	}

	void CMCPSession::SetServerCapabilities(const MCP::ServerCapabilities& capabilities)
	{
		m_capabilities = capabilities;
	}

	void CMCPSession::SetServerToolsPagination(bool bPagination)
	{
		m_bToolsPagination = bPagination;
	}

	void CMCPSession::SetServerTools(const std::vector<MCP::Tool>& tools)
	{
		m_tools = tools;
	}

	void CMCPSession::SetServerResourcesPagination(bool bPagination)
	{
		m_bResourcesPagination = bPagination;
	}

	void CMCPSession::SetServerResources(const std::vector<MCP::Resource>& resources)
	{
		m_resources = resources;
	}

	void CMCPSession::SetServerResourceTemplatesPagination(bool bPagination)
	{
		m_bResourceTemplatesPagination = bPagination;
	}

	void CMCPSession::SetServerResourceTemplates(const std::vector<MCP::ResourceTemplate>& resourceTemplates)
	{
		m_resourceTemplates = resourceTemplates;
	}

	void CMCPSession::SetServerPromptsPagination(bool bPagination)
	{
		m_bPromptsPagination = bPagination;
	}

	void CMCPSession::SetServerPrompts(const std::vector<MCP::Prompt>& prompts)
	{
		m_prompts = prompts;
	}

	void CMCPSession::SetServerCallToolsTasks(const std::unordered_map<std::string, std::shared_ptr<MCP::ProcessCallToolRequest>>& hashCallToolsTasks)
	{
		m_hashCallToolsTasks = hashCallToolsTasks;
	}

	void CMCPSession::SetServerReadResourceTasks(const std::unordered_map<std::string, std::shared_ptr<MCP::ProcessReadResourceRequest>>& hashReadResourceTasks)
	{
		m_hashReadResourceTasks = hashReadResourceTasks;
	}

	void CMCPSession::SetServerSubscribeResourceTasks(const std::unordered_map<std::string, std::shared_ptr<MCP::ProcessSubscribeResourceRequest>>& hashSubscribeResourceTasks)
	{
		m_hashSubscribeResourceTasks = hashSubscribeResourceTasks;
	}

	void CMCPSession::SetServerGetPromptTasks(const std::unordered_map<std::string, std::shared_ptr<MCP::ProcessGetPromptRequest>>& hashGetPromptTasks)
	{
		m_hashGetPromptTasks = hashGetPromptTasks;
	}

	void CMCPSession::SetServerCompleteTasks(const std::unordered_map<std::string, std::shared_ptr<MCP::ProcessCompleteRequest>>& hashCompleteTasks)
	{
		m_hashCompleteTasks = hashCompleteTasks;
	}

	MCP::Implementation CMCPSession::GetServerInfo() const
	{
		return m_serverInfo;
	}

	MCP::ServerCapabilities CMCPSession::GetServerCapabilities() const
	{
		return m_capabilities;
	}

	bool CMCPSession::GetServerToolsPagination() const
	{
		return m_bToolsPagination;
	}

	std::vector<MCP::Tool> CMCPSession::GetServerTools() const
	{
		return m_tools;
	}

	bool CMCPSession::GetServerResourcesPagination() const
	{
		return m_bResourcesPagination;
	}

	std::vector<MCP::Resource> CMCPSession::GetServerResources() const
	{
		return m_resources;
	}

	bool CMCPSession::GetServerResourceTemplatesPagination() const
	{
		return m_bResourceTemplatesPagination;
	}

	std::vector<MCP::ResourceTemplate> CMCPSession::GetServerResourceTemplates() const
	{
		return m_resourceTemplates;
	}

	bool CMCPSession::GetServerPromptsPagination() const
	{
		return m_bPromptsPagination;
	}

	std::vector<MCP::Prompt> CMCPSession::GetServerPrompts() const
	{
		return m_prompts;
	}

	std::shared_ptr<CMCPTransport> CMCPSession::GetTransport() const
	{
		return m_spTransport;
	}

	int CMCPSession::SwitchState(SessionState eState)
	{
		if (SessionState_Initializing == eState)
		{
			if (SessionState_Original != m_eSessionState)
				return ERRNO_INTERNAL_ERROR;
		}

		if (SessionState_Initialized == eState)
		{
			if (SessionState_Initializing != m_eSessionState)
				return ERRNO_INTERNAL_ERROR;
		}

		m_eSessionState = eState;

		return ERRNO_OK;
	}

	CMCPSession::SessionState CMCPSession::GetSessionState() const
	{
		return m_eSessionState;
	}

	std::shared_ptr<MCP::ProcessRequest> CMCPSession::GetServerCallToolsTask(const std::string& strToolName)
	{
		if (m_hashCallToolsTasks.count(strToolName) > 0)
			return m_hashCallToolsTasks[strToolName];

		return nullptr;
	}

	std::shared_ptr<MCP::ProcessRequest> CMCPSession::GetServerReadResourceTask(const std::string& strResourceUri)
	{
		if (m_hashReadResourceTasks.count(strResourceUri) > 0)
			return m_hashReadResourceTasks[strResourceUri];

		return nullptr;
	}

	std::shared_ptr<MCP::ProcessRequest> CMCPSession::GetServerSubscribeResourceTask(const std::string& strResourceUri)
	{
		if (m_hashSubscribeResourceTasks.count(strResourceUri) > 0)
			return m_hashSubscribeResourceTasks[strResourceUri];

		return nullptr;
	}

	std::shared_ptr<MCP::ProcessRequest> CMCPSession::GetServerGetPromptTask(const std::string& strPromptName)
	{
		if (m_hashGetPromptTasks.count(strPromptName) > 0)
			return m_hashGetPromptTasks[strPromptName];

		return nullptr;
	}

	std::shared_ptr<MCP::ProcessRequest> CMCPSession::GetServerCompleteTask(const std::string& strTaskKey)
	{
		if (m_hashCompleteTasks.count(strTaskKey) > 0)
			return m_hashCompleteTasks[strTaskKey];

		return nullptr;
	}

	void CMCPSession::SetLoggingLevel(const MCP::LoggingLevel& level)
	{
		m_loggingLevel = level;
	}

	MCP::LoggingLevel CMCPSession::GetLoggingLevel() const
	{
		return m_loggingLevel;
	}

	int CMCPSession::SendLogMessage(const MCP::LoggingLevel& level, const std::string& strLogger, const Json::Value& jData)
	{
		if (!m_spTransport)
			return ERRNO_INTERNAL_ERROR;

		auto currentLevel = GetLoggingLevel();
		if (level.eLevel < currentLevel.eLevel)
		{
			return ERRNO_OK;
		}

		MCP::LoggingMessageNotification notification(false);
		notification.strMethod = METHOD_NOTIFICATION_MESSAGE;
		notification.level = level;
		notification.strLogger = strLogger;
		notification.jData = jData;

		std::string strNotification;
		if (ERRNO_OK != notification.Serialize(strNotification))
			return ERRNO_INTERNAL_ERROR;

		return m_spTransport->Write(strNotification);
	}

	int CMCPSession::CommitAsyncTask(const std::shared_ptr<MCP::CMCPTask>& spTask)
	{
		if (!spTask)
			return ERRNO_INTERNAL_ERROR;

		if (m_bRunAsyncTask) 
		{
			std::unique_lock<std::mutex> _lock(m_mtxAsyncThread);

			if (!m_bRunAsyncTask)
			{
				_lock.unlock();
				return ERRNO_OK;
			}

			m_deqAsyncTasks.push_back(spTask);

			_lock.unlock();
			m_cvAsyncThread.notify_one();
		}

		return ERRNO_OK;
	}

	int CMCPSession::CancelAsyncTask(const MCP::RequestId& requestId)
	{
		if (!requestId.IsValid())
			return ERRNO_INVALID_NOTIFICATION;

		if (m_bRunAsyncTask)
		{
			std::unique_lock<std::mutex> _lock(m_mtxAsyncThread);

			if (!m_bRunAsyncTask)
			{
				_lock.unlock();
				return ERRNO_OK;
			}

			m_vecCancelledTaskIds.push_back(requestId);

			_lock.unlock();
			m_cvAsyncThread.notify_one();
		}

		return ERRNO_OK;
	}

	int CMCPSession::StartAsyncTaskThread()
	{
		m_upTaskThread = std::make_unique<std::thread>(&CMCPSession::AsyncThreadProc, this);
		if (!m_upTaskThread)
			return ERRNO_INTERNAL_ERROR;

		return ERRNO_OK;
	}

	int CMCPSession::StopAsyncTaskThread()
	{
		std::unique_lock<std::mutex> _lock(m_mtxAsyncThread);
		m_bRunAsyncTask = false;
		_lock.unlock();

		m_cvAsyncThread.notify_all();

		return ERRNO_OK;
	}

	int CMCPSession::AsyncThreadProc()
	{
		while (m_bRunAsyncTask) 
		{
			std::unique_lock<std::mutex> _lock(m_mtxAsyncThread);

			// Wait for tasks
			m_cvAsyncThread.wait(
				_lock, 
				[this]() {
				return !m_deqAsyncTasks.empty() || !m_vecCancelledTaskIds.empty() || !m_bRunAsyncTask;
				});

			// Break the loop and clean up tasks
			if (!m_bRunAsyncTask)
			{
				_lock.unlock();

				std::for_each(m_vecAsyncTasksCache.begin(), m_vecAsyncTasksCache.end(), [this](auto& spTask)
					{
						if (spTask)
						{
							spTask->Cancel();
						}
					});

				break;
			}

			// Process new pending tasks
			std::vector<std::shared_ptr<MCP::CMCPTask>> vecTasks;
			while (!m_deqAsyncTasks.empty())
			{
				auto spTask = m_deqAsyncTasks.front();
				vecTasks.push_back(spTask);
				m_deqAsyncTasks.pop_front();
			}

			//  Process task cancellation requests
			std::for_each(m_vecAsyncTasksCache.begin(), m_vecAsyncTasksCache.end(), [this](auto& spTask) 
				{
					if (spTask)
					{
						auto spProcessRequestTask = std::dynamic_pointer_cast<MCP::ProcessRequest>(spTask);
						if (spProcessRequestTask)
						{
							auto spRequest = spProcessRequestTask->GetRequest();
							if (spRequest)
							{
								auto itrFound = std::find_if(m_vecCancelledTaskIds.begin(), m_vecCancelledTaskIds.end(), [&spRequest](auto& requestId)
									{
										if (requestId.IsEqual(spRequest->requestId))
											return true;
										return false;
									}
								);
								if (itrFound != m_vecCancelledTaskIds.end())
									spTask->Cancel();
							}
						}
					}
				});
			m_vecCancelledTaskIds.clear();
			_lock.unlock();

			// Clean up completed tasks
			m_vecAsyncTasksCache.erase(
				std::remove_if(m_vecAsyncTasksCache.begin(), m_vecAsyncTasksCache.end(), [](auto spTask) 
					{
						if (!spTask)
							return true;
						if (spTask->IsFinished() || spTask->IsCancelled())
							return true;
						return false;
					}),
				m_vecAsyncTasksCache.end());

			// Cache new tasks
			for (auto& spTask : vecTasks)
			{
				if (spTask)
				{
					if (ERRNO_OK == spTask->Execute())
					{
						m_vecAsyncTasksCache.push_back(spTask);
					}
				}
			}
		}

		return ERRNO_OK;
	}
}