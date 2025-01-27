
// influx::core
#include "core/basetypes.h"
#include "core/log.h"

// influx::shader
#include "influx_shader.h"

int main()
{
	using namespace influx;

	const string path = "D:/Git/Influx/assets/Shaders/source/basepass.hlsl";
	shader::compile_args args{};
	args.m_entrypoint = "main_vs";
	args.m_include_folder = "D:/Git/Influx/assets/Shaders/include/";
	args.m_type = shader::e_shader_type::vs;
	args.m_target = shader::e_shader_target::_6_6;
	args.m_reflection = true;

	shader::compile_output output = shader::compile_shader(path, args);

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