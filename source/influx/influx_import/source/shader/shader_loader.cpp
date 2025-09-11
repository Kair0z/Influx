#include "import_pch.h"
#include "influx_import.h"

#include "core/file.h"

namespace influx::imp
{
	/* loads a shader file (.hlsl) and extracts all valid compiled shaders it can find */
	result<vector<shader_data>> load_shaders_in_file(const string& filepath, const shader_load_args& load_args)
	{
		using result_type = result<vector<shader_data>>;

		if (!path::exists(filepath))
			return result_type::make_error("in filepath doesn't exist!");

		const shader::compile_args& compile_args = load_args.m_compile_args;
		// if (!path::exists(compile_args.m_include_folder))
		// 	return result_type::make_error("m_include_folder doesn't exist!");

		const bool valid_pbd_path = path::exists(compile_args.m_pdb_folder) && !compile_args.m_pdb_filename.empty();
		if (compile_args.m_pbd_enabled && valid_pbd_path == false)
			return result_type::make_error("m_pbd_enabled is true but the input pbd filepath is invalid!");

		// parse each shader in file
		auto parsed_file = shader::parse_shaders_in_file(filepath);
		if (parsed_file.is_unex())
			return result_type::make_error("shader::parse_shaders_in_file(filepath) failed!");
		
		shader::parse_output& parsed_shaders = parsed_file.get();

		// for each parsed shader, compile it and add it to the list	
		vector<shader_data> out_shaders{};
		out_shaders.reserve(parsed_shaders.get_total_num_shaders());
		for (auto& pair : parsed_shaders.m_shadermap)
			for (shader::parse_output::per_shader& parsed_shader : pair.second)
			{
				auto new_shader_data = shader::compile_shader_in_file(filepath, parsed_shader.m_signature, compile_args);
				if (new_shader_data.is_unex()) 
					return result_type::make_error("one of the compile_shaders failed!");

				shader_data data{};
				data.m_compile_result = new_shader_data.get();
				data.m_signature = data.m_compile_result.m_signature;
				data.m_type = parsed_shader.m_signature.m_type;
				out_shaders.push_back(data);
			}

		return out_shaders;
	}
}