#pragma once
#include "core/basetypes.h"
#include "core/time.h"

// influx::renderer
#include "influx_renderer.h"
#include "influx_renderer/upload_manager.h"
#include "influx_renderer/mesh.h"
#include "influx_renderer/renderer_backend.h"
#include "influx_renderer/descriptor_manager.h"
#include "influx_renderer/resource.h"

namespace influx::graphics
{
	class resource;
}

namespace influx::renderer
{
	class resource_manager final
	{
	public:
		resource_manager();
		~resource_manager();

		void load_internal_resources();

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
			debug_name m_debugname;
		};

		template <e_resource_type _t>
		using resource_map = umap<resource_sign<_t>, entry<_t>>;

	private:
		resource_map<e_resource_type::cubemap> m_texturecube_map;
		resource_map<e_resource_type::texture2D> m_texture2D_map;
		resource_map<e_resource_type::texture3D> m_texture3D_map;
		resource_map<e_resource_type::shader> m_shader_map;
		resource_map<e_resource_type::mesh> m_mesh_map;

		template <e_resource_type _t>
		resource_map<_t>& get_resource_map()
		{
			if constexpr (_t == e_resource_type::texture2D)
			{
				return m_texture2D_map;
			}
			else if constexpr (_t == e_resource_type::texture3D)
			{
				return m_texture3D_map;
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
			if constexpr (_t == e_resource_type::texture2D)
			{
				return m_texture2D_map;
			}
			else if constexpr (_t == e_resource_type::texture3D)
			{
				return m_texture3D_map;
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

				// recreate stuff
				if constexpr (_t == e_resource_type::cubemap)
				{
					recreate_cubemap(signature, data);
				}
				else if constexpr (_t == e_resource_type::texture2D)
				{
					recreate_texture2D(signature, data);
				}
				else if constexpr (_t == e_resource_type::texture3D)
				{
					recreate_texture3D(signature, data);
				}
				else if constexpr (_t == e_resource_type::mesh)
				{
					recreate_mesh(signature, data);
				}
				else if constexpr (_t == e_resource_type::shader)
				{
					recreate_shader(signature, data);
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
		entry<_t> const* get(const resource_sign<_t>& signature) const
		{
			const auto& map = get_resource_map<_t>();
			if (map.contains(signature))
			{
				return &map.at(signature);
			}
			else return nullptr;
		}

		template <e_resource_type _t>
		const entry<_t>& get_default() const
		{
			return get<_t>({});
		}

		template <e_resource_type _t>
		const debug_name& get_debugname(const resource_sign<_t>& signature) const
		{
			const auto& map = get_resource_map<_t>();
			if (map.contains(signature))
			{
				map.at(signature).m_debugname;
			}
			return {};
		}

		template <e_resource_type _t>
		vector<resource_sign<_t>> get_all_signatures() const
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
		vector<debug_name> get_all_debugnames() const
		{
			const auto& map = get_resource_map<_t>();

			vector<debug_name> result{};
			result.reserve(map.size());
			for (const auto& pair : map)
			{
				result.push_back(pair.second.m_debugname);
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

	private:
		void recreate_mesh(const mesh_id& id, detail::base_mesh_data const* data);
		void recreate_texture2D(const tex_id& id, const texture2D_data& data);
		void recreate_texture3D(const tex_id& id, const texture3D_data& data);
		void recreate_cubemap(const cubemap_id& id, const cubemap_data& data);
		void recreate_shader(const shader_id& sig, const shader_data& data);
	};

	using mesh_resource = resource_manager::entry<e_resource_type::mesh>;
	using texture2D_resource = resource_manager::entry<e_resource_type::texture2D>;
	using texture3D_resource = resource_manager::entry<e_resource_type::texture2D>;
}