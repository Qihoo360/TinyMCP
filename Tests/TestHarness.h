#pragma once
// Minimal zero-dependency test harness shared by unit and integration tests.
// Only the C++14 standard library and the vendored jsoncpp are used.

#include <json/json.h>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace MCP
{
	namespace Test
	{
		struct TestCase
		{
			std::string strName;
			const char* pFile{ nullptr };
			int iLine{ 0 };
			void (*pFunc)(){ nullptr };
		};

		class CTestContext
		{
		public:
			static CTestContext& GetInstance()
			{
				static CTestContext s_Instance;
				return s_Instance;
			}

			void Begin(const std::string& strName)
			{
				m_strName = strName;
				m_iFailures = 0;
			}

			void Fail(const char* pFile, int iLine, const std::string& strMessage)
			{
				++m_iFailures;
				std::cout << pFile << ":" << iLine << ": FAILED [" << m_strName << "]: " << strMessage << std::endl;
			}

			void FailEq(const char* pFile, int iLine, const std::string& strExpressionActual, const std::string& strExpressionExpected, const std::string& strActual, const std::string& strExpected)
			{
				++m_iFailures;
				std::cout << pFile << ":" << iLine << ": FAILED [" << m_strName << "]: CHECK_EQ(" << strExpressionActual << ", " << strExpressionExpected << ")"
					<< " actual=" << strActual << " expected=" << strExpected << std::endl;
			}

			int GetFailures() const
			{
				return m_iFailures;
			}

		private:
			CTestContext() = default;

			std::string m_strName;
			int m_iFailures{ 0 };
		};

		class CTestRegistry
		{
		public:
			static CTestRegistry& GetInstance()
			{
				static CTestRegistry s_Instance;
				return s_Instance;
			}

			bool Register(const std::string& strName, const char* pFile, int iLine, void (*pFunc)())
			{
				TestCase testCase;
				testCase.strName = strName;
				testCase.pFile = pFile;
				testCase.iLine = iLine;
				testCase.pFunc = pFunc;
				m_vecCases.push_back(testCase);

				return true;
			}

			const std::vector<TestCase>& GetCases() const
			{
				return m_vecCases;
			}

		private:
			CTestRegistry() = default;

			std::vector<TestCase> m_vecCases;
		};

		template <typename T>
		inline std::string ToDisplay(const T& value)
		{
			std::ostringstream oss;
			oss << value;
			return oss.str();
		}

		inline std::string ToDisplay(const std::string& value)
		{
			return std::string("\"") + value + "\"";
		}

		inline std::string ToDisplay(const char* value)
		{
			return std::string("\"") + (value ? value : "") + "\"";
		}

		inline std::string ToDisplay(const Json::Value& value)
		{
			Json::StreamWriterBuilder builder;
			builder["indentation"] = "  ";
			return Json::writeString(builder, value);
		}

		inline int RunCases(const std::vector<const TestCase*>& vecCases)
		{
			CTestContext& testContext = CTestContext::GetInstance();
			int iPassed = 0;
			int iFailed = 0;

			std::cout << "[==========] Running " << vecCases.size() << " test case(s)." << std::endl;
			for (auto pTestCase : vecCases)
			{
				if (!pTestCase || !pTestCase->pFunc)
					continue;

				testContext.Begin(pTestCase->strName);
				std::cout << "[ RUN      ] " << pTestCase->strName << std::endl;
				try
				{
					pTestCase->pFunc();
				}
				catch (const std::exception& e)
				{
					testContext.Fail(pTestCase->pFile, pTestCase->iLine, std::string("unhandled exception: ") + e.what());
				}
				catch (...)
				{
					testContext.Fail(pTestCase->pFile, pTestCase->iLine, "unhandled unknown exception");
				}

				if (0 == testContext.GetFailures())
				{
					++iPassed;
					std::cout << "[       OK ] " << pTestCase->strName << std::endl;
				}
				else
				{
					++iFailed;
					std::cout << "[  FAILED  ] " << pTestCase->strName << " (" << testContext.GetFailures() << " check(s) failed)" << std::endl;
				}
			}

			std::cout << "[==========] " << vecCases.size() << " test case(s) ran." << std::endl;
			std::cout << "[  PASSED  ] " << iPassed << " test case(s)." << std::endl;
			if (iFailed > 0)
				std::cout << "[  FAILED  ] " << iFailed << " test case(s)." << std::endl;

			return (0 == iFailed) ? 0 : 1;
		}
	}
}

#define TEST_CASE(name) \
	TEST_CASE_IMPL(name, __LINE__)

#define TEST_CASE_IMPL(name, id) \
	static void TestCaseFunc_##id(); \
	static const bool bTestCaseRegistered_##id = MCP::Test::CTestRegistry::GetInstance().Register(name, __FILE__, __LINE__, &TestCaseFunc_##id); \
	static void TestCaseFunc_##id()

#define CHECK(expression) \
	do \
	{ \
		if (!(expression)) \
			MCP::Test::CTestContext::GetInstance().Fail(__FILE__, __LINE__, std::string("CHECK(" #expression ") evaluated false")); \
	} while (0)

#define CHECK_EQ(actual, expected) \
	do \
	{ \
		const auto& _valueActual = (actual); \
		const auto& _valueExpected = (expected); \
		if (!(_valueActual == _valueExpected)) \
			MCP::Test::CTestContext::GetInstance().FailEq(__FILE__, __LINE__, #actual, #expected, \
				MCP::Test::ToDisplay(_valueActual), MCP::Test::ToDisplay(_valueExpected)); \
	} while (0)

#define CHECK_JSON_EQ(actual, expected) \
	do \
	{ \
		const Json::Value& _jsonActual = (actual); \
		const Json::Value& _jsonExpected = (expected); \
		if (!(_jsonActual == _jsonExpected)) \
			MCP::Test::CTestContext::GetInstance().FailEq(__FILE__, __LINE__, #actual, #expected, \
				MCP::Test::ToDisplay(_jsonActual), MCP::Test::ToDisplay(_jsonExpected)); \
	} while (0)
