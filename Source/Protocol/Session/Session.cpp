#include "Session.h"
#include "../Public/PublicDef.h"
#include "../Public/StringHelper.h"
#include "../Message/BasicMessage.h"
#include "../Message/Notification.h"
#include "../Message/Request.h"
#include "../Message/Response.h"
#include "../Task/BasicTask.h"
#include "../Internal/Protocol.h"
#include "../Internal/VersionExtras.h"
#include "../Internal/MessageAccess.h"
#include "../Internal/SessionAccess.h"
#include "../Internal/TaskServices.h"
#include "../Internal/TaskAccess.h"

#include <memory>
#include <algorithm>
#include <json/json.h>

namespace MCP
{
	namespace
	{
		const char* CONST_META_PREFIX = "io.modelcontextprotocol/";

		std::shared_ptr<const ProtocolContext> MakeProfileContext(const std::shared_ptr<const ProtocolProfile>& pProfile)
		{
			ProtocolContext ctxProtocol;
			if (pProfile)
			{
				ctxProtocol.strVersion = pProfile->strVersion;
				ctxProtocol.pProfile = pProfile;
			}
			else
			{
				ctxProtocol.strVersion.clear();
				ctxProtocol.pProfile = nullptr;
			}
			return std::make_shared<const ProtocolContext>(ctxProtocol);
		}

		bool HasModernMetaKeys(const Json::Value& jMeta)
		{
			if (!jMeta.isObject())
				return false;

			std::string strPrefix(CONST_META_PREFIX);
			for (auto it = jMeta.begin(); it != jMeta.end(); ++it)
			{
				if (0 == it.name().compare(0, strPrefix.size(), strPrefix))
					return true;
			}

			return false;
		}

		Json::Value MakeRequestIdJson(const RequestId& requestId)
		{
			Json::Value jId(Json::objectValue);
			requestId.DoSerialize(jId);
			if (jId.isMember(MSG_KEY_ID))
				return jId[MSG_KEY_ID];

			return Json::Value();
		}

		// A subscription is keyed by both the JSON type and the value of its
		// listen request id, so "7" and 7 stay independent subscriptions.
		std::string MakeRequestIdKey(const RequestId& requestId)
		{
			if (DataType_Integer == requestId.eIdDataType)
				return std::string("i:") + std::to_string(requestId.iId);

			if (DataType_String == requestId.eIdDataType)
				return std::string("s:") + requestId.strId;

			return std::string();
		}

		// Request types served by the shared handshake switch in ProcessRequest.
		// A handshake profile must declare the same method before it is served,
		// otherwise a revision would silently accept a method it never defined.
		bool IsLegacyDispatchableMessageType(MessageType eMessageType)
		{
			switch (eMessageType)
			{
			case MessageType_InitializeRequest:
			case MessageType_ListToolsRequest:
			case MessageType_CallToolRequest:
			case MessageType_PingRequest:
			case MessageType_ListResourcesRequest:
			case MessageType_ReadResourceRequest:
			case MessageType_SubscribeResourceRequest:
			case MessageType_UnsubscribeResourceRequest:
			case MessageType_ListResourceTemplatesRequest:
			case MessageType_ListPromptsRequest:
			case MessageType_GetPromptRequest:
			case MessageType_CompleteRequest:
			case MessageType_SetLevelRequest:
				return true;
			default:
				break;
			}

			return false;
		}

	// Adapter handed to every dispatched task. It keeps the task layer free of
		// the session singleton while the session stays the only owner of the
		// transport, the registrations and the protocol registry.
		class CSessionTaskServices final : public TaskServices
		{
		public:
			explicit CSessionTaskServices(CMCPSession& session)
				: m_session(session)
			{
			}

			std::shared_ptr<CMCPTransport> GetTransport() const override
			{
				return m_session.GetTransport();
			}

			void PrepareResult(const std::shared_ptr<MCP::Request>& spRequest, const std::shared_ptr<MCP::Response>& spResponse) override
			{
				SessionAccess::PrepareResult(m_session, spRequest, spResponse);
			}

			int SendLogMessage(const MCP::LoggingLevel& level, const std::string& strLogger, const Json::Value& jData) override
			{
				return m_session.SendLogMessage(level, strLogger, jData);
			}

			int WriteLogNotification(const MCP::LoggingLevel& level, const std::string& strLogger, const Json::Value& jData) override
			{
				return m_session.WriteLogNotification(level, strLogger, jData);
			}

			int CancelTask(const MCP::RequestId& requestId) override
			{
				return m_session.CancelTask(requestId);
			}

			MCP::Implementation GetServerInfo() const override
			{
				return m_session.GetServerInfo();
			}

			MCP::ServerCapabilities GetServerCapabilities() const override
			{
				return m_session.GetServerCapabilities();
			}

			std::string GetServerInstructions() const override
			{
				return m_session.GetServerInstructions();
			}

			bool GetServerToolsPagination() const override
			{
				return m_session.GetServerToolsPagination();
			}

			std::vector<MCP::Tool> GetServerTools() const override
			{
				return m_session.GetServerTools();
			}

			bool GetServerResourcesPagination() const override
			{
				return m_session.GetServerResourcesPagination();
			}

			std::vector<MCP::Resource> GetServerResources() const override
			{
				return m_session.GetServerResources();
			}

			bool GetServerResourceTemplatesPagination() const override
			{
				return m_session.GetServerResourceTemplatesPagination();
			}

			std::vector<MCP::ResourceTemplate> GetServerResourceTemplates() const override
			{
				return m_session.GetServerResourceTemplates();
			}

			bool GetServerPromptsPagination() const override
			{
				return m_session.GetServerPromptsPagination();
			}

			std::vector<MCP::Prompt> GetServerPrompts() const override
			{
				return m_session.GetServerPrompts();
			}

			void SetLoggingLevel(const MCP::LoggingLevel& level) override
			{
				m_session.SetLoggingLevel(level);
			}

			std::shared_ptr<const ProtocolProfile> GetDefaultHandshakeProfile() const override
			{
				return ProtocolRegistryAccess::Get(m_session).GetDefaultHandshakeProfile();
			}

			std::vector<std::string> GetSupportedProtocolVersions() const override
			{
				return ProtocolRegistryAccess::Get(m_session).GetSupportedVersions();
			}

		private:
			CMCPSession& m_session;
		};
	}

