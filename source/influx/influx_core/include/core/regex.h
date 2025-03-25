#pragma once

#include "core/string.h"
#include <regex>

namespace influx
{
	using match = std::smatch;

	class regex
	{
	public:
		template <typename _func>
		static void for_each_match(const string& text, const string& pattern, _func&& func)
		{
			std::regex reg(pattern);
			auto words_begin = std::sregex_iterator(text.begin(), text.end(), reg);
			auto words_end = std::sregex_iterator();

			for (std::sregex_iterator i = words_begin; i != words_end; ++i)
			{
				match match = *i;
				func(match.str());
			}
		}

		static vector<string> get_all_matches(const string& text, const string& pattern)
		{
			vector<string> results{};

			std::regex reg(pattern);
			match match{};

			auto start = text.cbegin();
			while (std::regex_search(start, text.cend(), match, reg))
			{
				results.push_back(match[1].str());
				start = match.suffix().first;  // Move past last match
			}

			return results;
		}

		static bool has_match(const string& text, const string& pattern)
		{
			return get_all_matches(text, pattern).empty() == false;
		}
	};
}