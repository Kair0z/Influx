#pragma once

#include <string>
#include <iostream>

#include "core/time.h"
#include "core/container/map.h"
#include "core/string.h"

namespace influx
{
	struct scope_footprint
	{
		float m_durationsum; // ms
		uint32 m_times_ran = 0u;
	};

	inline umap<string, scope_footprint> g_scopedata{};

	inline void begin_event(const std::string& name)
	{
		g_scopedata[name].m_times_ran++;
	}

	inline void end_event(const string& name, const scope_footprint& data)
	{
		g_scopedata[name].m_durationsum += data.m_durationsum;
	}

	class scoped_event final
	{
	public:
		inline scoped_event(const std::string& str)
			: m_name{str}
		{
			m_start = m_end = time::get_now();

			begin_event(str);
		}

		inline ~scoped_event()
		{
			m_end = time::get_now();
			scope_footprint footprint{};
			footprint.m_durationsum = time::get_ms_between<float>(m_end, m_start);
			end_event(m_name, footprint);
		}

	private:
		time::point m_start;
		time::point m_end;
		string m_name;
	};

	inline void log_scopedata()
	{
		for (const auto& pair : g_scopedata)
		{
			const float avg_duration = pair.second.m_durationsum / pair.second.m_times_ran;
			std::cout << "[" << pair.first << "] " << avg_duration << " ms \n";
		}
	}
}

#define influx_scope_function() influx::scoped_event scoped_ev_{__FUNCTION__};
#define influx_scope(string) influx::scoped_event scoped_ev_{string};