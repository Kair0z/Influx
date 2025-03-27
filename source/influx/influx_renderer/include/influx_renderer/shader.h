#pragma once

// influx::renderer
#include "influx_renderer/types.h"

// influx::shader
#include "influx_shader.h"

namespace influx::renderer
{
	// compile arguments used by the renderer internal to compile its native shaders
	static inline shader::compile_args get_internal_default_compile_args()
	{
		static shader::compile_args compile_args{};
		compile_args.m_signature.m_target = shader::e_shader_target::_6_6;
		compile_args.m_reflection = true;
		compile_args.m_defines = {};
		compile_args.m_compile_debug = false;
		compile_args.m_pbd = false;
		return compile_args;
	}

	// the renderer backend stores shaders as slimmed down versions of shader::compile_output
	using shader_signature = shader::shader_signature;
	struct shader_data final
	{
		// convert shader::compile_output to renderer::shader_data
		INFLUX_RENDER_API
		static shader_data translate(const shader::compile_output& compile_output);

		shader::e_shader_type	m_type;
		shader::reflection		m_reflection;
		vector<byte>			m_bytecode;
		time::point				m_time_loaded;
		uint32					m_num_times_loaded = 0u;

		inline bool is_newer_than(const time::point& timepoint) const
		{
			return m_time_loaded > timepoint;
		}

		inline bool is_valid() const
		{
			return m_bytecode.empty() == false;
		}
	};

}