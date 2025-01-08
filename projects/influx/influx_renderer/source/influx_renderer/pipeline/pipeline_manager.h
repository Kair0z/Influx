#pragma once

// influx::core
#include "core/container/map.h"

// influx::graphics
namespace influx::graphics
{
	class device;
}

// influx::renderer
namespace influx::renderer
{
	class pipeline;
}

namespace influx::renderer
{
	struct pipeline_signature;

	class pipeline_manager final
	{
	public:
		pipeline_manager(graphics::device* device);

		pipeline* get_or_create_pipeline(const string& name, const pipeline_signature& key);

		uint32 get_num_pipelines() const;

		bool has_pipeline(const string& name) const;

		pipeline* find_pipeline(const string& name, const pipeline_signature& signature);

	private:
		graphics::device* mp_device;
		umap<string, vector<pipeline*>> m_pipeline_map;

		pipeline* create_pipeline(
			const string& name,
			const pipeline_signature& signature,
			const shader_data* vs,
			const shader_data* ps);
	};
}