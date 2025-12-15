// influx::core
#include "core/basetypes.h"
#include "core/log.h"
#include "core/commandline.h"
#include "core/file.h"

// influx::shader
#include "influx_shader.h"

// STL
#include <fstream>

static const char* k_invalid_path = "";
static const char* k_default_includes = "";

// required parameters:
influx::cvar cv_filepath		("cv_inputpath",	k_invalid_path, "required: filepath of the shader");
influx::cvar cv_output_filepath ("cv_outputpath",	k_invalid_path, "required: output filepath");
// optional parameters:
influx::cvar cv_entrypoint		("cv_entry", "main_cs", "entrypoint of the shader, if not set, we'll pick the first shader in the file...");
influx::cvar cv_includes		("cv_includes", k_default_includes, "folder of included files");
influx::cvar cv_num_shaders		("cv_num", "3", "haha");

enum e_result : int
{
	success							= +0,
	error_no_input_filepath			= -1,
	error_input_filepath_nfound		= -2,
	error_entrypoint_not_found		= -3,
	error_failed_parse				= -4,
	error_parse_is_empty			= -5,
	error_compile_error				= -6,
	error_no_output_filepath		= -7,
	error_output_write_failed		= -8
};

int main(int argc, char** argv)
{
	using namespace influx;
	cvar::parse_runargs(argc, argv);

	if (!cv_filepath.is_set())
		return error_no_input_filepath;
	if (!cv_output_filepath.is_set())
		return error_no_output_filepath;

	// 1. first, parse all shaders inside the filepath
	string filepath = cv_filepath.get_value<string>();
	auto parse_res = shader::parse_shaders_in_file(filepath);
	if (parse_res.is_fail())
		return error_failed_parse;

	shader::parse_output& parsed_shaders = parse_res.get();
	shader::parse_output::shadermap& parsed_shadermap = parsed_shaders.m_shadermap;
	const int num_parsed_shaders = parsed_shaders.get_total_num_shaders();
	if (num_parsed_shaders <= 0)
		return error_parse_is_empty;

	// 2. find the shader we specifically want compiled (either by optional entrypoint, or the first one we find)
	const shader::parse_output::per_shader* target_parsed_shader = parsed_shaders.get_first_shader();
	if (cv_entrypoint.is_set())
	{
		const shader::parse_output::per_shader* found =
			parsed_shaders.find_shader_by_entrypoint(cv_entrypoint.get_value<string>());

		if (found == nullptr)
			target_parsed_shader = found;
		else 
			return error_entrypoint_not_found;
	}

	// 3. fill in the signature of the selected parsed shader, and finally compile
	shader::shader_signature signature = target_parsed_shader->m_signature;
	shader::compile_args args{};
	args.m_include_folder = cv_includes.get_value<string>();
	args.m_target = shader::e_shader_target::_6_6;
	args.m_reflection_enabled = false;
	auto output_res = shader::compile_shader_in_file(filepath, signature, args);
	if (!output_res.is_success() || !output_res.get().m_success)
	{
		const string first_log = output_res.get().m_log.front();
		logwar("compile shader fail: {}", first_log.c_str());
		return error_compile_error;
	}

	// 4. write to file
	const vector<byte>& bytecode = output_res.get().m_bytecode;
	string output_path = cv_output_filepath.get_value<string>();
	std::ofstream file(output_path, std::ios::binary);
	file.write(reinterpret_cast<const char*>(bytecode.data()), bytecode.size());
	
	logwar("compile shader success!");
	return success;
}