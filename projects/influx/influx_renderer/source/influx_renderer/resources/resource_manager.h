#pragma once
#include "core/basetypes.h"
#include "core/time.h"

// influx::renderer
#include "influx_renderer.h"
#include "influx_renderer/upload_manager.h"
#include "influx_renderer/mesh.h"
#include "influx_renderer/renderer_backend.h"

namespace influx::graphics
{
	class resource;
}

namespace influx::renderer
{
	enum class e_resource_type
	{
		cubemap,
		texture,
		shader,
		mesh,
		count
	};
	static constexpr uint32 k_num_resource_types = static_cast<uint32>(e_resource_type::count);

	struct mesh_buffers final
	{
		graphics::resource* m_vertexbuffer;
		graphics::resource* m_indexbuffer;
	};

	template <e_resource_type _t>
	using resource_data = std::tuple_element_t<static_cast<uint32>(_t), std::tuple<
		cubemap_data,
		texture_data,
		shader_data,
		mesh_data
		>>;

	template <e_resource_type _t>
	using resource_sign = std::tuple_element_t<static_cast<uint32>(_t), std::tuple<
		string,
		string,
		shader::shader_signature,
		string
		>>;

	template <e_resource_type _t>
	using resource_type = std::tuple_element_t<static_cast<uint32>(_t), std::tuple<
		cubemap,
		texture2D,
		void,
		mesh_buffers
		>>;

	class resource_manager final
	{
		template <e_resource_type _t>
		struct entry final
		{
			entry() = default;
			explicit entry(const resource_sign<_t>& signature, const resource_data<_t>& data)
				: m_data{ data }, m_signature{ signature }{}

			resource_data<_t> m_data;
			resource_sign<_t> m_signature;
			resource_type<_t>* m_resource = nullptr;
			time::point m_load_time;
		};

		template <e_resource_type _t>
		using resource_map = umap<resource_sign<_t>, entry<_t>>;
		resource_map<e_resource_type::cubemap> m_texturecube_map;
		resource_map<e_resource_type::texture> m_texture_map;
		resource_map<e_resource_type::shader> m_shader_map;
		resource_map<e_resource_type::mesh> m_mesh_map;

		template <e_resource_type _t>
		resource_map<_t>& get_resource_map()
		{
			if constexpr (_t == e_resource_type::texture)
			{
				return m_texture_map;
			}
			else if constexpr (_t == e_resource_type::cubemap)
			{
				return m_texturecube_map;
			}
			else if constexpr (_t == e_resource_type::shader)
			{
				return m_shader_map;
			}
			else if constexpr (_t == e_resource_type::mesh)
			{
				return m_mesh_map;
			}
		}
		template <e_resource_type _t>
		const resource_map<_t>& get_resource_map() const
		{
			if constexpr (_t == e_resource_type::texture)
			{
				return m_texture_map;
			}
			else if constexpr (_t == e_resource_type::cubemap)
			{
				return m_texturecube_map;
			}
			else if constexpr (_t == e_resource_type::shader)
			{
				return m_shader_map;
			}
			else if constexpr (_t == e_resource_type::mesh)
			{
				return m_mesh_map;
			}
		}

	public:
		template <e_resource_type _t>
		void load(const resource_sign<_t>& signature, const resource_data<_t>& data, bool reload = false)
		{
			auto& map = get_resource_map<_t>();
			const bool is_recreate = !map.contains(signature) || reload;
			if (is_recreate)
			{
				// reload? -> overwrite!
				entry<_t>& entry = map[signature];
				entry.m_data = data;
				entry.m_signature = signature;
				entry.m_load_time = time::get_now();

				// 
				renderer_backend& backend = renderer_backend::get_instance();
				graphics::device& device = renderer_backend::get_device();
				upload_manager& uploadman = *renderer_backend::get_upload_manager();
				graphics::queue& queue = renderer_backend::get_graphics_queue();

				if constexpr (_t == e_resource_type::cubemap)
				{
					cubemap*& resource = map[signature].m_resource;
					if (resource == nullptr)
					{
						cubemap_desc create_args{};
						create_args.m_width = data.get_width();
						create_args.m_heigth = data.get_height();
						create_args.m_depth = data.get_depth();
						resource = new cubemap(&device, create_args);

						graphics::commandlist& commandlist = *device.create_graphics_commandlist();
						commandlist.start(&device);
						resource->upload(commandlist, data);
						commandlist.end();
						commandlist.submit(&queue);
						commandlist.wait_for_completion();
					}
				}
				else if constexpr (_t == e_resource_type::texture)
				{
					texture2D*& resource = map[signature].m_resource;
					texture_desc create_args{};
					create_args.m_width = data.get_width();
					create_args.m_heigth = data.get_height();
					resource = new texture2D(&device, create_args);
					uploadman.upload_texture(&queue, data, resource->get_resource());
				}
				else if constexpr (_t == e_resource_type::mesh)
				{
					mesh_buffers*& meshbuffers = map[signature].m_resource;
					meshbuffers = new mesh_buffers();
					meshbuffers->m_indexbuffer = backend.create_indexbuffer(signature, data.m_indices);
					meshbuffers->m_vertexbuffer = backend.create_vertexbuffer(signature, data.m_vertices);
				}
			}
		}

		template <e_resource_type _t>
		bool contains(const resource_sign<_t>& signature) const
		{
			const auto& map = get_resource_map<_t>();
			return map.contains(signature);
		}

		template <e_resource_type _t>
		const entry<_t>& get(const resource_sign<_t>& signature) const
		{
			const auto& map = get_resource_map<_t>();
			return map.at(signature);
		}

		template <e_resource_type _t>
		entry<_t>& get(const resource_sign<_t>& signature)
		{
			auto& map = get_resource_map<_t>();
			return map[signature];
		}

		template <e_resource_type _t>
		time::point get_time_loaded(const resource_sign<_t>& signature) const
		{
			auto& map = get_resource_map<_t>();
			if (map.contains(signature))
			{
				return map.at(signature).m_load_time;
			}
			else
			{
				return time::get_now();
			}
		}
	};
}