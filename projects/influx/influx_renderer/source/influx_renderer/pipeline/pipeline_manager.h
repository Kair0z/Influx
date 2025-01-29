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
		detail::tpipeline<_t>* find_pipeline(const string& name, const graphics_pipeline_signature& signature)
		{
			using pipeline_type = detail::tpipeline<_t>;
			const auto& existing_pipelines = get_map<_t>()[name];
			auto found = std::find_if(existing_pipelines.cbegin(), existing_pipelines.cend(), [&signature](const pipeline_type* pip)
			{
				return pip->get_signature() == signature;
			});

			return found != existing_pipelines.cend() ? *found : nullptr;
		}

		template <graphics::e_pipeline_type _t>
		detail::tpipeline<_t>* get_or_create_pipeline(const string& name, const pipeline_signature<_t>& signature)
		{
			auto& backend = renderer_backend::get_instance();

			// first try finding
			detail::tpipeline<_t>* result = find_pipeline<_t>(name, signature);
			if (result)
			{
				return result;
			}

			// try creating
			if constexpr (std::is_same_v<_t, graphics::e_pipeline_type::graphics>)
			{
				const auto& vs_shaders = backend.get_vertex_shaders();
				const auto& ps_shaders = backend.get_pixel_shaders();
				const auto& cs_shaders = backend.get_compute_shaders();
				const bool vertex_shader_found = vs_shaders.contains(signature.m_vs_name);
				const bool pixel_shader_found = ps_shaders.contains(signature.m_ps_name);

				if (!vertex_shader_found || !pixel_shader_found)
				{
					return nullptr;
				}

				result = create_pipeline(
					name, signature,
					&vs_shaders.at(signature.m_vs_name),
					&ps_shaders.at(signature.m_ps_name));
			}
			else if constexpr (std::is_same_v<_t, graphics::e_pipeline_type::compute>)
			{
				const string& shader_name = signature.m_cs_name;
				const auto& shaders = backend.get_compute_shaders();
				const bool shader_found = shaders.contains(shader_name);

				result = create_pipeline(
					name,
					signature,
					&shaders.at(shader_name));
			}
			else if constexpr (std::is_same_v<_t, graphics::e_pipeline_type::raytracing>)
			{
				const string& shader_name = signature.m_cs_name;
				const auto& shaders = backend.get_vertex_shaders();
				const bool shader_found = shaders.contains(shader_name);

				result = create_pipeline(
					name,
					signature,
					&shaders.at(shader_name));
			}

			return result;
		}

		graphics_pipeline* get_or_create_pipeline(const string& name, const graphics_pipeline_signature& signature);
		compute_pipeline* get_or_create_pipeline(const string& name, const compute_pipeline_signature& signature);
		raytracing_pipeline* get_or_create_pipeline(const string& name, const raytracing_pipeline_signature& signature);

	private:
		graphics::device* mp_device;
		umap<string, vector<graphics_pipeline*>> m_graphics_map;
		umap<string, vector<compute_pipeline*>> m_compute_map;
		umap<string, vector<raytracing_pipeline*>> m_raytracing_map;

		graphics_pipeline* create_pipeline(
			const string& name,
			const graphics_pipeline_signature& signature,
			const shader_data& vs,
			const shader_data& ps);

		compute_pipeline* create_pipeline(
			const string& name,
			const compute_pipeline_signature& signature,
			const shader_data& cs);

		raytracing_pipeline* create_pipeline(
			const string& name,
			const raytracing_pipeline_signature& signature,
			const shader_data& rgs);

		template <graphics::e_pipeline_type _t>
		umap<string, vector<detail::tpipeline<_t>*>>& get_map()
		{
			if constexpr (std::is_same_v<_t, graphics::e_pipeline_type::graphics>)
			{
				return m_graphics_map;
			}
			else if constexpr (std::is_same_v<_t, graphics::e_pipeline_type::compute>)
			{
				return m_compute_map;
			}
			else if constexpr (std::is_same_v<_t, graphics::e_pipeline_type::raytracing>)
			{
				return m_raytracing_map;
			}

			return m_graphics_map;
		}

		template <graphics::e_pipeline_type _t>
		const umap<string, vector<detail::tpipeline<_t>*>>& get_map() const
		{
			if constexpr (std::is_same_v<_t, graphics::e_pipeline_type::graphics>)
			{
				return m_graphics_map;
			}
			else if constexpr (std::is_same_v<_t, graphics::e_pipeline_type::compute>)
			{
				return m_compute_map;
			}
			else if constexpr (std::is_same_v<_t, graphics::e_pipeline_type::raytracing>)
			{
				return m_raytracing_map;
			}

			return m_graphics_map;
		}
	};
}