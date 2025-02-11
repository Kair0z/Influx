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
		graphics::device* mp_device;

		template <graphics::e_pipeline_type _t>
		using pipeline_map = umap<pipeline_signature<_t>, pipeline<_t> >;
		pipeline_map<graphics::e_pipeline_type::graphics> m_graphics_map;
		pipeline_map<graphics::e_pipeline_type::compute> m_compute_map;
		pipeline_map<graphics::e_pipeline_type::raytracing>	m_raytracing_map;

	public:
		pipeline_manager(graphics::device* device);

		uint32 get_num_pipelines() const;

		template <graphics::e_pipeline_type _t>
		bool has_pipeline(const pipeline_signature<_t>& signature) const
		{
			return get_map<_t>().contains(signature);
		}

		template <graphics::e_pipeline_type _t>
		pipeline<_t>* find_pipeline(const pipeline_signature<_t>& signature)
		{
			if (has_pipeline(signature))
			{
				return get_map<_t>[signature];
			}

			return nullptr;
		}

		template <graphics::e_pipeline_type _t>
		pipeline<_t>& get_or_create_pipeline(const pipeline_signature<_t>& signature)
		{
			influx_assert(mp_device != nullptr);
			influx_assert(signature.is_valid());

			pipeline_map<_t>& map = get_map<_t>();
			if (!has_pipeline<_t>(signature))
			{
				map[signature] = pipeline<_t>(*mp_device, signature);
			}

			return map[signature];
		}

		graphics_pipeline& get_or_create_pipeline(const graphics_pipeline_signature& signature)
		{
			return get_or_create_pipeline<graphics::e_pipeline_type::graphics>(signature);
		}

		compute_pipeline& get_or_create_pipeline(const compute_pipeline_signature& signature)
		{
			return get_or_create_pipeline<graphics::e_pipeline_type::compute>(signature);
		}

		raytracing_pipeline& get_or_create_pipeline(const raytracing_pipeline_signature& signature)
		{
			return get_or_create_pipeline<graphics::e_pipeline_type::raytracing>(signature);
		}

	private:
		template <graphics::e_pipeline_type _t>
		pipeline_map<_t>& get_map()
		{
			if constexpr (_t == graphics::e_pipeline_type::graphics)
			{
				return m_graphics_map;
			}
			else if constexpr (_t == graphics::e_pipeline_type::compute)
			{
				return m_compute_map;
			}
			else if constexpr (_t == graphics::e_pipeline_type::raytracing)
			{
				return m_raytracing_map;
			}
		}

		template <graphics::e_pipeline_type _t>
		const pipeline_map<_t>& get_map() const
		{
			if constexpr (_t == graphics::e_pipeline_type::graphics)
			{
				return m_graphics_map;
			}
			else if constexpr (_t == graphics::e_pipeline_type::compute)
			{
				return m_compute_map;
			}
			else if constexpr (_t == graphics::e_pipeline_type::raytracing)
			{
				return m_raytracing_map;
			}
		}
	};
}