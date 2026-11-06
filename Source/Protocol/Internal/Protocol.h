#pragma once
// Internal to the SDK. Applications pick protocol revisions through
// CMCPServer::SetSupportedProtocolVersions; profiles, feature bits and the
// registry never appear in public headers.

#include "../Public/PublicDef.h"

#include <bitset>
#include <set>
#include <string>
#include <vector>

namespace MCP
{
	// Interaction model of a protocol revision. This is the only architectural
	// extension point: adding a version number must not add a new era.
	enum ProtocolEra
	{
		Era_Handshake,
		Era_Stateless,
	};

	// Feature flags gate every version-specific field. Business logic must call
	// ProtocolContext::Supports() instead of comparing version strings.
	enum ProtocolFeature
	{
		Feature_ResultMeta,
		Feature_EntityMeta,
		Feature_ImplementationExtras,
		Feature_ResultType,
		Feature_Discover,
		Feature_Caching,
		Feature_Mrtr,
		Feature_ListenSubscriptions,
		Feature_Extensions,
		Feature_CompletionsCapability,
		Feature_ToolTitle,
		Feature_ToolAnnotations,
		Feature_StructuredContent,
		Feature_ContentAudio,
		Feature_ContentResourceLink,
		Feature_ContentToolUse,
		Feature_ContentAnnotations,
		Feature_LogLevelInMeta,
		Feature_Count,
	};

	struct ProtocolProfile
	{
		std::string strVersion;
		ProtocolEra eEra{ Era_Handshake };
		std::bitset<Feature_Count> bitsFeatures;
		std::set<std::string> setSupportedMethods;
		int iResourceNotFoundCode{ ERRNO_RESOURCE_NOT_FOUND };

		bool Supports(ProtocolFeature eFeature) const
		{
			return bitsFeatures.test(static_cast<size_t>(eFeature));
		}

		bool SupportsMethod(const std::string& strMethod) const
		{
			return setSupportedMethods.find(strMethod) != setSupportedMethods.end();
		}
	};

	// Version context carried by every message. The default construction always
	// resolves the built-in 2024-11-05 handshake profile so objects created by
	// existing code keep their legacy behavior.
	struct ProtocolContext
	{
		std::string strVersion;
		const ProtocolProfile* pProfile{ nullptr };

		ProtocolContext();
		static ProtocolContext Invalid(const std::string& strRequested);

		bool IsValid() const;
		bool IsStateless() const;
		bool Supports(ProtocolFeature eFeature) const;
		bool SupportsMethod(const std::string& strMethod) const;
	};

	// Read-only version table. Version differences are declared as profile data;
	// the order of the profile list defines version priority.
	class CProtocolRegistry
	{
	public:
		explicit CProtocolRegistry(const std::vector<ProtocolProfile>& vecProfiles);

		static const CProtocolRegistry& GetGlobalRegistry();

		const ProtocolProfile* Find(const std::string& strVersion) const;
		const ProtocolProfile* GetDefaultHandshakeProfile() const;
		const ProtocolProfile* GetLatestStatelessProfile() const;
		std::vector<std::string> GetSupportedVersions() const;
		std::vector<std::string> GetSupportedVersions(ProtocolEra eEra) const;

	private:
		std::vector<ProtocolProfile> m_vecProfiles;
	};
}
