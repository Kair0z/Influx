#pragma once
#include "core/basetypes.h"
#include "core/container/vector.h"

namespace influx::renderer
{
	struct frame_stats final
	{
		float m_ms_acquire = 0.0f; // percentage of total spent on acquire
		float m_ms_build = 0.0f; // time spent on generating the cmdlist
		float m_ms_frame = 0.0f; // time spend on a frame

		frame_stats& operator+=(const frame_stats& stats)
		{
			m_ms_acquire += stats.m_ms_acquire;
			m_ms_build += stats.m_ms_build;
			m_ms_frame += stats.m_ms_frame;
			return *this;
		}

		frame_stats& operator/=(float val)
		{
			m_ms_acquire /= val;
			m_ms_build /= val;
			m_ms_frame /= val;
			return *this;
		}

		static frame_stats average(const vector<frame_stats>& stats)
		{
			frame_stats result{};
			uint64 num = stats.size();
			for (uint64 i = 0u; i < num; ++i)
			{
				result += stats[i];
			}

			result /= (float)num;
			return result;
		}
	};
}