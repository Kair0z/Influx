#include "renderer_pch.h"
#include "pipeline_manager.h"

#include "pipeline.h"

#include "influx_renderer/renderer_backend.h"

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
			return nullptr;
		}

		return m_pipeline_map[name];
	}

	pipeline* pipeline_manager::get_scene_pipeline()
	{
		pipeline* scene_pipeline = get_pipeline(k_scene_pipeline_name);
		if (!scene_pipeline)
		{
			return nullptr;
		}

		return scene_pipeline;
	}

	pipeline* pipeline_manager::get_debug_pipeline()
	{
		pipeline* debug_pipeline = get_pipeline(k_debug_pipeline_name);
		if (!debug_pipeline)
		{
			return nullptr;
		}

		return debug_pipeline;
	}

	pipeline* pipeline_manager::get_or_create_pipeline(const string& name, const pipeline_key& key)
	{
		pipeline* result = nullptr;

		if (!m_pipeline_map.contains(name))
		{
			// try create using key
			auto& backend = renderer_backend::get_instance();
			const bool vertex_shader_found = backend.get_vertex_shaders().contains(key.m_vs_name);
			const bool pixel_shader_found = backend.get_pixel_shaders().contains(key.m_ps_name);

			if (vertex_shader_found && pixel_shader_found)
			{
				result = new_pipeline(name,
					backend.get_vertex_shaders()[key.m_vs_name],
					backend.get_pixel_shaders()[key.m_ps_name]);
			}
		}
		else
		{
			result = m_pipeline_map[name];
		}

		return result;
	}

	uint64 pipeline_manager::get_num_pipelines() const
	{
		return m_pipeline_map.size();
	}
}