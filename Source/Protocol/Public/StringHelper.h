#pragma once

#include <string>
#include <codecvt>
#include <locale>

namespace MCP
{
	namespace StringHelper
	{
		inline std::wstring utf8_string_to_wstring(const std::string& str) 
		{
			try
			{
				std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
				return converter.from_bytes(str);
			}
			catch (const std::exception& e) {
				return L"";
			}
		}

		inline std::string wstring_to_utf8_string(const std::wstring& strW)
		{
			try 
			{
				std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
				return converter.to_bytes(strW);
			}
			catch (const std::exception& e) {
				return "";
			}
		}

		inline std::vector<std::string> split_json_objects_string(const std::string& input)
		{
			std::vector<std::string> result;
			int braceCount = 0;
			bool inString = false;
			bool escape = false;

			size_t start = std::string::npos;

			for (size_t i = 0; i < input.size(); ++i)
			{
				char c = input[i];

				// Process escape characters inside the string
				if (inString)
				{
					if (escape)
					{
						escape = false;
					}
					else if (c == '\\')
					{
						escape = true;
					}
					else if (c == '"')
					{
						inString = false;
					}
					continue;
				}

				// String start
				if (c == '"')
				{
					inString = true;
					continue;
				}

				// Skip whitespace until the first '{' is encountered.
				if (braceCount == 0)
				{
					if (c == '{')
					{
						start = i;
						braceCount = 1;
					}
					continue;
				}

				// Count curly braces
				if (c == '{') braceCount++;
				else if (c == '}') braceCount--;

				// Complete a JSON object
				if (braceCount == 0 && start != std::string::npos)
				{
					result.push_back(input.substr(start, i - start + 1));
					start = std::string::npos;
				}
			}

			return result;
		}
	}
}