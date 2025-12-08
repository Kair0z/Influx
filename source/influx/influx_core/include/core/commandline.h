#pragma once

#include "core/container/vector.h"
#include "core/container/map.h"
#include "core/result.h"
#include "core/regex.h"
#include "core/string.h"

namespace influx
{
	class cvar final
	{
		static constexpr const char* k_prefix = "+";

		enum e_setlevel
		{
			none = 0,
			runargs = 1 << 0,
			runtime = 1 << 1
		};

		using cstr = const char*;
		using str = string;

		using cvar_map = umap<string, cvar*>;
		using cvar_vector = vector<cvar*>;
		inline static cvar_map g_all_cvars_map{};
		inline static cvar_vector g_all_cvars{};

		cstr m_title;
		cstr m_value_default;
		cstr m_description;
		cstr m_value;
		e_setlevel m_value_setlevel = e_setlevel::none;

	public:
		cvar(cstr title, cstr value_default, cstr description)
			: m_title{title}, m_value_default{value_default}, m_description{description}
		{
			m_value = m_value_default;
			add_cvar(this);
		}

		cstr get_title() const { return m_title; }
		cstr get_value_default() const { return m_value_default; }
		cstr get_description() const { return m_description; }

		template <typename _t>
		void set_value(const _t& new_value, e_setlevel level)
		{
			m_value = to_string(new_value);
			m_value_setlevel = e_setlevel(m_value_setlevel | level);
		}

		bool is_set() const
		{
			return m_value_setlevel != e_setlevel::none;
		}

		template <typename _t>
		_t get_value()
		{
			// todo... string -> _t
			_t result{};
			from_string<_t>(m_value, result);
			return result;
		}

	private:
		static void add_cvar(cvar* cvar)
		{
			if (cvar == nullptr)
				return;

			cstr title = cvar->get_title();
			if (!g_all_cvars_map.contains(title))
			{
				g_all_cvars.push_back(cvar);
				g_all_cvars_map[title] = g_all_cvars.back();
			}
		}

	public:
		static cvar* find_cvar(cstr title)
		{
			if (g_all_cvars_map.contains(title) == false)
				return nullptr;
			return g_all_cvars_map[title];
		}

		static void parse_runargs(int argc, char** argv)
		{
			static constexpr int k_args_begin = 1;
			for (int i = k_args_begin; i < argc; ++i)
			{
				char* arg = argv[i];
				str arg_str = arg;

				// register the found cvar as 'set' (by runargs)
				uint64 prefix_loc = arg_str.find(k_prefix, 0u);
				str cvar_title = arg_str.substr(prefix_loc + 1);
				cvar* var = find_cvar(cvar_title.c_str());
				if (var)
				{
					var->m_value = var->m_value_default;
					var->m_value_setlevel = e_setlevel(var->m_value_setlevel | e_setlevel::runargs);
				}
				else continue; // this argument is not a cvar

				// check if the next argument is a non-prefix str.
				// if it isn't, that means the next argument is the parameter
				// multiple parameters are (for now) not supported
				const bool is_last_arg = i + 1 >= argc;
				if (!is_last_arg)
				{
					const char* next_arg = argv[i + 1];
					const str next_arg_str = next_arg;
					const bool next_has_prefix = next_arg_str.find(k_prefix, 0u) != std::string::npos; // next_arg_str.size();
					const bool next_is_parameter = !next_has_prefix;
					if (next_is_parameter)
					{
						var->m_value = next_arg;
						i++; // skip past this parsed parameter argument
					}
				}
			}
		}
	};

#if 0
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
#endif
}