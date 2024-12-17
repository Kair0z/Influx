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
	static const char* k_debug_pipeline_name = "pip_debug";
	struct pipeline_signature;

	class pipeline_manager final
	{
	public:
		pipeline_manager(graphics::device* device);

		pipeline* get_or_create_pipeline(const string& name, const pipeline_signature& key);

		uint64 get_num_pipelines() const;

	private:
		graphics::device* mp_device;
		umap<string, vector<pipeline*>> m_pipeline_map;
	};
}