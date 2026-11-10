#pragma once
// Origin whitelist rule used by the HTTP transports for DNS-rebinding
// protection. Header-only and free of httplib so it can be unit tested.

#include <string>
#include <vector>

namespace MCP
{
	// An empty Origin (same-origin tools such as curl) is allowed. Otherwise an
	// entry must match the whole origin; the only suffix tolerated is ":<port>",
	// so "http://localhost" admits any local port but never
	// "http://localhost.evil.com".
	inline bool IsOriginAllowedBy(const std::vector<std::string>& vecAllowed, const std::string& strOrigin)
	{
		if (strOrigin.empty())
			return true;

		for (const auto& strAllowed : vecAllowed)
		{
			if (strAllowed.empty())
				continue;
			if (strOrigin.compare(0, strAllowed.size(), strAllowed) != 0)
				continue;

			std::string strRest = strOrigin.substr(strAllowed.size());
			if (strRest.empty())
				return true;

			// A port suffix is only meaningful when the entry does not already
			// pin one, otherwise "http://localhost:3000:1" would slip through.
			size_t nSchemeEnd = strAllowed.find("://");
			size_t nHostStart = (std::string::npos == nSchemeEnd) ? 0 : nSchemeEnd + 3;
			bool bEntryHasPort = (std::string::npos != strAllowed.find(':', nHostStart));
			if (!bEntryHasPort && strRest.size() > 1 && strRest[0] == ':'
				&& strRest.find_first_not_of("0123456789", 1) == std::string::npos)
				return true;
		}

		return false;
	}
}
