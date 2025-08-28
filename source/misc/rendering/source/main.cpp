// influx::core
#include "core/math/vector.h"
#include "core/string.h"
#include "core/scene/camera.h"
#include "core/math/transform.h"
// influx::import
#include "influx_import.h"
// influx::shader
#include "influx_shader.h"
// influx::graphics
#include "influx_graphics/device.h"
// influx::platform
#include "influx_platform/window.h"
// influx::rendergraph
#include "rendergraph.h"

using namespace influx;

// SDK 1.614.1
extern "C" { __declspec(dllexport) extern const influx::uint32 D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

int main()
{
	// settings
	const string k_scene_filepath = "";
	const string k_shaders_filepath = "D:/Git/Influx/source/test/test_rendergraph/resources/shaders.hlsl";
	static constexpr uint32 k_num_swapchain_buffers = 3u;
	static const math::float4 k_clear_colour = math::float4{ 1,0,0,1 };
	static math::float3 s_camera_startpos = { 0,0,500 };
	static math::float3 s_camera_lookatpos = {};
	static float s_camera_far = 1000.0f;
	static float s_camera_near = 0.001f;
	static float s_camera_fov = 110.0f;
	//

	// load a scene file (.fbx)
	
	{
		imp::scene_load_args scene_load_args{};
		scene_load_args.m_bake_transforms = false;
		scene_load_args.m_pre_scale = 1.0f;
		imp::scene_data loaded_scene = imp::load_scene_file(k_scene_filepath, scene_load_args).get();

		// foreach mesh
		for (uint32 i = 0u; i < loaded_scene.get_num_meshes(); ++i)
		{
			// foreach position (vertex)
			const imp::mesh_data& mesh = loaded_scene.get_mesh(i);
			for (uint64 i = 0u; i < mesh.m_positions.size(); ++i)
			{
				mesh.m_positions[i];
				mesh.m_normals[i];
				mesh.m_uvs[i];
			}
			// foreach (index)
			for (uint64 i = 0u; i < mesh.m_indices.size(); ++i)
			{
				mesh.m_indices[i];
			}
		}
	}

	// setup camera
	camera camera{};
	math::transform3D cam_transform = math::transform3D::identity();
	{
		cam_transform.look_at(s_camera_lookatpos);
		cam_transform.set_position(s_camera_startpos);
		camera.set_farplane(s_camera_far);
		camera.set_nearplane(s_camera_near);
		camera.set_fov(s_camera_fov);
	}

	// make platform window
	platform::window_desc window_desc{};
	window_desc.m_dimensions = { 1280, 720 };
	window_desc.m_name = "rendering";
	platform::window* window = platform::window::create(window_desc);

	// setup core graphics objects
	graphics::device& dev = *graphics::device::create(graphics::e_api_type::dx12);
	graphics::queue& queue = *dev.create_queue();
	graphics::commandlist& cmdlist = *dev.create_graphics_commandlist();

	graphics::swapchain_desc swapchain_desc{};
	swapchain_desc.m_dimensions = window_desc.m_dimensions;
	swapchain_desc.m_format = graphics::e_format::rgba8;
	swapchain_desc.m_num_buffers = k_num_swapchain_buffers;
	graphics::swapchain& swapchain = *dev.create_swapchain(&queue, *window, swapchain_desc);

	// build pipelines (/load shaders)
	graphics::rootsignature* signature_main = nullptr;
	graphics::rootsignature* signature_compute = nullptr;
	graphics::graphics_pipeline* pipeline_main = nullptr;
	graphics::compute_pipeline* pipeline_compute = nullptr;
	{
		// the signatures
		{
			graphics::rootsignature_desc desc{};
			signature_main = dev.create_rootsignature(desc);
			signature_compute = dev.create_rootsignature(desc);
		}

		// the pipelines
		{
			graphics::graphics_pipeline_desc desc{};

			// [input layout]
			desc.add_input_element("SV_POSITION", 0u, graphics::e_format::rgb32, 0u, false, 0u);
			desc.add_input_element("TEXCOORD", 0u, graphics::e_format::rg32, 1u, false, 0u);

			// [rasterizer]
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

			// sampler
			desc.m_sample_mask = (uint32)-1;
			desc.m_sample_count = 1u;

			// [output merger]
			desc.m_depth_stencil.m_depth_enable = false;
			desc.m_depth_stencil.m_stencil_enable = false;

			graphics::compute_pipeline_desc compute_desc{};

			// compile & bind shaders
			{
				shader::compile_args args{};
				auto res = shader::parse_shaders_in_file(k_shaders_filepath);
				influx_assert(res.is_success());

				for (const auto& parse : res.get())
				{
					args.m_signature = parse.m_signature;
					args.m_signature.m_target = shader::e_shader_target::_6_6;
					auto comp_res = shader::compile_shader_in_file(k_shaders_filepath, args);
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

			pipeline_main = dev.create_graphics_pipeline(signature_main, desc);
			pipeline_compute = dev.create_compute_pipeline(signature_compute, compute_desc);
		}
	}

	// create final target
	graphics::resource* final_target = nullptr;
	{
		graphics::tex2D_desc desc{};
		desc.m_allow_uav = true;
		desc.m_arraysize = 1u;
		desc.m_bindflags = graphics::e_bind_flags::rtv;
		desc.m_dimensions = window_desc.m_dimensions;
		desc.m_format = graphics::e_format::rgba8;
		desc.m_init_state = graphics::e_resource_state::render_target;
		desc.m_num_mips = 1u;
		desc.m_sample_count = 1u;
		final_target = dev.create_resource(desc);
		final_target->set_name("final_target");
	}

	// build frame rendergraph (deferred renderer)
	rendergraph::global_config graph_config{};
	rendergraph::rendergraph graph{ graph_config, dev };
	{
		using namespace influx::rendergraph;
		graph.import_texture(final_target);

		// 1. clear
		graph.add_clear_pass(final_target, clear_args{ .m_colour = k_clear_colour });
		
		// 2. basepass
		graph.add_pass(e_rgpass_type::graphics,
			[&final_target, &graph](rgpass_builder& builder)
			{
				texture_desc_options options{};
				builder.write_rendertarget(final_target, rgaccess::keep_and_keep(), options);
			},
			[](rgpass_context& ctx) 
			{
				
			});

		// 3. post processing
		graph.add_pass(e_rgpass_type::graphics,
			[&final_target, &graph](rgpass_builder& builder)
			{
				texture_desc_options options{};
				builder.write_rendertarget(final_target, rgaccess::keep_and_keep(), options);
			},
			[](rgpass_context& ctx)
			{

			});

		graph.build();
	}
	
	// run the render
	bool is_quit = false;
	while (!is_quit)
	{
		window->poll_events(is_quit);

		cmdlist.start(&dev).get();
		
		// render to final target
		graph.execute(cmdlist, dev).get();

		// copy final target into backbuffer
		graphics::resource* backbuffer = swapchain.get_current_backbuffer_resource().get();
		backbuffer->transition(cmdlist, graphics::e_resource_state::copy_dst);
		final_target->transition(cmdlist, graphics::e_resource_state::copy_src);

		graphics::copy_texture_args copy_args{};
		cmdlist.copy_texture(final_target, backbuffer, copy_args);

		backbuffer->transition(cmdlist, graphics::e_resource_state::present);
		
		// submit & present
		cmdlist.end().get();
		queue.submit({ &cmdlist }).get();
		swapchain.present({}).get();
	}
}