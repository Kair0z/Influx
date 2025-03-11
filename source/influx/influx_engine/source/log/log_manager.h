#pragma once

// influx::core
#include "core/enum.h"
#include "core/log.h"
#include "core/container/array.h"
#include "core/string.h"

namespace influx::engine
{
	enum class e_log_category : uint8
	{
		info		= 1 << 0,
		warning		= 1 << 1,
		error		= 1 << 2
	};

#pragma region translation
	constexpr influx::e_log_category translate(e_log_category cat)
	{
		if (has_flag(cat, e_log_category::info)) return influx::e_log_category::normal;
		if (has_flag(cat, e_log_category::warning)) return influx::e_log_category::warning;
		if (has_flag(cat, e_log_category::error)) return influx::e_log_category::error;
		return influx::e_log_category::normal;
	}
#pragma endregion

	class log_manager final
	{
		static constexpr uint32 k_capacity = 4096u;
		vector<e_log_category> m_categories{};
		vector<string> m_lines{};
		uint32 m_linecount = 0u;

	public:
		log_manager();
		~log_manager();

		template <typename ..._args>
		inline void log(e_log_category category, const string& format, const _args&... args)
		{
			const string logstr = influx::log( translate(category), format, args...);
			m_categories[m_linecount] = category;
			m_lines[m_linecount] = logstr;
			m_linecount++;
			influx_assert(m_linecount < k_capacity);
		}

		void tick();
		void flush_to_file(uint32 max_num_lines = (uint32)-1);
	};
}
ENABLE_ENUM_BIT_OPERATORS(influx::engine::e_log_category);