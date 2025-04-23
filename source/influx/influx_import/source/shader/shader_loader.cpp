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
		influx_assert(args.m_pbd == false || file::exists(args.m_pdb_folder));

		shader::compile_args args_copy = args;
		args_copy.m_signature.m_filename = file(filepath).m_filename_without_extension;

		auto compile_result = shader::compile_shader_in_file(filepath, args_copy);
		influx_assert(compile_result.is_success());

		out_shader.m_compile_result = compile_result.get();
		out_shader.m_type = args.m_signature.m_type;
		out_shader.m_signature = args.m_signature;
		return true;
	}

	/* loads a shader file (.hlsl) and extracts all valid compiled shaders it can find */
	bool load_shader_file(const string& filepath, vector<shader_data>& out_shaders, const shader::compile_args& args)
	{
		influx_assert(file::exists(filepath));
		influx_assert(file::exists(args.m_include_folder));
		influx_assert(args.m_pbd == false || file::exists(args.m_pdb_folder));

		auto parsed_file = shader::parse_shaders_in_file(filepath);
		influx_assert(parsed_file.is_success());

		vector<shader::parse_output>& parsed_shaders = parsed_file.get();

		out_shaders.clear();
		out_shaders.reserve(parsed_shaders.size());

		for (shader::parse_output& parsed_shader : parsed_shaders)
		{
			shader::compile_args args_copy = args;
			parsed_shader.m_signature.m_target = args.m_signature.m_target;
			args_copy.m_signature = parsed_shader.m_signature;

			shader_data new_shader_data{};
			if (load_shader_file(filepath, new_shader_data, args_copy))
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