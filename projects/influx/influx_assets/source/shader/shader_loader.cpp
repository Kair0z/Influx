#include "assets_pch.h"
#include "influx_assets.h"

namespace influx::assets
{
	/* Loads a Shader file (.hlsl) */
	bool load_shader_file(const string& filepath, shader_data& out_shader, const shader::compile_args& args)
	{
		out_shader.m_compile_result = shader::compile_shader(filepath, args);
		out_shader.m_type = args.m_type;
		return true;
	}
}