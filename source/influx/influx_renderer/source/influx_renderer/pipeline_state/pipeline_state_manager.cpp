#include "renderer_pch.h"
#include "pipeline_state_manager.h"

// influx::renderer
#include "pipeline_state.h"
#include "influx_renderer/renderer_backend.h"

namespace influx::renderer
{
	pipeline_state_manager::pipeline_state_manager(graphics::device* device)
		: mp_device{ device }
	{

	}

	uint32 pipeline_state_manager::get_num_pipelines() const
	{
		return
			static_cast<uint32>(
			get_map<graphics::e_pipeline_type::graphics>().size() +
			get_map<graphics::e_pipeline_type::compute>().size() +
			get_map<graphics::e_pipeline_type::raytracing>().size());
	}
}