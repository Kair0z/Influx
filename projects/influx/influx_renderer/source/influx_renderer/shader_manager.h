#pragma once

// influx::core
#include "core/shader.h"
#include "core/time.h"

namespace influx::renderer
{
	static constexpr shader::e_shader_target k_min_shader_target = shader::e_shader_target::_6_2;
	static constexpr uint8 k_num_shadermaps = shader::k_num_shadertargets * shader::k_num_shadertypes;

	namespace detail
	{
		class base_shader_map
		{
		public:
			virtual shader::e_shader_type get_type() const = 0;
			virtual shader::e_shader_target get_target() const = 0;
			virtual void load(const shader::shader_signature& signature, bool allow_reload = true) = 0;
			virtual bool has_shader(const shader::shader_signature& sig) const = 0;
			virtual shader_data* get_shader(const shader::shader_signature& sig) const = 0;

		protected:
			time::point m_last_loaded = time::get_now();
			uint32 m_num_times_loaded = 0u;
		};
	}

	template <shader::e_shader_type _type, shader::e_shader_target _target>
	class shader_map final : public detail::base_shader_map
	{
	public:
		virtual shader::e_shader_type get_type() const override { return _type; };
		virtual shader::e_shader_target get_target() const override { return _target; }
		
		virtual void load(const shader::shader_signature& signature, bool allow_reload) override
		{
			if (m_num_times_loaded == 0u)
			{
				// first load
				m_shaders[signature];
			}
			else if (allow_reload)
			{
				// reload
				
			}

			++m_num_times_loaded;
		}

		virtual bool has_shader(const shader::shader_signature& sig) const override
		{
			return m_shaders.contains(sig);
		}

		shader_data* get_shader(const shader::shader_signature& sig) const override
		{
			if (has_shader(sig))
			{
				return &m_shaders.at(sig);
			}
			return nullptr;
		}

	private:
		umap<shader::shader_signature, shader_data> m_shaders{};
	};

	class shader_manager final
	{
	public:
		shader_manager();

		void load(const shader::shader_signature& signature, bool allow_reload)
		{
			get_shadermap(signature.m_type, signature.m_target).load(signature, allow_reload);
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

		const detail::base_shader_map& get_shadermap(shader::e_shader_type type, shader::e_shader_target target = k_min_shader_target) const
		{
			return *m_shadermaps[get_shadermap_index(type, target)];
		}

		detail::base_shader_map& get_shadermap(shader::e_shader_type type, shader::e_shader_target target = k_min_shader_target)
		{
			return *m_shadermaps[get_shadermap_index(type, target)];
		}

	private:
		detail::base_shader_map* m_shadermaps[k_num_shadermaps]{};

		static uint8 get_shadermap_index(const shader::e_shader_type type, const shader::e_shader_target target)
		{
			return (static_cast<uint8>(target) * shader::k_num_shadertargets) + static_cast<uint8>(type);
		}

		static shader::e_shader_type get_shadertype(uint8 index)
		{
			return static_cast<shader::e_shader_type>(index % shader::k_num_shadertypes);
		}

		static shader::e_shader_target get_shadertarget(uint8 index)
		{
			return static_cast<shader::e_shader_target>(index / shader::k_num_shadertargets);
		}
	};
}