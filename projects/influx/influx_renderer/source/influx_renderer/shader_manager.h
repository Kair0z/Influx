#pragma once

// influx::core
#include "core/shader.h"
#include "core/time.h"

namespace influx::renderer
{
	static constexpr shader::e_shader_target k_min_shader_target = shader::e_shader_target::_6_2;
	static constexpr uint8 k_num_shadermaps = shader::k_num_shadertargets * shader::k_num_shadertypes;

	class shader_map final
	{
	public:
		shader_map() = default;
		shader_map(shader::e_shader_type type, shader::e_shader_target target)
			: m_type{ type }, m_target{ target }{}

		shader::e_shader_type get_type() const { return m_type; }
		shader::e_shader_target get_target() const { return m_target; }

		void load(const shader::shader_signature& signature, const shader_data& data, bool allow_reload)
		{
			if (m_num_times_loaded == 0u)
			{
				m_shaders[signature] = data;
			}
			else if (allow_reload)
			{
				m_shaders[signature] = data;
			}

			++m_num_times_loaded;
		}

		bool has_shader(const shader::shader_signature& sig) const
		{
			return m_shaders.contains(sig);
		}

		shader_data const* get_shader(const shader::shader_signature& sig) const
		{
			if (has_shader(sig))
			{
				return &m_shaders.at(sig);
			}

			return nullptr;
		}

	private:
		umap<shader::shader_signature, shader_data> m_shaders{};
		shader::e_shader_type m_type{};
		shader::e_shader_target m_target{};
		time::point m_last_loaded = time::get_now();
		uint32 m_num_times_loaded = 0u;
	};

	class shader_manager final
	{
	public:
		shader_manager();

		void load(const shader::shader_signature& signature, const shader_data& data, bool allow_reload)
		{
			shader_map& map = get_shadermap(signature.m_type, signature.m_target);
			influx_assert(signature.m_target == map.get_target());
			influx_assert(signature.m_type == map.get_type());

			map.load(signature, data, allow_reload);
		}

		bool has_shader(const shader::shader_signature& signature) const
		{
			const shader::e_shader_target target = signature.m_target;
			const shader::e_shader_type type = signature.m_type;
			return get_shadermap(type, target).has_shader(signature);
		}

		time::point get_shader_load_timepoint(const shader::shader_signature& signature) const
		{
			return time::get_now();
		}

		const shader_map& get_shadermap(shader::e_shader_type type, shader::e_shader_target target = k_min_shader_target) const
		{
			return m_shadermaps[get_shadermap_index(type, target)];
		}

		shader_map& get_shadermap(shader::e_shader_type type, shader::e_shader_target target = k_min_shader_target)
		{
			return m_shadermaps[get_shadermap_index(type, target)];
		}

	private:
		shader_map m_shadermaps[k_num_shadermaps];

		static uint8 get_shadermap_index(const shader::e_shader_type type, const shader::e_shader_target target)
		{
			return (static_cast<uint8>(target) * shader::k_num_shadertypes) + static_cast<uint8>(type);
		}

		static shader::e_shader_type get_shadertype(uint8 index)
		{
			return static_cast<shader::e_shader_type>(index % shader::k_num_shadertypes);
		}

		static shader::e_shader_target get_shadertarget(uint8 index)
		{
			return static_cast<shader::e_shader_target>(index / shader::k_num_shadertypes);
		}
	};
}