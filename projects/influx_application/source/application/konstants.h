#pragma once
#include "core/basetypes.h"

namespace influx::applications
{
	constexpr static uint8 k_cache_line_num_bytes = 64u;
	constexpr static bool k_render_scene = true;
	constexpr static bool k_jobify = true;
	constexpr static uint8 k_max_num_job_threads = 4u;
	constexpr static uint64 k_num_entities = 10u;
	constexpr static bool k_force_vsync = false;
	constexpr static bool k_force_single_threaded = false;
	constexpr static uint64 k_event_queue_capacity = 4096u;

	constexpr static uint64 k_stats_capacity = 256u;
	constexpr static uint64 k_stats_log_frame_intv = k_stats_capacity;
}