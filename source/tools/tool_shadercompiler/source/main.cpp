// influx::core
#include "core/basetypes.h"
#include "core/log.h"
#include "core/commandline.h"
#include "core/file.h"
#include "core/basetypes.h"

// influx::shader
#include "influx_shader.h"

// STL
#include <fstream>

static const char* k_invalid_path = "";
static const char* k_default_includes = "";
static const char* k_invalid_entrypoint = "_";

// required parameters:
influx::cvar cv_filepath		("cv_inputpath",	k_invalid_path,			"required: filepath of the shader");
influx::cvar cv_output_filepath ("cv_outputpath",	k_invalid_path,			"required: output filepath");
influx::cvar cv_output_refl		("cv_reflpath",		k_invalid_path,			"required: output filepath for the binary reflection output");
// optional parameters:
influx::cvar cv_entrypoint		("cv_entry",		k_invalid_entrypoint,	"entrypoint of the shader");
influx::cvar cv_includes		("cv_includes",		k_default_includes,		"folder of included files");

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
	error_output_write_failed		= -8,
	error_no_output_reflection_filepath = -9
};

void log(const influx::string& info)
{
	std::cout << info.c_str() << std::endl;
	// logwar("compile shader fail: {}", first_log.c_str());
}

int main(int argc, char** argv)
{
	using namespace influx;
	cvar::parse_runargs(argc, argv);

	const string input_filepath = cv_filepath.get_value<string>(); // input_filepath = "D:/Git/Influx/source/influx/influx_renderer/shaders/source/slang/basepass.slang";
	const string output_filepath = cv_output_filepath.get_value<string>();
	if (input_filepath == k_invalid_path)
	{
		return error_no_input_filepath;
	}
	if (output_filepath == k_invalid_path)
	{
		return error_no_output_filepath;
	}
	const string refl_filepath = cv_output_refl.get_value<string>();
	if (refl_filepath == k_invalid_path)
	{
		return error_no_output_reflection_filepath;
	}

	// 1. first, parse all shaders inside the filepath
	cptr<shader::parse_output::per_shader> target_parsed_shader = nullptr;
	auto parse_res = shader::parse_shaders_in_file(input_filepath);
	if (parse_res.is_fail())
	{
		return error_failed_parse;
	}
	shader::parse_output& parsed_shaders = parse_res.get();
	shader::parse_output::shadermap& parsed_shadermap = parsed_shaders.m_shadermap;
	const int num_parsed_shaders = parsed_shaders.get_total_num_shaders();
	if (num_parsed_shaders <= 0)
	{
		return error_parse_is_empty;
	}

	// 2. find the shader we specifically want compiled (either by optional entrypoint, or the first one we find)
	if (cv_entrypoint.is_set())
	{
		const shader::parse_output::per_shader* shader_found_in_parse =
			parsed_shaders.find_shader_by_entrypoint(cv_entrypoint.get_value<string>());

		if (shader_found_in_parse != nullptr)
			target_parsed_shader = shader_found_in_parse;
		else
			return error_entrypoint_not_found;
	}
	else target_parsed_shader = parsed_shaders.get_first_shader();

	// 3. fill in the signature of the selected parsed shader, 
	// and finally compile
	const string include_folder_path = cv_includes.get_value<string>(); // "D:/Git/Influx/source/influx/influx_renderer/shaders/source/slang/";
	const string entrypoint = cv_entrypoint.get_value<string>(); // "main_vs"
	shader::shader_signature signature = target_parsed_shader->m_signature;
	signature.m_entrypoint = entrypoint;

	shader::compile_args args{};
	args.m_source_language = shader::e_shader_language::HLSL;
	args.m_output_format = shader::e_shader_binary_output::DXIL;
	args.m_include_folder = include_folder_path;
	args.m_target = shader::e_shader_target::_6_6;
	args.m_reflection_enabled = true;
	auto output_res = shader::compile_shader_in_file(input_filepath, signature, args);
	if (!output_res.is_success() || output_res.get().m_bytecode.empty())
	{
		const string first_log = output_res.get_safe().m_log.front();
		log("compile shader fail");
		return error_compile_error;
	}

	// 4. write to output file (.cso)
	{
		const vector<byte>& bytecode = output_res.get().m_bytecode;
		string output_path = cv_output_filepath.get_value<string>();
		std::ofstream file(output_path, std::ios::binary);
		if (file.is_open() == false)
		{
			log("failed writing output file!");
			return error_output_write_failed;
		}
		file.write(reinterpret_cast<const char*>(bytecode.data()), bytecode.size());
	}
	// 5. write to refl file (.refl)
	{
		const shader::reflection& reflection = output_res.get().m_reflection;
		std::ofstream file(refl_filepath, std::ios::binary);
		if (file.is_open() == false)
		{
			log("failed writing reflection file!");
			return error_no_output_reflection_filepath;
		}
		shader::reflection::serialize(reflection, file);
	}
	
	log("compile shader success!");
	return success;
}