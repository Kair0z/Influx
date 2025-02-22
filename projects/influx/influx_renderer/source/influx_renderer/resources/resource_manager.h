#pragma once
#include "core/basetypes.h"
#include "core/time.h"

// influx::renderer
#include "influx_renderer.h"

namespace influx::renderer
{
	enum class e_resource_type
	{
		texturecube,
		texture,
		shader,
		mesh,
		count
	};
	static constexpr uint32 k_num_resource_types = static_cast<uint32>(e_resource_type::count);

	template <e_resource_type _t>
	using resource_data = std::tuple_element_t<static_cast<uint32>(_t), std::tuple<
		texturecube_data,
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
			time::point m_load_time;
		};

		template <e_resource_type _t>
		using resource_map = umap<resource_sign<_t>, entry<_t>>;
		resource_map<e_resource_type::texturecube> m_texturecube_map;
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
			else if constexpr (_t == e_resource_type::texturecube)
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
			else if constexpr (_t == e_resource_type::texturecube)
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
		void load(const resource_sign<_t>& signature, const resource_data<_t>& data)
		{
			auto& map = get_resource_map<_t>();
			if (map.contains(signature))
			{
				// reload? -> overwrite!
				entry<_t>& entry = map.at(signature);
				entry.m_data = data;
				entry.m_signature = signature;
				entry.m_load_time = time::get_now();
			}
			else
			{
				entry<_t> new_entry = entry<_t>(signature, data);
				new_entry.m_load_time = time::get_now();
				map[signature] = new_entry;
			}
		}

		template <e_resource_type _t>
		bool contains(const resource_sign<_t>& signature) const
		{
			const auto& map = get_resource_map<_t>();
			return map.contains(signature);
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