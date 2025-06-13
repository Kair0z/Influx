#include "import_pch.h"
#include "influx_import.h"

#include "core/file.h"

namespace influx::imp
{
	/* Loads a Shader file (.hlsl) */
	result<shader_data> load_shader_file(const string& filepath, const shader_load_args& load_args)
	{
		using result_type = result<shader_data>;

		if (!file::exists(filepath)) 
			return result_type::make_error("in filepath doesn't exist!");

		const shader::compile_args& compile_args = load_args.m_compile_args;
		if (!file::exists(compile_args.m_include_folder))
			return result_type::make_error("args.m_include_folder doesn't exist!");

		const bool valid_pbd_path = file::exists(compile_args.m_pdb_folder) && !compile_args.m_pdb_filename.empty();
		if (compile_args.m_pbd_enabled && valid_pbd_path == false)
			return result_type::make_error("args.m_pbd_enabled is true but the input pbd filepath is invalid!");

		shader::compile_args args_copy = compile_args;
		args_copy.m_signature.m_filename = file(filepath).m_filename_without_extension;

		auto compile_result = shader::compile_shader_in_file(filepath, args_copy);
		influx_assert(compile_result.is_success());

		shader_data out_shader{};
		out_shader.m_compile_result = compile_result.get();
		out_shader.m_type = compile_args.m_signature.m_type;
		out_shader.m_signature = compile_args.m_signature;
		return out_shader;
	}

	/* loads a shader file (.hlsl) and extracts all valid compiled shaders it can find */
	result<vector<shader_data>> load_shaders_in_file(const string& filepath, const shader_load_args& load_args)
	{
		using result_type = result<vector<shader_data>>;

		if (!file::exists(filepath)) 
			return result_type::make_error("in filepath doesn't exist!");

		const shader::compile_args& compile_args = load_args.m_compile_args;
		if (!file::exists(compile_args.m_include_folder))
			return result_type::make_error("args.m_include_folder doesn't exist!");

		const bool valid_pbd_path = file::exists(compile_args.m_pdb_folder) && !compile_args.m_pdb_filename.empty();
		if (compile_args.m_pbd_enabled && valid_pbd_path == false)
			return result_type::make_error("args.m_pbd_enabled is true but the input pbd filepath is invalid!");

		// parse all shaders in file
		auto parsed_file = shader::parse_shaders_in_file(filepath);
		if (parsed_file.is_unex())
			return result_type::make_error("shader::parse_shaders_in_file(filepath) failed!");

		vector<shader::parse_output>& parsed_shaders = parsed_file.get();

		// for each parsed shader, load it and add it to the list		
		vector<shader_data> out_shaders{};
		out_shaders.reserve(parsed_shaders.size());
		for (shader::parse_output& parsed_shader : parsed_shaders)
		{
			shader::compile_args args_copy = compile_args;
			parsed_shader.m_signature.m_target = compile_args.m_signature.m_target;
			args_copy.m_signature = parsed_shader.m_signature;

			result<shader_data> new_shader_data = load_shader_file(filepath, { args_copy });
			if (new_shader_data.is_unex())
				return result_type::make_error("one of the load_shader's failed!");
			else
			{
				out_shaders.push_back(new_shader_data.get());
			}
		}

		return out_shaders;
	}
}