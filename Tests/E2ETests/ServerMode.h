#pragma once
// Server side of the end-to-end tests. The test executable re-launches itself
// with `--server` and blocks inside the real stdio read loop until the client
// closes stdin.

namespace MCP
{
	namespace Test
	{
		int RunE2EServer(bool bLegacyOnly);
	}
}
