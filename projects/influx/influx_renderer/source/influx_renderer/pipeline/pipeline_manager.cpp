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

	graphics_pipeline* pipeline_manager::get_or_create_pipeline(const string& name, const graphics_pipeline_signature& signature)
	{
		return get_or_create_pipeline<graphics::e_pipeline_type::graphics>(name, signature);
	}

	compute_pipeline* pipeline_manager::get_or_create_pipeline(const string& name, const compute_pipeline_signature& signature)
	{
		return get_or_create_pipeline<graphics::e_pipeline_type::compute>(name, signature);
	}

	raytracing_pipeline* pipeline_manager::get_or_create_pipeline(const string& name, const raytracing_pipeline_signature& signature)
	{
		return get_or_create_pipeline<graphics::e_pipeline_type::raytracing>(name, signature);
	}

	graphics_pipeline* pipeline_manager::create_pipeline(const string& name, const graphics_pipeline_signature& signature, const shader_data& vs, const shader_data& ps)
	{
		graphics_pipeline* result = nullptr;
		result = new graphics_pipeline(*mp_device, signature);

		get_map<graphics::e_pipeline_type::graphics>()[name].push_back(result);
		return result;
	}

	compute_pipeline* pipeline_manager::create_pipeline(const string& name, const compute_pipeline_signature& signature, const shader_data& cs)
	{
		return nullptr;
	}

	raytracing_pipeline* pipeline_manager::create_pipeline(const string& name, const raytracing_pipeline_signature& signature, const shader_data& rgs)
	{
		return nullptr;
	}

	uint32 pipeline_manager::get_num_pipelines() const
	{
		uint32 sum = 0u;
		for (const auto& pair : get_map<graphics::e_pipeline_type::graphics>())
		{
			sum += static_cast<uint32>(pair.second.size());
		}
		return sum;
	}
}