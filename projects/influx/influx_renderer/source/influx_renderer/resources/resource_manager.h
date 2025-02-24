#pragma once
#include "core/basetypes.h"
#include "core/time.h"

// influx::renderer
#include "influx_renderer.h"
#include "influx_renderer/upload_manager.h"
#include "influx_renderer/mesh.h"
#include "influx_renderer/renderer_backend.h"
#include "influx_renderer/descriptor_manager.h"

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

#pragma region typedefs
	// resource-data: the user input data struct matching the resource type
	template <e_resource_type _t>
	using resource_data = std::tuple_element_t<static_cast<uint32>(_t), std::tuple<
		cubemap_data,
		texture_data,
		shader_data,
		detail::base_mesh_data*
		>>;

	// resource-signature: the unique signature struct used as the key for the map
	template <e_resource_type _t>
	using resource_sign = std::tuple_element_t<static_cast<uint32>(_t), std::tuple<
		string,
		string,
		shader::shader_signature,
		string
		>>;

	// resource-type: the graphics::resource objects matching the resource type
	template <e_resource_type _t>
	using resource_type = std::tuple_element_t<static_cast<uint32>(_t), std::tuple<
		cubemap,
		texture2D,
		void,
		mesh_buffers
		>>;
#pragma endregion

	class resource_manager final
	{
	public:
		resource_manager();
		~resource_manager();

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

	private:
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
		entry<_t>& load(
			const resource_sign<_t>& signature, 
			const resource_data<_t>& data, bool reload = false)
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
						resource->m_srv = renderer_backend::get_descriptor_manager()->create_srv(resource->mp_resource);

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

					resource->m_srv = renderer_backend::get_descriptor_manager()->create_srv(resource->mp_resource);
					uploadman.upload_texture(&queue, data, resource->get_resource());
				}
				else if constexpr (_t == e_resource_type::mesh)
				{
					detail::base_mesh_data* mesh_data = data;
					mesh_buffers*& meshbuffers = map[signature].m_resource;
					if (meshbuffers == nullptr) meshbuffers = new mesh_buffers();

					// vertex buffer
					{
						const uint64 old_bytesize = meshbuffers->m_vertexbuffer ? meshbuffers->m_vertexbuffer->get_bytesize() : 0u;
						const uint64 new_bytesize = mesh_data->get_vert_bytesize();
						if (old_bytesize < new_bytesize)
						{
							// destroy old resource
							if (meshbuffers->m_vertexbuffer)
								device.release(meshbuffers->m_vertexbuffer);

							// create new vertex buffer on the shared heap
							graphics::heap_desc heap_desc{};
							heap_desc.m_type = graphics::e_heap_type::shared;
							graphics::buffer_desc desc{};
							desc.m_init_state = graphics::e_resource_state::gen_read;

							// create resource
							desc.m_bytesize = new_bytesize;
							desc.m_bytestride = mesh_data->get_vert_bytestride();
							meshbuffers->m_vertexbuffer = device.create_resource(desc, heap_desc);
							meshbuffers->m_vertexbuffer->set_name("vb_" + signature);
						}

						// map new data to resource
						meshbuffers->m_vertexbuffer->map([mesh_data, new_bytesize](void* target)
						{
							memcpy(target, mesh_data->get_vert_data(), new_bytesize);
						});
					}
					// index buffer
					{
						const uint64 old_bytesize = meshbuffers->m_indexbuffer ? meshbuffers->m_indexbuffer->get_bytesize() : 0u;
						const uint64 new_bytesize = mesh_data->get_indx_bytesize();
						if (old_bytesize < new_bytesize)
						{
							// create index / vertex buffer on the shared heap (so cpu can write to it)
							graphics::heap_desc heap_desc{};
							heap_desc.m_type = graphics::e_heap_type::shared;
							graphics::buffer_desc desc{};
							desc.m_init_state = graphics::e_resource_state::gen_read;

							// create index buffer resource
							desc.m_bytesize = new_bytesize;
							desc.m_bytestride = mesh_data->get_indx_bytestride();
							desc.m_format = graphics::e_format::u32;
							meshbuffers->m_indexbuffer = device.create_resource(desc, heap_desc);
							meshbuffers->m_indexbuffer->set_name("ib_" + signature);
						}

						// map content regardless
						if (old_bytesize < new_bytesize || reload)
						{
							meshbuffers->m_indexbuffer->map([mesh_data, new_bytesize](void* target)
							{
								memcpy(target, mesh_data->get_indx_data(), new_bytesize);
							});
						}
					}
				}
			}

			return map[signature];
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
		vector<resource_sign<_t>> get_signatures() const
		{
			const auto& map = get_resource_map<_t>();

			vector<resource_sign<_t>> result{};
			result.reserve(map.size());
			
			for (const auto& pair : map)
			{
				result.push_back(pair.first);
			}

			return result;
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

	using mesh_resource = resource_manager::entry<e_resource_type::mesh>;
	using texture_resource = resource_manager::entry<e_resource_type::texture>;
}