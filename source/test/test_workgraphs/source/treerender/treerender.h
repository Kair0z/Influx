#pragma once

// influx::graphics
#include "influx_graphics/device.h"

// influx::shader
#include "influx_shader.h"

// STL
#include <iostream>

namespace treerender
{
	using namespace influx;

	struct settings final
	{

	};

	struct state final
	{
		graphics::rootsignature* m_rootsig = nullptr;
		graphics::graph_pipeline* m_pipeline = nullptr;
	};
	static state g_state{};

	const char* k_shader = "";

	void initialize(graphics::device& device)
	{
		const char* filepath = "D:/Git/Influx/source/test/test_workgraphs/shaders/main.hlsl";

		// todo: compile from shader snippet
		shader::shader_library_compile_args compile_args{};
		compile_args.m_target = shader::e_shader_target::_6_8;
		auto res = shader::compile_shader_library(filepath, compile_args);
		if (!res)
		{
			for (const auto& line : res.get_safe().m_log)
			{
				std::cout << line << "\n";
			}
			influx_assert(res);
		}
		

		graphics::rootsignature_desc desc{};
		desc.add_root_resource(graphics::root_param_resource::e_type::uav, 0u);
		g_state.m_rootsig = device.create_rootsignature(desc);

		graphics::graph_pipeline_desc pip_desc{};
		pip_desc.m_library_bytecode = res.get().m_bytecode;
		g_state.m_pipeline = device.create_workgraph_pipeline(g_state.m_rootsig, pip_desc);
	}

	void render(
		graphics::commandlist& commandlist, 
		const graphics::resource& backbuffer, 
		const settings& settings = {})
	{
		commandlist.dispatch_workgraph(g_state.m_pipeline);
	}
}