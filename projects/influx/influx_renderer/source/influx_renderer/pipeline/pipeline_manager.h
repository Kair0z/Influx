#pragma once

// influx::core
#include "core/container/map.h"

// influx::graphics
namespace influx::graphics
{
	class device;
}

// influx::renderer
#include "pipeline.h"

namespace influx::renderer
{
	class pipeline_manager final
	{
	public:
		pipeline_manager(graphics::device* device);

		uint32 get_num_pipelines() const;

		template <graphics::e_pipeline_type _t>
		bool has_pipeline(const string& name) const
		{
			return get_map<_t>().contains(name);
		}

		template <graphics::e_pipeline_type _t>
		detail::tpipeline<_t>* find_pipeline(const string& name, const pipeline_signature<_t>& signature)
		{
			using pipeline_type = detail::tpipeline<_t>;
			const auto& existing_pipelines = get_map<_t>()[name];
			auto found = std::find_if(existing_pipelines.cbegin(), existing_pipelines.cend(), [&signature](const pipeline_type* pip)
			{
				return static_cast<detail::tpipeline<_t>*>(pip)->get_signature() == signature;
			});

			return found != existing_pipelines.cend() ? static_cast<detail::tpipeline<_t>*>(*found) : nullptr;
		}

		template <graphics::e_pipeline_type _t>
		detail::tpipeline<_t>* get_or_create_pipeline(const string& name, const pipeline_signature<_t>& signature)
		{
			auto& backend = renderer_backend::get_instance();

			// first try finding
			if (detail::tpipeline<_t>* result = find_pipeline<_t>(name, signature))
			{
				return result;
			}
			else
			{
				// if finding failed, create
				return new detail::tpipeline<_t>(*mp_device, signature);
			}
		}

		graphics_pipeline* get_or_create_pipeline(const string& name, const graphics_pipeline_signature& signature)
		{
			return get_or_create_pipeline<graphics::e_pipeline_type::graphics>(name, signature);
		}

		compute_pipeline* get_or_create_pipeline(const string& name, const compute_pipeline_signature& signature)
		{
			return get_or_create_pipeline<graphics::e_pipeline_type::compute>(name, signature);
		}

		raytracing_pipeline* get_or_create_pipeline(const string& name, const raytracing_pipeline_signature& signature)
		{
			return get_or_create_pipeline<graphics::e_pipeline_type::raytracing>(name, signature);
		}

	private:
		graphics::device* mp_device;

		// detail::tpipeline<graphics::e_pipeline_type::graphics> = graphics_pipeline
		umap<string, vector<pipeline*>> m_maps[graphics::e_pipeline_type::count];

		template <graphics::e_pipeline_type _t>
		umap<string, vector<pipeline*>>& get_map()
		{
			return m_maps[int(_t)];
		}

		template <graphics::e_pipeline_type _t>
		const umap<string, vector<pipeline*>>& get_map() const
		{
			return m_maps[int(_t)];
		}
	};
}