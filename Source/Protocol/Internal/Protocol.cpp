#include "Protocol.h"

namespace MCP
{
	namespace
	{
		void AddModernMethodNames(ProtocolProfile& profile)
		{
			profile.setSupportedMethods.insert(METHOD_SERVER_DISCOVER);
			profile.setSupportedMethods.insert(METHOD_PING);
			profile.setSupportedMethods.insert(METHOD_SUBSCRIPTIONS_LISTEN);
			profile.setSupportedMethods.insert(METHOD_TOOLS_LIST);
			profile.setSupportedMethods.insert(METHOD_TOOLS_CALL);
			profile.setSupportedMethods.insert(METHOD_RESOURCES_LIST);
			profile.setSupportedMethods.insert(METHOD_RESOURCES_READ);
			profile.setSupportedMethods.insert(METHOD_RESOURCES_TEMPLATES_LIST);
			profile.setSupportedMethods.insert(METHOD_PROMPTS_LIST);
			profile.setSupportedMethods.insert(METHOD_PROMPTS_GET);
			profile.setSupportedMethods.insert(METHOD_COMPLETION_COMPLETE);
		}

		void AddLegacyMethodNames(ProtocolProfile& profile)
		{
			profile.setSupportedMethods.insert(METHOD_INITIALIZE);
			profile.setSupportedMethods.insert(METHOD_PING);
			profile.setSupportedMethods.insert(METHOD_TOOLS_LIST);
			profile.setSupportedMethods.insert(METHOD_TOOLS_CALL);
			profile.setSupportedMethods.insert(METHOD_RESOURCES_LIST);
			profile.setSupportedMethods.insert(METHOD_RESOURCES_READ);
			profile.setSupportedMethods.insert(METHOD_RESOURCES_TEMPLATES_LIST);
			profile.setSupportedMethods.insert(METHOD_RESOURCES_SUBSCRIBE);
			profile.setSupportedMethods.insert(METHOD_RESOURCES_UNSUBSCRIBE);
			profile.setSupportedMethods.insert(METHOD_PROMPTS_LIST);
			profile.setSupportedMethods.insert(METHOD_PROMPTS_GET);
			profile.setSupportedMethods.insert(METHOD_COMPLETION_COMPLETE);
			profile.setSupportedMethods.insert(METHOD_LOGGING_SET_LEVEL);
		}

		ProtocolProfile MakeModernProfile()
		{
			ProtocolProfile profile;
			profile.strVersion = PROTOCOL_VER_2026_07_28;
			profile.eEra = Era_Stateless;

			// Features are listed explicitly. A feature introduced for a later
			// revision must not be enabled here automatically, otherwise the
			// behaviour of an already released version would silently change.
			const ProtocolFeature arrFeatures[] = {
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
			};
			for (const ProtocolFeature& eFeature : arrFeatures)
				profile.bitsFeatures.set(static_cast<size_t>(eFeature));

			profile.iResourceNotFoundCode = ERRNO_INVALID_PARAMS;
			AddModernMethodNames(profile);
			return profile;
		}

		ProtocolProfile MakeLegacyProfile()
		{
			ProtocolProfile profile;
			profile.strVersion = PROTOCOL_VER_2024_11_05;
			profile.eEra = Era_Handshake;
			profile.iResourceNotFoundCode = ERRNO_RESOURCE_NOT_FOUND;
			AddLegacyMethodNames(profile);
			return profile;
		}
	}

	ProtocolContext::ProtocolContext()
	{
		std::shared_ptr<const ProtocolProfile> pDefault =
			CProtocolRegistry::GetGlobalRegistry().GetDefaultHandshakeProfile();
		if (pDefault)
		{
			strVersion = pDefault->strVersion;
			pProfile = pDefault;
		}
	}

	ProtocolContext ProtocolContext::Invalid(const std::string& strRequested)
	{
		ProtocolContext context;
		context.strVersion = strRequested;
		context.pProfile = nullptr;
		return context;
	}

	bool ProtocolContext::IsValid() const
	{
		return pProfile != nullptr;
	}

	bool ProtocolContext::IsStateless() const
	{
		return pProfile != nullptr && pProfile->eEra == Era_Stateless;
	}

	bool ProtocolContext::Supports(ProtocolFeature eFeature) const
	{
		return pProfile != nullptr && pProfile->Supports(eFeature);
	}

	bool ProtocolContext::SupportsMethod(const std::string& strMethod) const
	{
		return pProfile != nullptr && pProfile->SupportsMethod(strMethod);
	}

	CProtocolRegistry::CProtocolRegistry(const std::vector<std::shared_ptr<const ProtocolProfile>>& vecProfiles)
		: m_vecProfiles(vecProfiles)
	{
	}

	const CProtocolRegistry& CProtocolRegistry::GetGlobalRegistry()
	{
		static const CProtocolRegistry s_Registry(
			std::vector<std::shared_ptr<const ProtocolProfile>>{
				std::make_shared<const ProtocolProfile>(MakeModernProfile()),
				std::make_shared<const ProtocolProfile>(MakeLegacyProfile()) });
		return s_Registry;
	}

	std::shared_ptr<const ProtocolProfile> CProtocolRegistry::Find(const std::string& strVersion) const
	{
		for (size_t i = 0; i < m_vecProfiles.size(); ++i)
		{
			if (m_vecProfiles[i]->strVersion == strVersion)
				return m_vecProfiles[i];
		}

		return nullptr;
	}

	std::shared_ptr<const ProtocolProfile> CProtocolRegistry::GetDefaultHandshakeProfile() const
	{
		for (size_t i = 0; i < m_vecProfiles.size(); ++i)
		{
			if (m_vecProfiles[i]->eEra == Era_Handshake)
				return m_vecProfiles[i];
		}

		return nullptr;
	}

	std::shared_ptr<const ProtocolProfile> CProtocolRegistry::GetLatestStatelessProfile() const
	{
		for (size_t i = 0; i < m_vecProfiles.size(); ++i)
		{
			if (m_vecProfiles[i]->eEra == Era_Stateless)
				return m_vecProfiles[i];
		}

		return nullptr;
	}

	std::vector<std::string> CProtocolRegistry::GetSupportedVersions() const
	{
		std::vector<std::string> vecVersions;
		vecVersions.reserve(m_vecProfiles.size());
		for (size_t i = 0; i < m_vecProfiles.size(); ++i)
			vecVersions.push_back(m_vecProfiles[i]->strVersion);

		return vecVersions;
	}

	std::vector<std::string> CProtocolRegistry::GetSupportedVersions(ProtocolEra eEra) const
	{
		std::vector<std::string> vecVersions;
		for (size_t i = 0; i < m_vecProfiles.size(); ++i)
		{
			if (m_vecProfiles[i]->eEra == eEra)
				vecVersions.push_back(m_vecProfiles[i]->strVersion);
		}

		return vecVersions;
	}
}
