#pragma once

#include <string>
#include <iostream>

// influx::core
#include "core/time.h"
#include "core/container/map.h"
#include "core/string.h"
#include "core/container/vector.h"

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
		using scope_pair = std::pair<string, float>;
		std::vector<scope_pair> sorted_scopes{};

		for (const auto& pair : g_scopedata)
		{
			const float avg_duration = pair.second.m_durationsum / pair.second.m_times_ran;
			sorted_scopes.push_back({ pair.first, avg_duration });
		}

		std::sort(sorted_scopes.begin(), sorted_scopes.end(), [](const scope_pair& a, const scope_pair& b)
		{
			return a.second > b.second;
		});

		std::cout << "scopes avg. duration:" << "\n";
		for (const auto& pair : sorted_scopes)
		{
			std::cout << "[" << pair.first << "] " << pair.second << " ms \n";
		}
	}
}

#define influx_scope_function() influx::scoped_event scoped_ev_{__FUNCTION__};
#define influx_scope(string) influx::scoped_event scoped_ev_{string};