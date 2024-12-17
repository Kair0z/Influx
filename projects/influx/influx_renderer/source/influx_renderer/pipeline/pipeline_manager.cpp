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

	pipeline* pipeline_manager::get_or_create_pipeline(const string& name, const pipeline_signature& signature)
	{
		pipeline* result = nullptr;
		auto& backend = renderer_backend::get_instance();

		const bool vertex_shader_found = backend.get_vertex_shaders().contains(signature.m_vs_name);
		const bool pixel_shader_found = backend.get_pixel_shaders().contains(signature.m_ps_name);

		if (!m_pipeline_map.contains(name))
		{
			if (vertex_shader_found && pixel_shader_found)
			{
				result = new pipeline(
					mp_device,
					signature,
					&backend.get_vertex_shaders()[signature.m_vs_name],
					&backend.get_pixel_shaders()[signature.m_ps_name]);

				m_pipeline_map[name].push_back(result);
			}
		}
		else
		{
			const vector<pipeline*>& existing_pipelines = m_pipeline_map[name];
			auto found = std::find_if(existing_pipelines.cbegin(), existing_pipelines.cend(), [&signature](const pipeline* pip)
			{
				return pip->get_signature() == signature;
			});

			// if we can't find, create a new variant
			if (found == existing_pipelines.cend())
			{
				if (vertex_shader_found && pixel_shader_found)
				{
					result = new pipeline(
						mp_device,
						signature,
						&backend.get_vertex_shaders()[signature.m_vs_name],
						&backend.get_pixel_shaders()[signature.m_ps_name]);

					m_pipeline_map[name].push_back(result);
				}
			}
			else
			{
				// we did find it
				return *found;
			}
		}

		return result;
	}

	uint64 pipeline_manager::get_num_pipelines() const
	{
		uint64 sum = 0u;
		for (const auto& pair : m_pipeline_map)
		{
			sum += pair.second.size();
		}
		return sum;
	}
}