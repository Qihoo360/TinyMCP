#include "../TestHarness.h"

#include <Public/PublicDef.h>

#include <string>
#include <vector>

using namespace MCP;

TEST_CASE("unit.version_registry")
{
	const CProtocolRegistry& registry = CProtocolRegistry::GetGlobalRegistry();

	// Built-in legacy profile keeps handshake semantics and no modern features.
	const ProtocolProfile* pLegacy = registry.Find(PROTOCOL_VER_2024_11_05);
	CHECK(pLegacy != nullptr);
	if (pLegacy)
	{
		CHECK_EQ(Era_Handshake, pLegacy->eEra);
		CHECK(!pLegacy->Supports(Feature_ResultType));
		CHECK(!pLegacy->Supports(Feature_Discover));
		CHECK(!pLegacy->Supports(Feature_Mrtr));
		CHECK(pLegacy->SupportsMethod(METHOD_INITIALIZE));
		CHECK(!pLegacy->SupportsMethod(METHOD_SERVER_DISCOVER));
		CHECK_EQ(ERRNO_RESOURCE_NOT_FOUND, pLegacy->iResourceNotFoundCode);
	}

	// Built-in modern profile is stateless and enables every declared feature.
	const ProtocolProfile* pModern = registry.Find(PROTOCOL_VER_2026_07_28);
	CHECK(pModern != nullptr);
	if (pModern)
	{
		CHECK_EQ(Era_Stateless, pModern->eEra);
		for (size_t i = 0; i < static_cast<size_t>(Feature_Count); ++i)
			CHECK(pModern->bitsFeatures.test(i));
		CHECK(pModern->SupportsMethod(METHOD_SERVER_DISCOVER));
		CHECK(pModern->SupportsMethod(METHOD_SUBSCRIPTIONS_LISTEN));
		CHECK(!pModern->SupportsMethod(METHOD_INITIALIZE));
		CHECK_EQ(ERRNO_INVALID_PARAMS, pModern->iResourceNotFoundCode);
	}

	CHECK(registry.Find("1900-01-01") == nullptr);

	// Version priority follows the declaration order inside the registry.
	const ProtocolProfile* pDefaultHandshake = registry.GetDefaultHandshakeProfile();
	CHECK(pDefaultHandshake != nullptr);
	if (pDefaultHandshake)
		CHECK_EQ(std::string(PROTOCOL_VER_2024_11_05), pDefaultHandshake->strVersion);

	const ProtocolProfile* pLatestModern = registry.GetLatestStatelessProfile();
	CHECK(pLatestModern != nullptr);
	if (pLatestModern)
		CHECK_EQ(std::string(PROTOCOL_VER_2026_07_28), pLatestModern->strVersion);

	std::vector<std::string> vecVersions = registry.GetSupportedVersions();
	CHECK_EQ(static_cast<size_t>(2), vecVersions.size());
	if (vecVersions.size() == 2)
	{
		CHECK_EQ(std::string(PROTOCOL_VER_2026_07_28), vecVersions[0]);
		CHECK_EQ(std::string(PROTOCOL_VER_2024_11_05), vecVersions[1]);
	}

	CHECK_EQ(static_cast<size_t>(1), registry.GetSupportedVersions(Era_Stateless).size());
	CHECK_EQ(static_cast<size_t>(1), registry.GetSupportedVersions(Era_Handshake).size());

	// Default construction resolves the built-in handshake profile.
	ProtocolContext ctxDefault;
	CHECK(ctxDefault.IsValid());
	CHECK(!ctxDefault.IsStateless());
	CHECK_EQ(std::string(PROTOCOL_VER_2024_11_05), ctxDefault.strVersion);
	CHECK(!ctxDefault.Supports(Feature_ResultType));

	ProtocolContext ctxInvalid = ProtocolContext::Invalid("1900-01-01");
	CHECK(!ctxInvalid.IsValid());
	CHECK(!ctxInvalid.IsStateless());
	CHECK(!ctxInvalid.Supports(Feature_Caching));
	CHECK_EQ(std::string("1900-01-01"), ctxInvalid.strVersion);

	// A synthetic registry proves new versions only require data registration.
	ProtocolProfile profileFuture;
	profileFuture.strVersion = "2099-01-01";
	profileFuture.eEra = Era_Stateless;
	profileFuture.bitsFeatures.set(Feature_Caching);
	profileFuture.setSupportedMethods.insert(METHOD_SERVER_DISCOVER);
	profileFuture.iResourceNotFoundCode = ERRNO_INVALID_PARAMS;

	ProtocolProfile profileOld;
	profileOld.strVersion = "2098-01-01";
	profileOld.eEra = Era_Handshake;

	CProtocolRegistry syntheticRegistry(std::vector<ProtocolProfile>{ profileFuture, profileOld });

	const ProtocolProfile* pFuture = syntheticRegistry.Find("2099-01-01");
	CHECK(pFuture != nullptr);
	if (pFuture)
	{
		CHECK_EQ(Era_Stateless, pFuture->eEra);
		CHECK(pFuture->Supports(Feature_Caching));
		CHECK(!pFuture->Supports(Feature_Mrtr));
		CHECK(pFuture->SupportsMethod(METHOD_SERVER_DISCOVER));
	}

	const ProtocolProfile* pOld = syntheticRegistry.GetDefaultHandshakeProfile();
	CHECK(pOld != nullptr);
	if (pOld)
		CHECK_EQ(std::string("2098-01-01"), pOld->strVersion);
	CHECK_EQ(static_cast<size_t>(2), syntheticRegistry.GetSupportedVersions().size());
	CHECK_EQ(static_cast<size_t>(1), syntheticRegistry.GetSupportedVersions(Era_Handshake).size());
}
