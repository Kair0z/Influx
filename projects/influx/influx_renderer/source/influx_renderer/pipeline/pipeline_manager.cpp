#include "renderer_pch.h"
#include "pipeline_manager.h"

#include "pipeline.h"

namespace influx::renderer
{
	pipeline_manager::pipeline_manager(graphics::device* device)
		: mp_device{ device }
	{

	}

	pipeline* pipeline_manager::new_pipeline(const string& name, const renderer::shader_data& vertex_shader, const renderer::shader_data& pixel_shader)
	{
		if (m_pipeline_map.contains(name))
		{
			influx_assert(false);
			return m_pipeline_map[name];
		}

		pipeline* new_pipeline = new pipeline(
			mp_device, 
			&vertex_shader, 
			&pixel_shader);

#if _DEBUG
		new_pipeline->set_name(name);
#endif

		m_pipeline_map[name] = new_pipeline;
		return new_pipeline;
	}

	pipeline* pipeline_manager::get_pipeline(const string& name)
	{
		if (!m_pipeline_map.contains(name))
		{
			influx_assert(false);
			return nullptr;
		}

		return m_pipeline_map[name];
	}

	pipeline* pipeline_manager::get_scene_pipeline()
	{
		return get_pipeline(k_scene_pipeline_name);
	}

	uint64 pipeline_manager::get_num_pipelines() const
	{
		return m_pipeline_map.size();
	}
}