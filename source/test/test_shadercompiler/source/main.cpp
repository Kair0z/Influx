// STL
#include <iostream>
// influx::core
#include "core/basetypes.h"
// influx::import
#include "influx_import.h"

void print_info(const char* mssg)
{
	std::cout << "[] " << mssg << "\n";
}
int main()
{
	print_info("test_shadercompiler");

	using namespace influx;
	const auto filepath = "D:/Git/Influx/source/misc/rendering/resources/shaders.hlsl";
	imp::shader_load_args args{};
	args.m_shaderfilter; // all shaders
	args.m_compile_args.m_target = shader::e_shader_target::_6_6;
	auto res = imp::load_shaders_in_file(filepath, args);

	if (res.is_success())
	{
		for (const auto& shader : res.get())
		{
			// print_info(shader);
		}
		print_info("success!");
	}
	else print_info(res.get_unex());
}