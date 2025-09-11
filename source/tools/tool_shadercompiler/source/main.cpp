
// influx::core
#include "core/basetypes.h"
#include "core/log.h"

// influx::shader
#include "influx_shader.h"

int main()
{
	using namespace influx;

	const string path = "E:/Git/Influx/assets/engine/shaders/source/resolvepass.hlsl";
	
	shader::shader_signature signature{};
	shader::compile_args args{};
	signature.m_entrypoint = "main_cs";
	signature.m_filename = "resolvepass";
	signature.m_type = shader::e_shader_type::cs;

	args.m_include_folder = "E:/Git/Influx/assets/engine/shaders/";
	args.m_target = shader::e_shader_target::_6_6;
	args.m_reflection_enabled = true;

	shader::compile_output output = shader::compile_shader_in_file(path, signature, args).get();
	if (!output.m_success)
	{
		const string first_log = output.m_log.front();
		logwar("compile shader fail: {}", first_log.c_str());
	}
	else
	{
		logwar("compile shader success!");
	}
}