
// influx::core
#include "core/basetypes.h"
#include "core/math/vector.h"

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

using index = uint32;
struct vertex final
{
	math::float3 m_position;
	math::float2 m_uv;
};
static const index k_indexbuffer[3u]{ 0,1,2 };
static const vertex k_vertexbuffer[3u]
{
	{{3, -1, 0}		, { 0, 2}},
	{{-1, -1, 0}	, { 0, 0}},
	{{-1, 3, 0}		, { 2, 0}}
};

struct pipeline final
{
	graphics::graphics_pipeline* m_pipeline = nullptr;
	graphics::rootsignature* m_signature = nullptr;
	graphics::compute_pipeline* m_compute_pipeline = nullptr;
	graphics::rootsignature* m_compute_signature = nullptr;

	pipeline(graphics::device& device)
	{
		{
			graphics::rootsignature_desc desc{};
			m_signature = device.create_rootsignature(desc);

			// ...
			m_compute_signature = device.create_rootsignature(desc);
		}
		
		// create the pipelines
		graphics::graphics_pipeline_desc desc{};
		
		// input layout
		desc.add_input_element("SV_POSITION", 0u, graphics::e_format::rgb32	, 0u, false, 0u);
		desc.add_input_element("TEXCOORD"	, 0u, graphics::e_format::rg32	, 1u, false, 0u);

		// rasterizer
		desc.m_prim_type = graphics::e_primitive_topology_type::triangle;
		desc.m_rasterizer.m_cullmode = graphics::e_cull_mode::nocull;
		desc.m_rasterizer.m_fillmode = graphics::e_fill_mode::solid;
		desc.m_rasterizer.m_forced_samplecount = 0u;
		desc.m_rasterizer.m_front_ccw = false;
		desc.m_rasterizer.m_multisample = false;
		desc.m_rasterizer.m_antialiased_line = false;
		desc.m_rasterizer.m_depth_clip_enable = false;
		desc.m_rasterizer.m_conservative = false;
		desc.m_rasterizer.m_depth_bias = 0;
		desc.m_rasterizer.m_depth_bias_clamp = 0.0f;
		desc.m_rasterizer.m_slope_depth_bias = 0.0f;

		desc.m_sample_mask = (uint32)-1;
		desc.m_sample_count = 1u;

		desc.m_depth_stencil.m_depth_enable = false;
		desc.m_depth_stencil.m_stencil_enable = false;

		graphics::compute_pipeline_desc compute_desc{};

		// gather shaders
		const string filepath = "D:/Git/Influx/source/test/test_rendergraph/resources/shaders.hlsl";
		{
			shader::compile_args args{};
			auto res = shader::parse_shaders_in_file(filepath);
			influx_assert(res.is_success());

			for (const auto& parse : res.get())
			{
				args.m_signature = parse.m_signature;
				args.m_signature.m_target = shader::e_shader_target::_6_6;
				auto comp_res = shader::compile_shader_in_file(filepath, args);
				influx_assert(comp_res.is_success());

				switch (args.m_signature.m_type)
				{
				case shader::e_shader_type::vs:
					desc.m_shaders.set(graphics::e_graphics_shader_slots::vs, comp_res.get().m_bytecode);
					break;

				case shader::e_shader_type::ps:
					desc.m_shaders.set(graphics::e_graphics_shader_slots::ps, comp_res.get().m_bytecode);
					break;

				case shader::e_shader_type::cs:
					compute_desc.m_shaders.set(graphics::e_compute_shader_slots::cs, comp_res.get().m_bytecode);
					break;
				}
			}
		}
		
		m_pipeline = device.create_graphics_pipeline(m_signature, desc);
		m_compute_pipeline = device.create_compute_pipeline(m_compute_signature, compute_desc);
	}
};

struct geometry final
{
	graphics::resource* m_vertexbuffer = nullptr;
	graphics::resource* m_indexbuffer= nullptr;

	geometry(graphics::device& device)
	{
		{
			graphics::buffer_desc desc{};
			desc.m_bytesize = sizeof(k_vertexbuffer);
			desc.m_bytestride = sizeof(vertex);
			m_vertexbuffer = device.create_resource(desc, graphics::heap_desc::shared_heap());
			m_vertexbuffer->map([](void* target)
			{
				memcpy(target, k_vertexbuffer, sizeof(k_vertexbuffer));
			});
		}
		{
			graphics::buffer_desc desc{};
			desc.m_bytesize = sizeof(k_indexbuffer);
			desc.m_bytestride = sizeof(index);
			desc.m_format = graphics::e_format::u32;
			m_indexbuffer = device.create_resource(desc, graphics::heap_desc::shared_heap());
			m_indexbuffer->map([](void* target)
			{
				memcpy(target, k_indexbuffer, sizeof(k_indexbuffer));
			});
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
	geometry geometry{ *device };

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
		[&current_backbuffer_name, &swapchain](rgpass_builder& builder) // build
		{
			builder.set_viewport(swapchain->get_dimensions().x, swapchain->get_dimensions().y);
			builder.write_rendertarget(current_backbuffer_name, rgaccess::keep_and_keep());
		},
		[&pipeline, &geometry](auto& ctx) // execute
		{
			graphics::commandlist& cmdlist = ctx.get_commandlist();
			cmdlist.set_rootsignature(pipeline.m_signature);
			cmdlist.set_pipeline(pipeline.m_pipeline);
			cmdlist.set_primitive_topology(graphics::e_primitive_topology::trilist);
			cmdlist.set_vertexbuffer(geometry.m_vertexbuffer);
			cmdlist.set_indexbuffer(geometry.m_indexbuffer);
			cmdlist.draw_indexed({
				.m_num_indexes_per_instance = 3u,
				.m_num_instances = 1u,
				.m_start_index = 0u,
				.m_start_vertex = 0,
				.m_start_instance = 0u
			});
		});

		// add a compute pass
#if 0
		graph.add_compute_pass(
		[&current_backbuffer_name](auto& builder)
		{
			builder.write_texture(current_backbuffer_name);
		},
		[&pipeline, &current_backbuffer_name](rgpass_context& context)
		{
			graphics::commandlist& cmdlist = context.get_commandlist();
			
			/* get the write texture */
			auto write_texture = context.get_write_texture(current_backbuffer_name);
			
			cmdlist.set(pipeline.m_compute_pipeline);
			cmdlist.set(pipeline.m_compute_signature);
			cmdlist.set_uav(write_texture.get().m_resource, 0u, graphics::e_pipeline_type::compute);

			const auto num_thread_groups = math::uint3{ 1,1,1 };
			cmdlist.dispatch({ num_thread_groups });
		});
#endif

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