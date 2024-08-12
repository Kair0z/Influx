#pragma once
#include "core/container/map.h"

namespace influx::graphics
{
	class device;
}

namespace influx::renderer
{
	class pipeline;
}

namespace influx::renderer
{
	static const char* k_scene_pipeline_name = "pip_scene";

	class pipeline_manager final
	{
	public:
		pipeline_manager(graphics::device* device);

		pipeline* new_pipeline(
			const string& name,
			const renderer::shader_data& vertex_shader,
			const renderer::shader_data& pixel_shader);

		pipeline* get_pipeline(const string& name);
		pipeline* get_scene_pipeline();

		uint64 get_num_pipelines() const;

	private:
		graphics::device* mp_device;
		umap<string, pipeline*> m_pipeline_map;
	};
}