	CMCPSession CMCPSession::s_Instance;

	CMCPSession::CMCPSession()
		: m_upRegistry(new CProtocolRegistry(CProtocolRegistry::GetGlobalRegistry()))
	{
		m_spTaskServices = std::make_shared<CSessionTaskServices>(*this);
	}

	template <typename TTask>
	std::shared_ptr<TTask> CMCPSession::MakeProcessTask(const std::shared_ptr<MCP::Request>& spRequest)
	{
		auto spTask = std::make_shared<TTask>(spRequest);
		TaskServicesAccess::Set(*spTask, m_spTaskServices);

		return spTask;
	}

	void CMCPSession::BindTaskServices(const std::shared_ptr<MCP::CMCPTask>& spTask)
	{
		auto spProcessTask = std::dynamic_pointer_cast<MCP::ProcessRequest>(spTask);
		if (spProcessTask)
			TaskServicesAccess::Set(*spProcessTask, m_spTaskServices);
	}

	CMCPSession::~CMCPSession() = default;

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

		// Close every live subscription before the transport goes away so the
		// client still observes a complete result for its listen request.
		CloseSubscriptions();

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

	int CMCPSession::ResetForNewClient()
	{
		// Stop and join the async task thread that was spawned for the previous client's
		// `notifications/initialized`. AsyncThreadProc's exit path already cancels every
		// task still sitting in m_vecAsyncTasksCache (see the `!m_bRunAsyncTask` branch),
		// so we do NOT re-cancel them here to avoid double-emitting cancellation side
		// effects from user-supplied tool implementations.
		if (m_upTaskThread)
		{
			StopAsyncTaskThread();
			if (m_upTaskThread->joinable())
				m_upTaskThread->join();
			m_upTaskThread.reset();
		}

		// After join(), the async thread no longer touches any of the members below.
		// The reader thread (single caller of ResetForNewClient) is the only owner of
		// m_hashMessage / m_hashUri2SubscribeResourceTaskId / m_eSessionState, so no
		// lock is needed for those. The async-thread-private containers are cleared
		// under m_mtxAsyncThread purely for contract cleanliness -- there is no live
		// contender at this point.
		{
			std::unique_lock<std::mutex> _lock(m_mtxAsyncThread);
			m_deqAsyncTasks.clear();
			m_vecCancelledTaskIds.clear();
			m_vecAsyncTasksCache.clear();
		}

		// Roll back the MCP protocol state machine to the pre-handshake state so the next
		// client's `initialize` request is accepted (see ProcessRequest / SwitchState).
		m_eSessionState = SessionState_Original;

		// Drop cached per-connection message history and subscription bookkeeping. The
		// server-side static registrations (tools/resources/prompts/handlers/capabilities)
		// remain untouched -- they were populated at server start and are not per-client.
		m_hashMessage.clear();
		m_hashUri2SubscribeResourceTaskId.clear();
		m_loggingLevel = MCP::LoggingLevel{};

		{
			std::lock_guard<std::mutex> _lock(m_mtxSubscriptions);
			m_hashSubscriptions.clear();
		}

		// Re-arm the async task flag LAST so that CommitAsyncTask/CancelAsyncTask can
		// never observe m_bRunAsyncTask == true while m_upTaskThread is still null
		// (which would silently drop tasks into a deque that no worker consumes).
		m_bRunAsyncTask.store(true);

		return ERRNO_OK;
	}

	int CMCPSession::ProcessMessage(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg)
	{
		if (!spMsg || !spMsg->IsValid())
			return ERRNO_INTERNAL_ERROR;

		const ProtocolContext& ctxProtocol = MessageContextAccess::Get(*spMsg);

		// Messages are split by era before the legacy switch runs. A request that
		// carries modern _meta but cannot resolve a profile must still produce a
		// JSON-RPC error, so it also enters the modern path.
		if (MessageCategory_Request == spMsg->eMessageCategory)
		{
			if (!ctxProtocol.IsValid() || ctxProtocol.IsStateless())
				return ProcessModernMessage(iErrCode, spMsg);
		}
		else if (MessageCategory_Notification == spMsg->eMessageCategory)
		{
			if (!ctxProtocol.IsValid())
				return ERRNO_OK;

			if (ctxProtocol.IsStateless())
			{
				return ProcessModernNotification(
					MessageContextAccess::GetShared(*spMsg),
					std::dynamic_pointer_cast<MCP::Notification>(spMsg));
			}
		}

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

		// The handshake switch below serves every known handshake method. A
		// profile that does not declare the method must not be served, so that
		// an older or newer handshake revision stays inside its own surface.
		// Unknown methods keep the historical behaviour handled by the switch
		// default branch.
		if (IsLegacyDispatchableMessageType(spRequest->eMessageType)
			&& !MessageContextAccess::Get(*spRequest).SupportsMethod(spRequest->strMethod))
		{
			strMessage = ERROR_MESSAGE_METHOD_NOT_FOUND;
			iErrCode = ERRNO_METHOD_NOT_FOUND;
			goto PROC_END;
		}

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

			auto spTask = MakeProcessTask<ProcessInitializeRequest>(spRequest);
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

			auto spTask = MakeProcessTask<ProcessListToolsRequest>(spRequest);
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
			BindTaskServices(spNewProcessCallToolRequest);
			iErrCode = CommitAsyncTask(spNewProcessCallToolRequest);

		} break;
		case MessageType_PingRequest:
		{
			if (CMCPSession::SessionState_Initialized != CMCPSession::GetInstance().GetSessionState())
			{
				iErrCode = ERRNO_INVALID_REQUEST;
				goto PROC_END;
			}

			auto spTask = MakeProcessTask<ProcessPingRequest>(spRequest);
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

			auto spTask = MakeProcessTask<ProcessListResourcesRequest>(spRequest);
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
			BindTaskServices(spNewProcessReadResourceRequest);
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
			BindTaskServices(spNewProcessSubscribeResourceRequest);
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
			auto spTask = MakeProcessTask<ProcessUnsubscribeResourceRequest>(spRequest);
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

			auto spTask = MakeProcessTask<ProcessListResourceTemplatesRequest>(spRequest);
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

			auto spTask = MakeProcessTask<ProcessListPromptsRequest>(spRequest);
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
			BindTaskServices(spNewProcessGetPromptRequest);
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
			BindTaskServices(spNewProcessCompleteRequest);
			iErrCode = CommitAsyncTask(spNewProcessCompleteRequest);

		} break;
		case MessageType_SetLevelRequest:
		{
			if (CMCPSession::SessionState_Initialized != CMCPSession::GetInstance().GetSessionState())
			{
				iErrCode = ERRNO_INVALID_REQUEST;
				goto PROC_END;
			}

			auto spTask = MakeProcessTask<ProcessSetLevelRequest>(spRequest);
			if (!spTask)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				goto PROC_END;
			}

