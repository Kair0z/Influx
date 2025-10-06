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

// shader frontend
namespace frontend
{
#include "D:/Git/Influx/source/misc/virtualbox/resources/shaders.hlsl"
}

struct globals
{
	// constants
	inline static const string appname		= "virtualbox";
	inline static const string path_assets	= "D:/Git/Influx/assets/misc/";
	inline static const string path_mesh	= path_assets + "soldier.fbx";
	inline static const string path_resources = "D:/Git/Influx/source/misc/virtualbox/resources/";
	inline static const string path_shaders = path_resources + "shaders.hlsl";

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

	rhi::pipeline pip_basepass; rhi::pipeline pip_shadepass;
	rhi::rootsignature sig_basepass; rhi::rootsignature sig_shadepass;
	rhi::renderpass renderpass;

	rhi::memheap gpu_memory;
	{
		rhi::memheap_create_args args{};
		args.m_bytesize = 4u * 1024u * 1024u;
		args.m_bytesize *= 64u;
		gpu_memory = device.create(args).get();
	}
	
	const math::uint2 virtual_texture_dimensions = math::uint2::make_one() * 8u * 1024u;
	rhi::texture tex_virtual;
	{
		rhi::texture_create_args args = rhi::texture_create_args::tex2D(virtual_texture_dimensions);
		args.m_is_virtual = true;
		tex_virtual = device.create(args).get();
	}
	rhi::buffer buff_virtual;
	{
		rhi::buffer_create_args args{};
		args.m_bytesize = 8u * 1024u;
		args.m_is_virtual = true;
		buff_virtual = device.create(args).get();
	}

	rhi::texture tex_gbalbedo;
	rhi::texture tex_gbdepth;
	rhi::texture tex_depth;
	rhi::texture tex_ftarget;
	{
		rhi::texture_create_args args = rhi::texture::create_args::tex2D(win_desc.m_dimensions);
		args.mod_bindflags(rhi::e_resource_bindflags::rtv);
		tex_ftarget = device.create(args).get();

		args.mod_format(rhi::pixelformat::make_f32(4u));
		tex_gbalbedo = device.create(args).get();
		args.mod_format(rhi::pixelformat::make_f32(1u));
		tex_gbdepth = device.create(args).get();

		args = rhi::texture_create_args::tex2D_depth(win_desc.m_dimensions);
		tex_depth = device.create(args).get();
	}

	rhi::renderpass_create_args pass_args{};
	pass_args.describe_color(0u, tex_gbalbedo);
	pass_args.describe_color(1u, tex_gbdepth);
	pass_args.describe_depth(tex_depth);
	renderpass = device.create(pass_args).get();

	g.reload_shaders_func = [&device, &pip_basepass, &sig_basepass, &pip_shadepass, &sig_shadepass, &renderpass]()
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
		
		// signatures
		{
			rhi::rootsignature_create_args rootsig_args{};
			rootsig_args.m_direct_indexing = true;
			rootsig_args.reflect_shader(vertexshader.m_reflection, shader::e_shader_type::vs);
			rootsig_args.reflect_shader(pixelshader.m_reflection, shader::e_shader_type::ps);
			sig_basepass = device.create(rootsig_args).get();

			rootsig_args = {};
			rootsig_args.reflect_shader(computeshader.m_reflection, shader::e_shader_type::cs);
			sig_shadepass = device.create(rootsig_args).get();
		}
		// pipelines
		{
			rhi::graphics_shaderslots graphics_shaders;
			rhi::graphics_pipeline_desc args{};
			args.reflect_input_elements(vertexshader.m_reflection);
			args.reflect_pixelshader(pixelshader.m_reflection);
			args.m_shaderpipeline = rhi::e_graphics_shader_pipeline::vs_ps;
			graphics_shaders.set(shader::e_shader_type::vs, vertexshader.m_bytecode);
			graphics_shaders.set(shader::e_shader_type::ps, pixelshader.m_bytecode);
			pip_basepass = device.create_graphics_pipeline(sig_basepass, renderpass, graphics_shaders, args).get();

			rhi::compute_shaderslots compute_shaders;
			compute_shaders.set(shader::e_shader_type::cs, computeshader.m_bytecode);
			pip_shadepass = device.create_compute_pipeline(sig_shadepass, compute_shaders).get();
		}
	};

	rhi::buffer buff_drawcb;
	{
		rhi::buffer_create_args args{};
		args.m_bindflags = rhi::e_resource_bindflags::constbuffer;
		args.m_bytesize	= sizeof(frontend::constants);
		args.m_init_state = rhi::e_resource_state::gen_read;
		args.m_memoryheap_desc = rhi::memoryheap_desc::cpu_writable();
		buff_drawcb = device.create(args).get();
	}
	
	while (!g.is_quit)
	{
		window->poll_events(g.is_quit);
		cmdlist.start(device);

		rhi::texture backbuffer = swapchain.get_backbuffer_resource().get();
		cmdlist.transition(backbuffer, rhi::e_resource_state::render_target);
		cmdlist.clear_texture(device, backbuffer, { .m_colour = {1,0,0,1} });

		rhi::vmemory_map_args map_args{};
		map_args.m_heap_start = 0u;
		map_args.m_texelrange_start = { 0,0,0 };
		map_args.m_texelrange_size = { virtual_texture_dimensions.x, virtual_texture_dimensions.y, 1u };
		rhi::vmemory_map_result result = queue.map_vmemory(device, tex_virtual, gpu_memory, map_args).get();
		
		queue.unmap_vmemory(device, tex_virtual, map_args);

		if (pip_basepass.is_valid())
		{
			buff_drawcb.write_data<frontend::constants>({ .m_viewprojection = {} });

			rhi::begin_renderpass_args args{};
			args.bind_depth(tex_depth);
			args.bind_color(0u, tex_gbalbedo);
			args.bind_color(1u, tex_gbdepth);

			cmdlist.renderpass_begin(device, renderpass, args);
			cmdlist.bind_pipeline(pip_basepass);
			cmdlist.bind_rootsignature(sig_basepass);
			cmdlist.bind_buffer_cbv(buff_drawcb, 0u);
			cmdlist.draw_indexed({});
			cmdlist.renderpass_end();
		}
		if (pip_shadepass.is_valid())
		{
			const bool is_compute = true;
			cmdlist.bind_rootsignature(sig_shadepass, is_compute);
			cmdlist.bind_pipeline(pip_shadepass);
			cmdlist.bind_texture_uav(tex_ftarget, 0u, is_compute);
			cmdlist.bind_texture_srv(tex_gbalbedo, 0u, is_compute);
			cmdlist.bind_texture_srv(tex_gbdepth, 1u, is_compute);
			const uint32 num_groups_x = win_desc.m_dimensions.x / TGSIZE;
			const uint32 num_groups_y = win_desc.m_dimensions.y / TGSIZE;
			cmdlist.dispatch({ num_groups_x,num_groups_y, 1u });
		}

		cmdlist.transition(tex_ftarget, rhi::e_resource_state::copy_src);
		cmdlist.transition(backbuffer, rhi::e_resource_state::copy_dst);
		cmdlist.copy(tex_ftarget, backbuffer);
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