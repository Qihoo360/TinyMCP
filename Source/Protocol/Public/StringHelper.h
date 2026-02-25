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
				// 如果转换失败，返回空字符串
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
				// 如果转换失败，返回空字符串
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

				// 处理字符串内部的转义字符
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

				// 字符串开始
				if (c == '"')
				{
					inString = true;
					continue;
				}

				// 跳过空白直到遇到第一个 '{'
				if (braceCount == 0)
				{
					if (c == '{')
					{
						start = i;
						braceCount = 1;
					}
					continue;
				}

				// 计数大括号
				if (c == '{') braceCount++;
				else if (c == '}') braceCount--;

				// 完成一个 JSON 对象
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