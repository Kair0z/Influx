#include "renderer_pch.h"
#include "pipeline_manager.h"

// influx::renderer
#include "pipeline.h"
#include "influx_renderer/renderer_backend.h"

namespace influx::renderer
{
	pipeline_manager::pipeline_manager(graphics::device* device)
		: mp_device{ device }
	{

	}

	bool pipeline_manager::has_pipeline(const string& name) const
	{
		return m_pipeline_map.contains(name);
	}

	pipeline* pipeline_manager::create_pipeline(
		const string& name,
		const pipeline_signature& signature,
		const shader_data* vs,
		const shader_data* ps)
	{
		pipeline* result = nullptr;
		result = new pipeline(
			mp_device,
			signature,
			vs,
			ps);

		m_pipeline_map[name].push_back(result);
		return result;
	}

	pipeline* pipeline_manager::find_pipeline(const string& name, const pipeline_signature& signature)
	{
		const vector<pipeline*>& existing_pipelines = m_pipeline_map[name];
		auto found = std::find_if(existing_pipelines.cbegin(), existing_pipelines.cend(), [&signature](const pipeline* pip)
		{
			return pip->get_signature() == signature;
		});

		return found != existing_pipelines.cend() ? *found : nullptr;
	}

	pipeline* pipeline_manager::get_or_create_pipeline(const string& name, const pipeline_signature& signature)
	{
		pipeline* result = nullptr;
		auto& backend = renderer_backend::get_instance();

		const auto& vs_shaders = backend.get_vertex_shaders();
		const auto& ps_shaders = backend.get_pixel_shaders();
		const bool vertex_shader_found = vs_shaders.contains(signature.m_vs_name);
		const bool pixel_shader_found = ps_shaders.contains(signature.m_ps_name);

		if (!vertex_shader_found || !pixel_shader_found)
		{
			return nullptr;
		}

		if (!has_pipeline(name))
		{
			result = create_pipeline(
				name,
				signature,
				&vs_shaders.at(signature.m_vs_name),
				&ps_shaders.at(signature.m_ps_name));
		}
		else
		{
			pipeline* found = find_pipeline(name, signature);
			if (found == nullptr)
			{
				result = create_pipeline(
					name,
					signature,
					&vs_shaders.at(signature.m_vs_name),
					&ps_shaders.at(signature.m_ps_name));
			}
			else
			{
				result = found;
			}
		}

		return result;
	}

	uint32 pipeline_manager::get_num_pipelines() const
	{
		uint32 sum = 0u;
		for (const auto& pair : m_pipeline_map)
		{
			sum += static_cast<uint32>(pair.second.size());
		}
		return sum;
	}
}