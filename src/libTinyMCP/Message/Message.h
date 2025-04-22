#pragma once
// 为保证较好的跨平台特性，MCP命名空间的代码均使用标准c++编码
// 非必要情况下，禁止使用特定系统平台API

#include "../Public/PublicDef.h"
#include <atomic>

namespace MCP
{
	struct Message
	{
	public:
		Message(MessageType eMsgType, MessageCategory eCategory, bool bNeedIdentity)
			: eMessageType(eMsgType)
			, eMessageCategory(eCategory)
		{
			if (bNeedIdentity)
			{
				auto now = std::chrono::system_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
				ullTimestamp = duration.count();

				ulRuntimeId = s_ulIdBase++;
			}
		}
		virtual ~Message() {};

		MessageCategory eMessageCategory{ MessageCategory_Unknown };
		MessageType eMessageType{ MessageType_Unknown };
		bool bExist{ true };	// 可选时对象是否存在
		unsigned long long ullTimestamp{ 0 };	// 毫秒级时间戳
		unsigned long ulRuntimeId{ 0 };	// 运行时id
		static std::atomic_ulong s_ulIdBase;

		int Serialize(std::string& str) const
		{
			Json::Value jMsg(Json::objectValue);
			int iErrCode = DoSerialize(jMsg);
			if (ERRNO_OK != iErrCode)
				return iErrCode;

			Json::FastWriter writer;
			str = writer.write(jMsg);

			return ERRNO_OK;
		}

		int Deserialize(const std::string& str)
		{
			Json::Reader reader;
			Json::Value jMsg(Json::objectValue);
			if (!reader.parse(str, jMsg) || !jMsg.isObject())
				return ERRNO_PARSE_ERROR;

			int iErrCode = DoDeserialize(jMsg);
			if (ERRNO_OK != iErrCode)
				return iErrCode;

			return ERRNO_OK;
		}

		virtual bool IsValid() const = 0;
		virtual int DoSerialize(Json::Value& jMsg) const = 0;
		virtual int DoDeserialize(const Json::Value& jMsg) = 0;
	};
}
