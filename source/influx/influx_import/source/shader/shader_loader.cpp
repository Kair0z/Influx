#include "import_pch.h"
#include "influx_import.h"

#include "core/file.h"

namespace influx::imp
{
	/* Loads a Shader file (.hlsl) */
	bool load_shader_file(const string& filepath, shader_data& out_shader, const shader::compile_args& args)
	{
		influx_assert(file::exists(filepath));
		influx_assert(file::exists(args.m_include_folder));
		influx_assert(file::exists(args.m_pdb_folder));

		shader::compile_args args_copy = args;
		args_copy.m_signature.m_filename = file(filepath).m_filename_without_extension;

		out_shader.m_compile_result = shader::compile_shader(filepath, args_copy);
		out_shader.m_type = args.m_signature.m_type;
		out_shader.m_signature = args.m_signature;
		return true;
	}

	bool load_shader_file(const string& filepath, vector<shader_data>& out_shaders, const shader::compile_args& args)
	{
		influx_assert(file::exists(filepath));
		influx_assert(file::exists(args.m_include_folder));
		influx_assert(file::exists(args.m_pdb_folder));

		shader::parse_output parsed_file = shader::parse_shaderfile(filepath, args);
		
		out_shaders.clear();
		out_shaders.reserve(parsed_file.m_shaders.size());

		for (const shader::parse_output::per_shader& parsed_shader : parsed_file.m_shaders)
		{
			shader_data new_shader_data{};
			if (load_shader_file(filepath, new_shader_data, parsed_shader.m_compile_args))
			{
				out_shaders.push_back(new_shader_data);
			}
			else
			{
				return false;
			}
		}

		return true;
	}
}