			iErrCode = spTask->Execute();

		} break;
		default:
		{
			// A handshake client probing the stateless discovery method receives the
			// standard method-not-found error instead of a silent drop.
			if (spRequest->strMethod.compare(METHOD_SERVER_DISCOVER) == 0)
			{
				strMessage = ERROR_MESSAGE_METHOD_NOT_FOUND;
				iErrCode = ERRNO_METHOD_NOT_FOUND;
			}

		} break;
		}

	PROC_END:
		if (ERRNO_OK != iErrCode)
		{
			auto spTask = MakeProcessTask<ProcessErrorRequest>(spRequest);
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

		std::string strMethod;
		if (jVal.isMember(MSG_KEY_METHOD) && jVal[MSG_KEY_METHOD].isString())
			strMethod = jVal[MSG_KEY_METHOD].asString();

		Json::Value jMeta(Json::objectValue);
		if (jVal.isMember(MSG_KEY_PARAMS) && jVal[MSG_KEY_PARAMS].isObject()
			&& jVal[MSG_KEY_PARAMS].isMember(MSG_KEY_META))
		{
			jMeta = jVal[MSG_KEY_PARAMS][MSG_KEY_META];
		}

		std::string strRequestedVersion;
		if (METHOD_INITIALIZE == strMethod
			&& jVal.isMember(MSG_KEY_PARAMS) && jVal[MSG_KEY_PARAMS].isObject()
			&& jVal[MSG_KEY_PARAMS].isMember(MSG_KEY_PROTOCOL_VERSION)
			&& jVal[MSG_KEY_PARAMS][MSG_KEY_PROTOCOL_VERSION].isString())
		{
			strRequestedVersion = jVal[MSG_KEY_PARAMS][MSG_KEY_PROTOCOL_VERSION].asString();
		}

		// The era must be resolved from the raw message before the typed parse so
		// version gated fields such as params._meta.logLevel survive deserialization.
		std::shared_ptr<const ProtocolContext> spProtocolContext = ResolveProtocolContext(strMethod, jMeta, strRequestedVersion);

		switch (eCategory)
		{
			case MessageCategory_Request:
			{
				return ParseRequest(strMsg, spProtocolContext, spMsg);
			} break;
			case MessageCategory_Response:
			{
				return ParseResponse(strMsg, spMsg);
			} break;
			case MessageCategory_Notification:
			{
				return ParseNotification(strMsg, spProtocolContext, spMsg);
			} break;
			default: break;
		}

		return ERRNO_INTERNAL_ERROR;
	}

	int CMCPSession::ParseRequest(const std::string& strMsg, const std::shared_ptr<const ProtocolContext>& spProtocolContext, std::shared_ptr<MCP::Message>& spMsg)
	{
		const ProtocolContext& ctxProtocol = *spProtocolContext;

		auto spRequest = std::make_shared<MCP::Request>(MessageType_Unknown, false);
		if (!spRequest)
			return ERRNO_PARSE_ERROR;

		int iErrCode = ERRNO_OK;
		MessageContextAccess::Set(*spRequest, spProtocolContext);
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

			MessageContextAccess::Set(*spInitializeRequest, spProtocolContext);
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

			MessageContextAccess::Set(*spListToolsRequest, spProtocolContext);
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

			MessageContextAccess::Set(*spCallToolRequest, spProtocolContext);
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

			MessageContextAccess::Set(*spPingRequest, spProtocolContext);
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

			MessageContextAccess::Set(*spListResourcesRequest, spProtocolContext);
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

			MessageContextAccess::Set(*spReadResourceRequest, spProtocolContext);
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

			MessageContextAccess::Set(*spSubscribeResourceRequest, spProtocolContext);
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

			MessageContextAccess::Set(*spUnsubscribeResourceRequest, spProtocolContext);
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

			MessageContextAccess::Set(*spListResourceTemplatesRequest, spProtocolContext);
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

			MessageContextAccess::Set(*spListPromptsRequest, spProtocolContext);
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

			MessageContextAccess::Set(*spGetPromptRequest, spProtocolContext);
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

			MessageContextAccess::Set(*spCompleteRequest, spProtocolContext);
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

			MessageContextAccess::Set(*spSetLevelRequest, spProtocolContext);
			iErrCode = spSetLevelRequest->Deserialize(strMsg);
			if (ERRNO_OK != iErrCode)
				return ERRNO_INVALID_REQUEST;

			spMsg = spSetLevelRequest;

			return ERRNO_OK;
		}
		else if (spRequest->strMethod.compare(METHOD_SERVER_DISCOVER) == 0)
		{
			// Discovery only exists in the stateless era. A handshake client probing
			// it keeps the legacy path and receives method-not-found.
			if (ctxProtocol.Supports(Feature_Discover))
			{
				auto spDiscoverRequest = std::make_shared<MCP::DiscoverRequest>(true);
				if (!spDiscoverRequest)
					return ERRNO_PARSE_ERROR;

				MessageContextAccess::Set(*spDiscoverRequest, spProtocolContext);
				iErrCode = spDiscoverRequest->Deserialize(strMsg);
				if (ERRNO_OK != iErrCode)
					return ERRNO_INVALID_REQUEST;

				spMsg = spDiscoverRequest;

				return ERRNO_OK;
			}
		}
		else if (spRequest->strMethod.compare(METHOD_SUBSCRIPTIONS_LISTEN) == 0)
		{
			if (!ctxProtocol.Supports(Feature_ListenSubscriptions))
				return ERRNO_INTERNAL_ERROR;

			auto spListenRequest = std::make_shared<MCP::SubscriptionsListenRequest>(true);
			if (!spListenRequest)
				return ERRNO_PARSE_ERROR;

			MessageContextAccess::Set(*spListenRequest, spProtocolContext);
			iErrCode = spListenRequest->Deserialize(strMsg);
			if (ERRNO_OK != iErrCode)
				return ERRNO_INVALID_REQUEST;

			spMsg = spListenRequest;

			return ERRNO_OK;
		}

		// An unresolved or stateless request with an unknown method must still
		// produce a JSON-RPC error, so the generic request is handed back. The
		// handshake discover probe follows the same path to receive -32601.
		if (ctxProtocol.IsStateless() || !ctxProtocol.IsValid()
			|| METHOD_SERVER_DISCOVER == spRequest->strMethod)
		{
			spMsg = spRequest;

			return ERRNO_OK;
		}

		return ERRNO_INTERNAL_ERROR;
	}

	int CMCPSession::ParseResponse(const std::string& strMsg, std::shared_ptr<MCP::Message>& spMsg)
	{
		return ERRNO_INTERNAL_ERROR;
	}

	int CMCPSession::ParseNotification(const std::string& strMsg, const std::shared_ptr<const ProtocolContext>& spProtocolContext, std::shared_ptr<MCP::Message>& spMsg)
	{
		auto spNotification = std::make_shared<MCP::Notification>(MessageType_Unknown, false);
		if (!spNotification)
			return ERRNO_PARSE_ERROR;

		int iErrCode = ERRNO_OK;
		MessageContextAccess::Set(*spNotification, spProtocolContext);
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

			MessageContextAccess::Set(*spInitializedNotification, spProtocolContext);
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

			MessageContextAccess::Set(*spCancelledNotification, spProtocolContext);
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

		// Transports that carry several clients over one channel report the gap
		// between them. Binding the reset here keeps that knowledge in the session
		// instead of making the transport call back into the singleton.
		if (m_spTransport)
			m_spTransport->SetOnClientDisconnected([this]() { ResetForNewClient(); });
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

	size_t CMCPSession::GetMessageCount(MessageCategory eCategory) const
	{
		auto it = m_hashMessage.find(eCategory);
		if (it == m_hashMessage.end())
			return 0;

		return it->second.size();
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
		auto currentLevel = GetLoggingLevel();
		if (level.eLevel < currentLevel.eLevel)
		{
			return ERRNO_OK;
		}

		return WriteLogNotification(level, strLogger, jData);
	}

	int CMCPSession::WriteLogNotification(const MCP::LoggingLevel& level, const std::string& strLogger, const Json::Value& jData)
	{
		if (!m_spTransport)
			return ERRNO_INTERNAL_ERROR;

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

	int CMCPSession::SetSupportedProtocolVersions(const std::vector<std::string>& vecVersions)
	{
		if (!m_spTransport)
			return ERRNO_INTERNAL_ERROR;

		const CProtocolRegistry& registry = CProtocolRegistry::GetGlobalRegistry();
		const bool bStatelessSupported = m_spTransport->SupportsStatelessProtocol();
		const bool bExplicit = !vecVersions.empty();

		// The declaration is a set: callers may repeat a version, but it must only
		// contribute one profile to the registry.
		std::vector<std::string> vecEffective;
		for (const auto& strVersion : (bExplicit ? vecVersions : registry.GetSupportedVersions()))
		{
			if (std::find(vecEffective.begin(), vecEffective.end(), strVersion) == vecEffective.end())
				vecEffective.push_back(strVersion);
		}

		std::vector<std::shared_ptr<const ProtocolProfile>> vecProfiles;
		for (const auto& strVersion : vecEffective)
		{
			std::shared_ptr<const ProtocolProfile> pProfile = registry.Find(strVersion);
			if (!pProfile)
				return ERRNO_INVALID_PARAMS;

			// An explicitly requested stateless revision over a handshake-only
			// transport is a configuration error; the implicit default is filtered
			// instead so an SSE server keeps working without extra configuration.
			if (!bStatelessSupported && Era_Stateless == pProfile->eEra)
			{
				if (bExplicit)
					return ERRNO_INVALID_PARAMS;

				continue;
			}

			// Reuse the registry-owned profile instead of copying it, so contexts
			// handed out earlier keep a valid profile after this registry is reset.
			vecProfiles.push_back(pProfile);
		}

		if (vecProfiles.empty())
			return ERRNO_INVALID_PARAMS;

		m_upRegistry.reset(new CProtocolRegistry(vecProfiles));
		return ERRNO_OK;
	}

	void CMCPSession::SetProtocolRegistry(const CProtocolRegistry& registry)
	{
		m_upRegistry.reset(new CProtocolRegistry(registry));
	}

	const CProtocolRegistry& CMCPSession::GetProtocolRegistry() const
	{
		return *m_upRegistry;
	}

	void CMCPSession::SetServerCacheOptions(int iTtlMs, const std::string& strCacheScope)
	{
		m_iCacheTtlMs = iTtlMs;
		m_strCacheScope = strCacheScope.empty() ? CACHE_SCOPE_PRIVATE : strCacheScope;
	}

	int CMCPSession::GetServerCacheTtlMs() const
	{
		return m_iCacheTtlMs;
	}

	std::string CMCPSession::GetServerCacheScope() const
	{
		return m_strCacheScope;
	}

	void CMCPSession::SetServerInstructions(const std::string& strInstructions)
	{
		m_strServerInstructions = strInstructions;
	}

	std::string CMCPSession::GetServerInstructions() const
	{
		return m_strServerInstructions;
	}

	std::shared_ptr<const ProtocolContext> CMCPSession::ResolveProtocolContext(const std::string& strMethod, const Json::Value& jMeta,
		const std::string& strRequestedVersion) const
	{
		const CProtocolRegistry& registry = GetProtocolRegistry();

		if (METHOD_INITIALIZE == strMethod)
		{
			// Handshake negotiation: honor an explicitly requested version when the
			// registry serves it in the handshake era. A version the registry does
			// not know falls back to the default handshake profile, which is the
			// version the server reports back to the client.
			std::shared_ptr<const ProtocolProfile> pRequested = registry.Find(strRequestedVersion);
			if (pRequested && Era_Handshake == pRequested->eEra)
				return MakeProfileContext(pRequested);

			return MakeProfileContext(registry.GetDefaultHandshakeProfile());
		}

		// No io.modelcontextprotocol/* key keeps the legacy handshake flow. Only
		// progressToken/toolCallId style metadata stays in that era.
		if (!HasModernMetaKeys(jMeta))
			return MakeProfileContext(registry.GetDefaultHandshakeProfile());

		std::string strVersion;
		if (jMeta.isMember(META_KEY_PROTOCOL_VERSION) && jMeta[META_KEY_PROTOCOL_VERSION].isString())
			strVersion = jMeta[META_KEY_PROTOCOL_VERSION].asString();

		if (strVersion.empty())
			return std::make_shared<const ProtocolContext>(ProtocolContext::Invalid(strVersion));

		std::shared_ptr<const ProtocolProfile> pProfile = registry.Find(strVersion);
		if (!pProfile)
			return std::make_shared<const ProtocolContext>(ProtocolContext::Invalid(strVersion));

		return MakeProfileContext(pProfile);
	}

	std::shared_ptr<const ProtocolContext> CMCPSession::ResolveProtocolContext(const std::shared_ptr<MCP::Message>& spMsg) const
	{
		auto spRequest = std::dynamic_pointer_cast<MCP::Request>(spMsg);
		if (!spRequest)
			return MakeProfileContext(GetProtocolRegistry().GetDefaultHandshakeProfile());

		Json::Value jMeta(Json::objectValue);
		spRequest->meta.DoSerialize(jMeta);

		std::string strRequestedVersion;
		auto spInitializeRequest = std::dynamic_pointer_cast<MCP::InitializeRequest>(spRequest);
		if (spInitializeRequest)
			strRequestedVersion = spInitializeRequest->strProtocolVer;

		return ResolveProtocolContext(spRequest->strMethod, jMeta, strRequestedVersion);
	}

	int CMCPSession::ValidateProtocolContext(const std::shared_ptr<const ProtocolContext>& spProtocolContext, const std::shared_ptr<MCP::Request>& spRequest)
	{
		if (!spRequest)
			return ERRNO_INVALID_REQUEST;

		const ProtocolContext& ctxProtocol = *spProtocolContext;
		if (!ctxProtocol.IsValid())
		{
			// A missing version must not silently fall back to a default.
			if (spRequest->meta.strProtocolVersion.empty())
				return ERRNO_INVALID_PARAMS;

			return ERRNO_UNSUPPORTED_PROTOCOL_VERSION;
		}

		if (!ctxProtocol.IsStateless())
			return ERRNO_OK;

		if (spRequest->meta.strProtocolVersion.empty() || !spRequest->meta.bHasClientCapabilities)
			return ERRNO_INVALID_PARAMS;

		return ERRNO_OK;
	}

	void CMCPSession::PrepareResult(const std::shared_ptr<MCP::Request>& spRequest, const std::shared_ptr<MCP::Response>& spResponse)
	{
		if (!spRequest || !spResponse)
			return;

		MessageContextAccess::Copy(*spResponse, *spRequest);
		spResponse->bMrtrRetry = spRequest->jInputResponses.isObject() || !spRequest->strRequestState.empty();

		if (spResponse->IsCacheableResult())
		{
			if (spResponse->iTtlMs < 0)
				spResponse->iTtlMs = m_iCacheTtlMs;

			if (spResponse->strCacheScope.empty())
				spResponse->strCacheScope = m_strCacheScope;
		}

		Json::Value jServerInfo(Json::objectValue);
		m_serverInfo.DoSerialize(jServerInfo);
		VersionExtrasAccess::Serialize(m_serverInfo, jServerInfo, MessageContextAccess::Get(*spResponse));
		spResponse->jServerInfo = jServerInfo;
	}

	int CMCPSession::SendErrorResponse(const std::shared_ptr<MCP::Request>& spRequest, int iErrCode, const std::string& strMessage, const Json::Value& jErrorData)
	{
		if (!spRequest)
			return ERRNO_INTERNAL_ERROR;

		auto spTask = MakeProcessTask<ProcessErrorRequest>(spRequest);
		if (!spTask)
			return ERRNO_INTERNAL_ERROR;

		spTask->SetErrorCode(iErrCode);
		spTask->SetErrorMessage(strMessage);
		spTask->SetErrorData(jErrorData);

		return spTask->Execute();
	}

	int CMCPSession::ProcessModernMessage(int iErrCode, const std::shared_ptr<MCP::Message>& spMsg)
	{
		auto spRequest = std::dynamic_pointer_cast<MCP::Request>(spMsg);
		if (!spRequest)
			return ERRNO_INTERNAL_ERROR;

		int iValidate = ValidateProtocolContext(MessageContextAccess::GetShared(*spRequest), spRequest);
		if (ERRNO_OK != iValidate)
		{
			Json::Value jErrorData;
			std::string strMessage = ERROR_MESSAGE_INVALID_PARAMS;
			if (ERRNO_UNSUPPORTED_PROTOCOL_VERSION == iValidate)
			{
				Json::Value jSupported(Json::arrayValue);
				std::vector<std::string> vecVersions = GetProtocolRegistry().GetSupportedVersions();
				for (auto& strVersion : vecVersions)
					jSupported.append(strVersion);

				jErrorData[MSG_KEY_SUPPORTED] = jSupported;
				jErrorData[MSG_KEY_REQUESTED] = spRequest->meta.strProtocolVersion;
				strMessage = ERROR_MESSAGE_UNSUPPORTED_PROTOCOL_VERSION;
			}

			return SendErrorResponse(spRequest, iValidate, strMessage, jErrorData);
		}

		if (ERRNO_OK != iErrCode)
			return iErrCode;

		return ProcessModernRequest(MessageContextAccess::GetShared(*spRequest), spRequest);
	}

	int CMCPSession::ProcessModernRequest(const std::shared_ptr<const ProtocolContext>& spProtocolContext, const std::shared_ptr<MCP::Request>& spRequest)
	{
		if (!spRequest)
			return ERRNO_INTERNAL_ERROR;

		std::shared_ptr<const ProtocolProfile> pProfile = spProtocolContext->pProfile;
		if (!pProfile || !pProfile->SupportsMethod(spRequest->strMethod))
			return SendErrorResponse(spRequest, ERRNO_METHOD_NOT_FOUND, ERROR_MESSAGE_METHOD_NOT_FOUND);

		EnsureAsyncTaskThread();

		auto DispatchAsyncTask = [this](const std::shared_ptr<MCP::ProcessRequest>& spPrototype,
			const std::shared_ptr<MCP::Request>& spRequest) -> int
		{
			if (!spPrototype)
				return ERRNO_INTERNAL_ERROR;

			std::shared_ptr<MCP::CMCPTask> spNewTask = spPrototype->Clone();
			if (!spNewTask)
				return ERRNO_INTERNAL_ERROR;

			auto spNewProcessTask = std::dynamic_pointer_cast<MCP::ProcessRequest>(spNewTask);
			if (!spNewProcessTask)
				return ERRNO_INTERNAL_ERROR;

			spNewProcessTask->SetRequest(spRequest);
			BindTaskServices(spNewProcessTask);

			return CommitAsyncTask(spNewProcessTask);
		};

		switch (spRequest->eMessageType)
		{
			case MessageType_DiscoverRequest:
			{
				auto spTask = MakeProcessTask<ProcessDiscoverRequest>(spRequest);
				if (!spTask)
					return ERRNO_INTERNAL_ERROR;

				return spTask->Execute();
			} break;
			case MessageType_SubscriptionsListenRequest:
			{
				return ProcessSubscriptionsListenRequest(spProtocolContext, spRequest);
			} break;
			case MessageType_ListToolsRequest:
			case MessageType_ListResourcesRequest:
			case MessageType_ListResourceTemplatesRequest:
			case MessageType_ListPromptsRequest:
			{
				std::shared_ptr<MCP::ProcessRequest> spTask;
				if (MessageType_ListToolsRequest == spRequest->eMessageType)
					spTask = MakeProcessTask<ProcessListToolsRequest>(spRequest);
				else if (MessageType_ListResourcesRequest == spRequest->eMessageType)
					spTask = MakeProcessTask<ProcessListResourcesRequest>(spRequest);
				else if (MessageType_ListResourceTemplatesRequest == spRequest->eMessageType)
					spTask = MakeProcessTask<ProcessListResourceTemplatesRequest>(spRequest);
				else
					spTask = MakeProcessTask<ProcessListPromptsRequest>(spRequest);

				if (!spTask)
					return ERRNO_INTERNAL_ERROR;

				return spTask->Execute();
			} break;
			case MessageType_CallToolRequest:
			{
				auto spCallToolRequest = std::dynamic_pointer_cast<MCP::CallToolRequest>(spRequest);
				if (!spCallToolRequest)
					return ERRNO_INTERNAL_ERROR;

				auto spPrototype = GetServerCallToolsTask(spCallToolRequest->strName);
				if (!spPrototype)
					return SendErrorResponse(spRequest, ERRNO_INVALID_PARAMS, ERROR_MESSAGE_INVALID_PARAMS);

				return DispatchAsyncTask(spPrototype, spRequest);
			} break;
			case MessageType_ReadResourceRequest:
			{
				auto spReadResourceRequest = std::dynamic_pointer_cast<MCP::ReadResourceRequest>(spRequest);
				if (!spReadResourceRequest)
					return ERRNO_INTERNAL_ERROR;

				auto spPrototype = GetServerReadResourceTask(spReadResourceRequest->strUri);
				if (!spPrototype)
				{
					int iNotFoundCode = pProfile->iResourceNotFoundCode;
					return SendErrorResponse(spRequest, iNotFoundCode, ERROR_MESSAGE_INVALID_PARAMS);
				}

				return DispatchAsyncTask(spPrototype, spRequest);
			} break;
			case MessageType_GetPromptRequest:
			{
				auto spGetPromptRequest = std::dynamic_pointer_cast<MCP::GetPromptRequest>(spRequest);
				if (!spGetPromptRequest)
					return ERRNO_INTERNAL_ERROR;

				auto spPrototype = GetServerGetPromptTask(spGetPromptRequest->strName);
				if (!spPrototype)
					return SendErrorResponse(spRequest, ERRNO_INVALID_PARAMS, ERROR_MESSAGE_INVALID_PARAMS);

				return DispatchAsyncTask(spPrototype, spRequest);
			} break;
			case MessageType_CompleteRequest:
			{
				auto spCompleteRequest = std::dynamic_pointer_cast<MCP::CompleteRequest>(spRequest);
				if (!spCompleteRequest)
					return ERRNO_INTERNAL_ERROR;

				std::string strTaskKey = spCompleteRequest->strRefType + ":";
				if (CONST_REF_PROMPT == spCompleteRequest->strRefType)
					strTaskKey += spCompleteRequest->promptRef.strName;
				else if (CONST_REF_RESOURCE == spCompleteRequest->strRefType)
					strTaskKey += spCompleteRequest->resourceRef.strUri;

				auto spPrototype = strTaskKey.empty() ? std::shared_ptr<MCP::ProcessRequest>() : GetServerCompleteTask(strTaskKey);
				if (!spPrototype)
					return SendErrorResponse(spRequest, ERRNO_INVALID_PARAMS, ERROR_MESSAGE_INVALID_PARAMS);

				return DispatchAsyncTask(spPrototype, spRequest);
			} break;
			default:
				break;
		}

		return SendErrorResponse(spRequest, ERRNO_METHOD_NOT_FOUND, ERROR_MESSAGE_METHOD_NOT_FOUND);
	}

	int CMCPSession::ProcessModernNotification(const std::shared_ptr<const ProtocolContext>&, const std::shared_ptr<MCP::Notification>& spNotification)
	{
		if (!spNotification)
			return ERRNO_OK;

		if (MessageType_CancelledNotification == spNotification->eMessageType)
		{
			auto spCancelledNotification = std::dynamic_pointer_cast<MCP::CancelledNotification>(spNotification);
			if (spCancelledNotification && spCancelledNotification->IsValid())
			{
				RemoveSubscription(spCancelledNotification->requestId);

				return CancelAsyncTask(spCancelledNotification->requestId);
			}
		}

		// Other stateless notifications carry no session state.
		return ERRNO_OK;
	}

	int CMCPSession::EnsureAsyncTaskThread()
	{
		if (m_upTaskThread)
			return ERRNO_OK;

		if (!m_bRunAsyncTask.load())
			m_bRunAsyncTask.store(true);

		return StartAsyncTaskThread();
	}

	int CMCPSession::SendToolListChanged()
	{
		return SendListChangedNotification(MessageType_ToolListChangedNotification, METHOD_NOTIFICATION_TOOLS_LIST_CHANGED);
	}

	int CMCPSession::SendPromptListChanged()
	{
		return SendListChangedNotification(MessageType_PromptListChangedNotification, METHOD_NOTIFICATION_PROMPTS_LIST_CHANGED);
	}

	int CMCPSession::SendResourceListChanged()
	{
		return SendListChangedNotification(MessageType_ResourceListChangedNotification, METHOD_NOTIFICATION_RESOURCES_LIST_CHANGED);
	}

	int CMCPSession::SendListChangedNotification(MessageType eMessageType, const std::string& strMethod)
	{
		if (!m_spTransport)
			return ERRNO_INTERNAL_ERROR;

		bool bHasSubscriptions = false;
		{
			std::lock_guard<std::mutex> _lock(m_mtxSubscriptions);
			bHasSubscriptions = !m_hashSubscriptions.empty();
		}

		if (bHasSubscriptions)
			return SendSubscriptionNotification(strMethod, Json::Value());

		std::shared_ptr<MCP::Notification> spNotification;
		if (MessageType_ToolListChangedNotification == eMessageType)
			spNotification = std::make_shared<ToolListChangedNotification>(false);
		else if (MessageType_PromptListChangedNotification == eMessageType)
			spNotification = std::make_shared<PromptListChangedNotification>(false);
		else
			spNotification = std::make_shared<ResourceListChangedNotification>(false);

		if (!spNotification)
			return ERRNO_INTERNAL_ERROR;

		spNotification->strMethod = strMethod;

		std::string strNotification;
		if (ERRNO_OK != spNotification->Serialize(strNotification))
			return ERRNO_INTERNAL_ERROR;

		return m_spTransport->Write(strNotification);
	}

	int CMCPSession::SendSubscriptionNotification(const std::string& strMethod, const Json::Value& jParams, const std::string& strMatchUri)
	{
		if (!m_spTransport)
			return ERRNO_INTERNAL_ERROR;

		std::vector<SubscriptionState> vecTargets;
		{
			std::lock_guard<std::mutex> _lock(m_mtxSubscriptions);
			for (auto& pair : m_hashSubscriptions)
			{
				const SubscriptionState& subscription = pair.second;
				if (!subscription.bActive)
					continue;

				bool bMatch = false;
				if (METHOD_NOTIFICATION_TOOLS_LIST_CHANGED == strMethod)
					bMatch = subscription.bToolsListChanged;
				else if (METHOD_NOTIFICATION_PROMPTS_LIST_CHANGED == strMethod)
					bMatch = subscription.bPromptsListChanged;
				else if (METHOD_NOTIFICATION_RESOURCES_LIST_CHANGED == strMethod)
					bMatch = subscription.bResourcesListChanged;
				else if (METHOD_NOTIFICATION_RESOURCES_UPDATED == strMethod)
					bMatch = subscription.setResourceSubscriptions.find(strMatchUri) != subscription.setResourceSubscriptions.end();

				if (bMatch)
					vecTargets.push_back(subscription);
			}
		}

		int iErrCode = ERRNO_OK;
		for (auto& subscription : vecTargets)
		{
			std::shared_ptr<MCP::Notification> spNotification;
			if (METHOD_NOTIFICATION_RESOURCES_UPDATED == strMethod)
			{
				auto spUpdatedNotification = std::make_shared<ResourceUpdatedNotification>(false);
				if (jParams.isObject() && jParams.isMember(MSG_KEY_URI))
					spUpdatedNotification->strUri = jParams[MSG_KEY_URI].asString();

				spNotification = spUpdatedNotification;
			}
			else if (METHOD_NOTIFICATION_TOOLS_LIST_CHANGED == strMethod)
				spNotification = std::make_shared<ToolListChangedNotification>(false);
			else if (METHOD_NOTIFICATION_PROMPTS_LIST_CHANGED == strMethod)
				spNotification = std::make_shared<PromptListChangedNotification>(false);
			else if (METHOD_NOTIFICATION_RESOURCES_LIST_CHANGED == strMethod)
				spNotification = std::make_shared<ResourceListChangedNotification>(false);

			if (!spNotification)
				continue;

			spNotification->strMethod = strMethod;
			if (subscription.spProtocolContext)
				MessageContextAccess::Set(*spNotification, subscription.spProtocolContext);
			spNotification->bHasSubscriptionId = true;
			spNotification->jSubscriptionId = MakeRequestIdJson(subscription.requestId);

			std::string strNotification;
			if (ERRNO_OK != spNotification->Serialize(strNotification))
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				continue;
			}

			if (ERRNO_OK != m_spTransport->Write(strNotification))
				iErrCode = ERRNO_INTERNAL_ERROR;
		}

		return iErrCode;
	}

	int CMCPSession::RemoveSubscription(const MCP::RequestId& requestId)
	{
		std::lock_guard<std::mutex> _lock(m_mtxSubscriptions);
		for (auto it = m_hashSubscriptions.begin(); it != m_hashSubscriptions.end(); ++it)
		{
			if (it->second.requestId.IsEqual(requestId))
			{
				m_hashSubscriptions.erase(it);
				break;
			}
		}

		return ERRNO_OK;
	}

	int CMCPSession::CloseSubscriptions()
	{
		std::vector<SubscriptionState> vecStates;
		{
			std::lock_guard<std::mutex> _lock(m_mtxSubscriptions);
			for (auto& pair : m_hashSubscriptions)
				vecStates.push_back(pair.second);

			m_hashSubscriptions.clear();
		}

		if (!m_spTransport)
			return ERRNO_OK;

		int iErrCode = ERRNO_OK;
		for (auto& subscription : vecStates)
		{
			if (!subscription.bActive || !subscription.spListenRequest)
				continue;

			auto spResult = std::make_shared<SubscriptionsListenResult>(true);
			if (!spResult)
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				continue;
			}

			spResult->requestId = subscription.requestId;
			spResult->jSubscriptionId = MakeRequestIdJson(subscription.requestId);
			PrepareResult(subscription.spListenRequest, spResult);

			std::string strResult;
			if (ERRNO_OK != spResult->Serialize(strResult))
			{
				iErrCode = ERRNO_INTERNAL_ERROR;
				continue;
			}

			if (ERRNO_OK != m_spTransport->Write(strResult))
				iErrCode = ERRNO_INTERNAL_ERROR;
		}

		return iErrCode;
	}

	int CMCPSession::ProcessSubscriptionsListenRequest(const std::shared_ptr<const ProtocolContext>& spProtocolContext, const std::shared_ptr<MCP::Request>& spRequest)
	{
		if (!spRequest)
			return ERRNO_INTERNAL_ERROR;

		const ProtocolContext& ctxProtocol = *spProtocolContext;
		auto spListenRequest = std::dynamic_pointer_cast<MCP::SubscriptionsListenRequest>(spRequest);
		if (!spListenRequest || !spListenRequest->IsValid() || !spListenRequest->bHasNotifications)
			return SendErrorResponse(spRequest, ERRNO_INVALID_PARAMS, ERROR_MESSAGE_INVALID_PARAMS);

		if (!ctxProtocol.Supports(Feature_ListenSubscriptions))
			return SendErrorResponse(spRequest, ERRNO_METHOD_NOT_FOUND, ERROR_MESSAGE_METHOD_NOT_FOUND);

		MCP::ServerCapabilities capabilities = GetServerCapabilities();

		SubscriptionState subscription;
		subscription.requestId = spListenRequest->requestId;
		subscription.spProtocolContext = spProtocolContext;
		subscription.spListenRequest = spRequest;

		// Only grant the notification types the server can actually publish.
		subscription.bToolsListChanged = spListenRequest->bToolsListChanged && capabilities.tools.bExist;
		subscription.bPromptsListChanged = spListenRequest->bPromptsListChanged && capabilities.prompts.bExist;
		subscription.bResourcesListChanged = spListenRequest->bResourcesListChanged && capabilities.resources.bExist;
		if (capabilities.resources.bExist)
		{
			for (const auto& strUri : spListenRequest->vecResourceSubscriptions)
				subscription.setResourceSubscriptions.insert(strUri);
		}

		Json::Value jGranted(Json::objectValue);
		if (subscription.bToolsListChanged)
			jGranted[MSG_KEY_TOOLS_LIST_CHANGED] = true;
		if (subscription.bPromptsListChanged)
			jGranted[MSG_KEY_PROMPTS_LIST_CHANGED] = true;
		if (subscription.bResourcesListChanged)
			jGranted[MSG_KEY_RESOURCES_LIST_CHANGED] = true;
		if (!subscription.setResourceSubscriptions.empty())
		{
			Json::Value jUris(Json::arrayValue);
			for (const auto& strUri : subscription.setResourceSubscriptions)
				jUris.append(strUri);
			jGranted[MSG_KEY_RESOURCE_SUBSCRIPTIONS] = jUris;
		}

		// Register the subscription as inactive until the acknowledgment has been
		// written, so an event racing with the ack cannot be delivered before it.
		const std::string strKey = MakeRequestIdKey(spListenRequest->requestId);
		{
			std::lock_guard<std::mutex> _lock(m_mtxSubscriptions);
			m_hashSubscriptions[strKey] = subscription;
		}

		auto spAcknowledged = std::make_shared<SubscriptionsAcknowledgedNotification>(false);
		if (!spAcknowledged)
		{
			RemoveSubscription(spListenRequest->requestId);
			return ERRNO_INTERNAL_ERROR;
		}

		spAcknowledged->strMethod = METHOD_NOTIFICATION_SUBSCRIPTIONS_ACKNOWLEDGED;
		MessageContextAccess::Set(*spAcknowledged, spProtocolContext);
		spAcknowledged->bHasSubscriptionId = true;
		spAcknowledged->jSubscriptionId = MakeRequestIdJson(spListenRequest->requestId);
		spAcknowledged->jNotifications = jGranted;

		std::string strAcknowledged;
		if (ERRNO_OK != spAcknowledged->Serialize(strAcknowledged))
		{
			RemoveSubscription(spListenRequest->requestId);
			return ERRNO_INTERNAL_ERROR;
		}

		if (!m_spTransport || ERRNO_OK != m_spTransport->Write(strAcknowledged))
		{
			RemoveSubscription(spListenRequest->requestId);
			return ERRNO_INTERNAL_ERROR;
		}

		{
			std::lock_guard<std::mutex> _lock(m_mtxSubscriptions);
			auto it = m_hashSubscriptions.find(strKey);
			if (it != m_hashSubscriptions.end())
				it->second.bActive = true;
		}

		return ERRNO_OK;
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
		// Stateless requests may spawn the worker before the legacy handshake
		// completes. Reuse it instead of replacing a joinable thread, which would
		// call std::terminate through the thread move assignment.
		if (m_upTaskThread && m_upTaskThread->joinable())
			return ERRNO_OK;

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

			// A cancellation that arrives before the worker starts a queued task
			// must drop it: the task has not observed the request yet, so no
			// response may be produced for the abandoned request.
			vecTasks.erase(
				std::remove_if(vecTasks.begin(), vecTasks.end(), [this](auto& spTask)
					{
						auto spProcessRequestTask = std::dynamic_pointer_cast<MCP::ProcessRequest>(spTask);
						if (!spProcessRequestTask)
							return false;

						auto spRequest = spProcessRequestTask->GetRequest();
						if (!spRequest)
							return false;

						auto itrFound = std::find_if(m_vecCancelledTaskIds.begin(), m_vecCancelledTaskIds.end(), [&spRequest](auto& requestId)
							{
								return requestId.IsEqual(spRequest->requestId);
							});

						return itrFound != m_vecCancelledTaskIds.end();
					}),
				vecTasks.end());

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
					int iExecuteErrCode = spTask->Execute();
					if (ERRNO_OK == iExecuteErrCode)
					{
						m_vecAsyncTasksCache.push_back(spTask);
						continue;
					}

					// A stateless task that rejected its input without emitting a
					// result must still answer, otherwise the client would wait
					// forever on a request that can never complete.
					auto spProcessTask = std::dynamic_pointer_cast<MCP::ProcessRequest>(spTask);
					if (spProcessTask && !spTask->IsFinished())
					{
						auto spRequest = spProcessTask->GetRequest();
						if (spRequest && MessageContextAccess::Get(*spRequest).IsStateless())
						{
							int iCode = (ERRNO_INVALID_PARAMS == iExecuteErrCode) ? ERRNO_INVALID_PARAMS : ERRNO_INTERNAL_ERROR;
							std::string strMessage = (ERRNO_INVALID_PARAMS == iCode) ? ERROR_MESSAGE_INVALID_PARAMS : ERROR_MESSAGE_INTERNAL_ERROR;
							SendErrorResponse(spRequest, iCode, strMessage);
						}
					}
				}
			}
		}

		return ERRNO_OK;
	}
}
