// influx::core
#include "core/threading/thread.h"
#include "core/string.h"
// influx::platform
#include "influx_platform/window.h"
// influx::rhi
#include "influx_rhi.h"
// influx::imp
#include "influx_import.h"
// STL
#include <iostream>
#include <functional>
using namespace influx;

struct globals
{
	// constants
	inline static const string appname		= "virtualbox";
	inline static const string path_assets	= "D:/Git/Influx/assets/misc/";
	inline static const string path_mesh	= path_assets + "soldier.fbx";
	inline static const string path_shaders = path_assets + "shaders.hlsl";

	inline static umap<string, string> path_aliases =
	{
		{"soldier", path_mesh}
	};

	static globals& get() { static globals g; return g; }
	bool is_quit = false;

	umap<string, imp::scene_data> m_loaded_scenes{};
	umap<string, imp::shader_data> m_loaded_shaders{};

	std::function<void()> reload_shaders_func = nullptr;
};

int commandline_thread()
{
	globals& g = globals::get();
	string line{};
	while (!g.is_quit)
	{
		std::cout << "> "; // prompt
		if (!std::getline(std::cin, line)) 
			continue;

		// exit if exit
		if (line == "exit")
		{
			g.is_quit = true;
			return 0;
		}

		// parse line tokens
		std::istringstream iss(line);
		std::vector<std::string> tokens;
		std::string token;
		while (iss >> token) { tokens.push_back(token); }
		if (tokens.empty()) continue;

		// loadmesh command
		const string& cmd = tokens[0];
		if (cmd == "loadmesh" && tokens.size() == 2)
		{
			string path = tokens[1];

			// see if it's an alias
			if (g.path_aliases.contains(path))
				path = g.path_aliases[path];

			if (g.m_loaded_scenes.contains(path))
			{
				std::cout << "> mesh [" << path << "] already loaded! skipping...\n";
				continue;
			}

			std::cout << "> loading mesh [" << path << "]...\n";
			imp::scene_load_args args{};
			auto load_res = imp::load_scene_file(path, args);
			if (load_res.is_fail())
			{
				std::cout << "> failed loading mesh!\n";
				continue;
			}
			else
			{
				std::cout << "> success! \n";
				g.m_loaded_scenes[path] = load_res.get();
			}
		}
		
		// reload shaders command
		if (cmd == "reloadsh")
		{
			if (g.reload_shaders_func)
				g.reload_shaders_func();
		}
	}
}
int main()
{
	globals& g = globals::get();
	thread command_thread = thread([]() { commandline_thread(); });

	platform::window_desc win_desc{};
	win_desc.set_dimensions({ 640u, 480u }).set_name(g.appname).set_style(platform::window_style::get_nodecoration());
	platform::window* window = platform::window::create(win_desc);

	rhi::device device = rhi::create_device(rhi::device::create_args::make(g.appname, true)).get();
	rhi::queue queue = device.create(rhi::queue::create_args::default_graphics()).get();
	rhi::commandlist cmdlist = device.create(rhi::commandlist::create_args::default_graphics()).get();

	rhi::swapchain::create_args swap_desc{};
	{
		swap_desc.m_platform_instance = platform::platform::get_current_instance();
		swap_desc.m_window = window->get_platform_handle();
		swap_desc.m_dimensions = window->get_dimensions();
		swap_desc.m_queue = queue.m_native_object;
		swap_desc.m_format = rhi::pixelformat::rgba_8_unorm();
	}
	rhi::swapchain swapchain = device.create(swap_desc).get();

	rhi::pipeline pipeline;
	rhi::rootsignature signature;
	rhi::renderpass renderpass;
	g.reload_shaders_func = [&device, &pipeline, &signature, &renderpass]()
	{
		globals& g = globals::get();
		std::cout << "reloading shaders \n";
		
		// shaders
		shader::compile_output vertexshader;
		shader::compile_output pixelshader;
		shader::compile_output computeshader;
		{
			auto res_parsed_shaders = shader::parse_shaders_in_file(g.path_shaders);
			if (!res_parsed_shaders)
				return;

			shader::compile_args args{};
			args.set_debug_level(shader::e_compile_debug_level::debug)
				.set_include_folder("")
				.set_pdb_enabled(false)
				.set_platform(shader::e_shader_platform::DXIL)
				.set_reflection_enabled(true)
				.set_target(shader::e_shader_target::_6_6);

			const auto& parsed_shaders = res_parsed_shaders.get();
			if (parsed_shaders.contains(shader::e_shader_type::vs))
			{
				shader::shader_signature signature = parsed_shaders.get_shadermap(shader::e_shader_type::vs)[0].m_signature;
				vertexshader = shader::compile_shader_in_file(g.path_shaders, signature, args).get();
			}
			if (parsed_shaders.contains(shader::e_shader_type::ps))
			{
				shader::shader_signature signature = parsed_shaders.get_shadermap(shader::e_shader_type::ps)[0].m_signature;
				pixelshader = shader::compile_shader_in_file(g.path_shaders, signature, args).get();
			}
			if (parsed_shaders.contains(shader::e_shader_type::cs))
			{
				shader::shader_signature signature = parsed_shaders.get_shadermap(shader::e_shader_type::cs)[0].m_signature;
				computeshader = shader::compile_shader_in_file(g.path_shaders, signature, args).get();
			}
		}
		
		// signature
		{
			rhi::rootsignature_create_args rootsig_args{};
			rootsig_args.m_direct_indexing = true;
			rootsig_args.reflect_shader(vertexshader.m_reflection, shader::e_shader_type::vs);
			rootsig_args.reflect_shader(pixelshader.m_reflection, shader::e_shader_type::ps);
			signature = device.create(rootsig_args).get();
		}
		// pipeline
		{
			rhi::graphics_shaderslots graphics_shaders;
			rhi::graphics_pipeline_desc args{};
			args.reflect_input_elements(vertexshader.m_reflection);
			args.m_shaderpipeline = rhi::e_graphics_shader_pipeline::vs_ps;
			graphics_shaders.set(shader::e_shader_type::vs, vertexshader.m_bytecode);
			graphics_shaders.set(shader::e_shader_type::ps, pixelshader.m_bytecode);
			pipeline = device.create_graphics_pipeline(signature, renderpass, graphics_shaders, args).get();
		}
	};

	while (!g.is_quit)
	{
		window->poll_events(g.is_quit);
		cmdlist.start(device);

		rhi::texture backbuffer = swapchain.get_backbuffer_resource().get();
		cmdlist.transition(backbuffer, rhi::e_resource_state::render_target);
		cmdlist.clear_texture(device, backbuffer, { .m_colour = {1,0,0,1} });

		if (pipeline.is_valid())
		{
			cmdlist.bind_pipeline(pipeline);
			cmdlist.bind_rootsignature(signature);
		}

		cmdlist.transition(backbuffer, rhi::e_resource_state::present);
		cmdlist.end();
		cmdlist.submit(queue);

		rhi::present_args args{};
		args.m_device = device.m_native_object;
		args.m_present_queue = queue.m_native_object;
		args.m_flags;
		args.m_sync_interval;
		swapchain.present(args);
	}

	command_thread.detach();
}