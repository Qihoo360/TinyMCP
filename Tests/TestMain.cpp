#include "TestHarness.h"
#include "IntegrationTests/Golden.h"

#include <cstring>
#include <string>
#include <vector>

#ifdef _MSC_VER
#include <crtdbg.h>
#include <cstdlib>
#endif

namespace
{
	// Route CRT assertion/abort reports to stderr so a failing test fails in the
	// console instead of blocking the run on a modal debug dialog.
	void ConfigureCrashReporting()
	{
#ifdef _MSC_VER
		_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
		_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
	}
	const char* USAGE =
		"Usage:\n"
		"  TinyMCPTests --unit                 Run all unit test cases (unit.*).\n"
		"  TinyMCPTests --unit <module>        Run a single unit module (unit.<module>).\n"
		"  TinyMCPTests --case <name>          Run a single integration scenario (integration.<name>).\n"
		"  TinyMCPTests --list                 List all registered test cases.\n"
		"  TinyMCPTests --generate-golden      Regenerate the legacy golden baseline.\n";

	int ListCases()
	{
		auto& registry = MCP::Test::CTestRegistry::GetInstance();
		for (auto& testCase : registry.GetCases())
		{
			std::cout << testCase.strName << std::endl;
		}

		return 0;
	}

	int RunSelected(const std::vector<std::string>& vecNames)
	{
		auto& registry = MCP::Test::CTestRegistry::GetInstance();
		std::vector<const MCP::Test::TestCase*> vecSelected;
		for (auto& testCase : registry.GetCases())
		{
			for (auto& strName : vecNames)
			{
				if (testCase.strName == strName)
				{
					vecSelected.push_back(&testCase);
					break;
				}
			}
		}

		if (vecSelected.empty())
		{
			std::cout << "No test case matched the given filter." << std::endl;
			return 1;
		}

		return MCP::Test::RunCases(vecSelected);
	}

	std::vector<std::string> SelectUnitAll()
	{
		std::vector<std::string> vecNames;
		auto& registry = MCP::Test::CTestRegistry::GetInstance();
		for (auto& testCase : registry.GetCases())
		{
			if (0 == testCase.strName.compare(0, 5, "unit."))
				vecNames.push_back(testCase.strName);
		}

		return vecNames;
	}
}

int main(int argc, char** argv)
{
	ConfigureCrashReporting();

	if (argc < 2)
	{
		std::cout << USAGE;
		return 1;
	}

	for (int i = 1; i < argc; ++i)
	{
		if (0 == std::strcmp(argv[i], "--list"))
		{
			return ListCases();
		}
		else if (0 == std::strcmp(argv[i], "--generate-golden"))
		{
			return MCP::Test::GenerateLegacyGolden();
		}
		else if (0 == std::strcmp(argv[i], "--unit"))
		{
			if (i + 1 < argc && argv[i + 1][0] != '-')
			{
				return RunSelected({ std::string("unit.") + argv[i + 1] });
			}

			return RunSelected(SelectUnitAll());
		}
		else if (0 == std::strcmp(argv[i], "--case"))
		{
			if (i + 1 >= argc)
			{
				std::cout << "--case requires a scenario name." << std::endl;
				return 1;
			}

			std::string strName = argv[i + 1];
			if (0 != strName.compare(0, 12, "integration."))
				strName = std::string("integration.") + strName;

			return RunSelected({ strName });
		}
	}

	std::cout << USAGE;
	return 1;
}
