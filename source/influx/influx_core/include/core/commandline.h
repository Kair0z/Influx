#pragma once

#include "core/container/vector.h"
#include "core/result.h"
#include "core/regex.h"
#include <string>

namespace influx
{
	using string = std::string;
	class commandline final
	{
	public:
		enum class e_common_prefix
		{
			line,		// -arg
			lineline,	// --arg
			slash,		// /arg
			num
		};
		static constexpr uint32 k_num_common_prefixes = static_cast<uint32>(e_common_prefix::num);
		inline static const char* k_common_prefixes[k_num_common_prefixes]
		{
			"-",
			"--",
			"/"
		};

		struct argument final
		{
			int		m_index;
			char*	m_cstring;
		};

		commandline(int argc, char* argv[]) : m_arguments{}
		{
			m_arguments.reserve(argc);

			for (int i = 0; i < argc; ++i) 
			{
				argument new_arg{};
				new_arg.m_index = i;
				new_arg.m_cstring = argv[i];
				m_arguments.push_back(new_arg);
			}

			cache_filters();
		}

		const vector<argument>& get_arguments() const
		{
			return m_arguments;
		}

		const vector<argument const*> get_arguments(e_common_prefix prefix) const
		{
			return m_filtered_arguments[static_cast<uint32>(prefix)];
		}

	private:
		void cache_filters()
		{
			for (uint32 i = 0u; i < k_num_common_prefixes; ++i)
			{
				const char* prefix = k_common_prefixes[i];
				const string prefix_str = string(prefix);
				const uint32 prefix_num_characters = static_cast<uint32>(prefix_str.size());

				vector<argument const*>& filter = m_filtered_arguments[i];
				for (const argument& arg : m_arguments)
				{
					const char* arg_cstring = arg.m_cstring;
					const string arg_string = string(arg_cstring);

					if (arg_string.size() <= prefix_num_characters)
						continue;

					// clumsy way of finding the first elements of the string :(
					bool is_prefix_match = true;
					for (uint32 j = 0u; j < prefix_num_characters; ++j)
					{
						// if a mismatch happens, no full match
						if (arg_string[j] != prefix[j])
						{
							is_prefix_match = false;
							continue;
						}
					}

					if (is_prefix_match)
					{
						filter.push_back(&arg);
					}
				}
			}
		}

	private:
		using argument_array = vector<argument>;
		vector<argument> m_arguments{};
		vector<argument const*> m_filtered_arguments[k_num_common_prefixes]{};
	};
}