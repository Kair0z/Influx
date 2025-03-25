#include "import_pch.h"
#include "influx_import.h"

#include "core/regex.h"
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

		// parse out potential shader types
		static const char* type_to_signature[shader::k_num_shadertypes] =
		{
			R"(\[shader\(\"vertex\"\)\])",
			R"(\[shader\(\"pixel\"\)\])",
			R"(\[shader\(\"domain\"\)\])",
			R"(\[shader\(\"geometry\"\)\])",
			R"(\[shader\(\"hull\"\)\])",

			R"(\[shader\(\"compute\"\)\])",

			R"(\[shader\(\"raygen\"\)\])",
			R"(\[shader\(\"miss\"\)\])",
			R"(\[shader\(\"closesthit\"\)\])",
			R"(\[shader\(\"anyhit\"\)\])",
			R"(\[shader\(\"intersection\"\)\])"
		};

		// gather all shader compiles
		vector<shader::compile_args> per_shader_compile_args{};
		vector<string> file_lines = file::get_lines(filepath, 0u);
		for (uint32 i = 0u; i < shader::k_num_shadertypes; ++i)
		{
			for (uint32 l = 0u; l < file_lines.size(); ++l)
			{
				const string& line = file_lines[l];
				influx::regex::for_each_match(line, type_to_signature[i], 
					[&args, i, &file_lines, &per_shader_compile_args, l](const string& str)
				{
					// we matched '[shader("anyhit")]', now figure out the function entrypoint name:
					string next_line = file_lines[l + 1];

					vector<string> entrypoints = regex::get_all_matches(next_line, R"(\b\w+\s+(\w+)\()");
					if (entrypoints.size() > 0 && entrypoints[0].empty() == false)
					{
						shader::compile_args individual_args = args;
						individual_args.m_signature.m_type = static_cast<shader::e_shader_type>(i);
						individual_args.m_signature.m_entrypoint = entrypoints[0];
						per_shader_compile_args.push_back(individual_args);
					}
				});
			}
		}

		out_shaders.clear();
		out_shaders.reserve(per_shader_compile_args.size());
		for (const shader::compile_args& single_args : per_shader_compile_args)
		{
			shader_data new_shader_data{};
			if (load_shader_file(filepath, new_shader_data, single_args))
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