
#include "core/basetypes.h"

// SDK 1.614.1
extern "C" { __declspec(dllexport) extern const influx::uint32 D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

// influx::platform
#include "influx_platform/window.h"

// influx::graphics
#include "influx_graphics/device.h"

// influx::rendergraph
#include "rendergraph.h"

// influx::import
#include "influx_shader.h"

using namespace influx;

struct pipeline
{
	graphics::graphics_pipeline* m_pipeline = nullptr;
	graphics::rootsignature* m_signature = nullptr;

	pipeline(graphics::device& device)
	{
		{
			graphics::rootsignature_desc desc{};
			m_signature = device.create_rootsignature(desc);
		}
		{
			graphics::graphics_pipeline_desc desc{};

			const string filepath = "";
			shader::compile_args args{};
			auto res = shader::parse_shaders_in_file(filepath);
			influx_assert(res.is_success());

			for (const auto& parse : res.get())
			{
				args.m_signature = parse.m_signature;
				args.m_signature.m_target = shader::e_shader_target::_6_6;
				auto comp_res = shader::compile_shader_in_file(filepath, args);
				influx_assert(comp_res.is_success());
			}

			m_pipeline = device.create_graphics_pipeline(m_signature, desc);
		}
	}
};

int main()
{
	using namespace influx::rendergraph;

	platform::window_desc window_desc{};
	window_desc.m_dimensions = { 640u, 480u };
	window_desc.m_name = "rendergraph";

	platform::window* window = platform::window::create(window_desc);
	graphics::device* device = graphics::device::create(graphics::e_api_type::dx12);
	graphics::queue* queue = device->create_queue();
	graphics::commandlist* commandlist = device->create_graphics_commandlist();
	
	graphics::swapchain_desc swpchain_desc{};
	swpchain_desc.m_dimensions = window->get_dimensions(platform::window::e_space::client);
	swpchain_desc.m_format = graphics::e_format::rgba8;
	swpchain_desc.m_num_buffers = 3u;
	graphics::swapchain* swapchain = device->create_swapchain(queue, *window, swpchain_desc);

	influx::rendergraph::rendergraph graph{ {}, *device };

	pipeline pipeline{ *device };

	while (true)
	{
		auto res = swapchain->get_current_backbuffer_resource();
		if (res.is_unex()) continue;

		uint8 backbuffer_index = swapchain->get_current_backbuffer_index();
		const string& current_backbuffer_name = "backbuffer_" + to_string(backbuffer_index);
		graph.import_texture(current_backbuffer_name, res.get());

		// clear backbuffer pass
		graph.add_graphics_pass([&current_backbuffer_name](auto& builder)
		{
			// simply load the rendertarget with a clear
			builder.write_rendertarget(current_backbuffer_name, rgaccess::clear_and_keep({ 1,0,0,1 }));
		});

		// simple draw pass
		graph.add_graphics_pass(
		[&current_backbuffer_name](auto& builder) // build
		{
			builder.write_rendertarget(current_backbuffer_name, rgaccess::keep_and_keep());
		},
		[&pipeline](auto& ctx) // execute
		{
			graphics::commandlist& cmdlist = ctx.get_commandlist();
			cmdlist.set(&pipeline.m_pipeline,);
		});

		commandlist->start(device);

		graph.build();
		graph.execute(*commandlist, *device);

		// transition backbuffer to present
		res.get()->transition(commandlist, graphics::e_resource_state::present);
		commandlist->end();
		commandlist->submit(queue);
		commandlist->wait_for_completion();
		
		graph.reset_graph();

		swapchain->present({ .m_vsync = false });
		
	}
}