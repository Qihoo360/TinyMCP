#pragma once
// Internal to the SDK. Protocol revision adaptation for individual entities is
// a cross-cutting concern, so it lives behind this accessor instead of in the
// public API of every entity class. Applications never need this header.

#include "Protocol.h"
#include "../Message/BasicMessage.h"
#include "../Message/Response.h"
#include "../Message/Notification.h"

namespace MCP
{
	struct VersionExtrasAccess
	{
		// Deserialize / Serialize overloads keep the call sites uniform while the
		// implementation stays a member of the entity that owns the fields.
		static int Serialize(const Implementation& obj, Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoSerializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Deserialize(Implementation& obj, const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoDeserializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Serialize(const ServerCapabilities& obj, Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoSerializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Serialize(const Tool& obj, Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoSerializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Deserialize(Tool& obj, const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoDeserializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Serialize(const TextContent& obj, Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoSerializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Deserialize(TextContent& obj, const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoDeserializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Serialize(const ImageContent& obj, Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoSerializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Deserialize(ImageContent& obj, const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoDeserializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Serialize(const AudioContent& obj, Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoSerializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Deserialize(AudioContent& obj, const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoDeserializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Serialize(const ResourceLink& obj, Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoSerializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Deserialize(ResourceLink& obj, const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoDeserializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Serialize(const ToolUseContent& obj, Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoSerializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Deserialize(ToolUseContent& obj, const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoDeserializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Serialize(const ToolResultContent& obj, Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoSerializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Deserialize(ToolResultContent& obj, const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoDeserializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Serialize(const EmbeddedResource& obj, Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoSerializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Deserialize(EmbeddedResource& obj, const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoDeserializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Serialize(const TextResourceContents& obj, Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoSerializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Deserialize(TextResourceContents& obj, const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoDeserializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Serialize(const BlobResourceContents& obj, Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoSerializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Deserialize(BlobResourceContents& obj, const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoDeserializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Serialize(const Resource& obj, Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoSerializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Deserialize(Resource& obj, const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoDeserializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Serialize(const ResourceTemplate& obj, Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoSerializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Deserialize(ResourceTemplate& obj, const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoDeserializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Serialize(const Prompt& obj, Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoSerializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Deserialize(Prompt& obj, const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoDeserializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Serialize(const PromptMessage& obj, Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoSerializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Serialize(const MetaObject& obj, Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoSerializeVersionExtras(jMsg, ctxProtocol);
		}

		static int Deserialize(MetaObject& obj, const Json::Value& jMsg, const ProtocolContext& ctxProtocol)
		{
			return obj.DoDeserializeVersionExtras(jMsg, ctxProtocol);
		}

		static void WriteResultFields(const Response& obj, Json::Value& jResult, const ProtocolContext& ctxProtocol, const Json::Value& jServerInfo)
		{
			obj.WriteVersionResultFields(jResult, ctxProtocol, jServerInfo);
		}

		static void WriteParamsMeta(const Notification& obj, Json::Value& jParams, const ProtocolContext& ctxProtocol)
		{
			obj.WriteVersionParamsMeta(jParams, ctxProtocol);
		}
	};
}
