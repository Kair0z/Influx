// influx::core
#include "core/math/vector.h"
#include "core/string.h"
#include "core/scene/camera.h"
#include "core/math/transform.h"
#include "core/time.h"
#include "core/container/map.h"
#include "core/ascii_art.h"
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

#include <iostream>

// shader frontend
namespace frontend
{
#include "E:/Git/Influx/source/misc/rendering/resources/shaders.hlsl"
}

using namespace influx;

// SDK 1.614.1
extern "C" { __declspec(dllexport) extern const influx::uint32 D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

int main()
{
	struct cpu_timings
	{
		umap<const char*, float> m_records{};
		umap<const char*, uint32> m_record_nums{};
		void reset()
		{
			for (const auto& pair : m_records)
			{
				m_records[pair.first] = 0.0f;
				m_record_nums[pair.first] = 0u;
			}
		}
	};

	static cpu_timings timings{};
	static uint64 gpu_memory_budget = 0u;
	static uint64 gpu_memory_used = 0u;
	class scope final
	{
		influx::time::point m_start;
		const char* m_name;
	public:
		scope(const char* name) : m_name{ name }
		{
			m_start = time::get_now();
		}
		~scope()
		{
			float ms = time::get_ms_between<float>(time::get_now(), m_start);
			timings.m_records[m_name] += ms;
			timings.m_record_nums[m_name]++;
		}
	};

	auto log_timings = []()
	{
		std::cout << "\033[H\033[J"; // clear prev
		using pair = std::pair<const char*, float>;
		vector<pair> averages{};
		averages.reserve(timings.m_records.size());

		// gather avgs
		for (const auto& pair : timings.m_records)
		{
			const uint32 num = timings.m_record_nums[pair.first];
			const float avg = pair.second / (float)num;
			averages.push_back({ pair.first, avg });
		}
		std::sort(averages.begin(), averages.end(), [](const pair& a, const pair& b)
		{
			return a.second > b.second;
		});
		
		// print avgs
		artscii::progress_bar bar{}; 
		bar.cursor_char() = '=';
		bar.bar_length() = 64u;
		const float frame_avg = averages[0].second;
		for (const auto& pair : averages)
		{
			const float avg = pair.second;
			bar.pc() = avg / frame_avg;
			std::cout
				<< "[" << std::left << std::setw(15) << pair.first << "]"			// Label aligned with ']'
				<< std::setw(10)													// Space between ']' and bar
				<< std::setw(20) << bar.get_cstr()									// Bar with fixed width
				<< std::right << std::setw(10) << std::fixed << std::setprecision(4) // Right-align the average
				<< avg << " ms" << std::endl;
		}
		std::cout << std::endl;

		bar.bar_length() = 32u;
		const float gpu_used = (float)gpu_memory_used;
		const float gpu_budget = (float)gpu_memory_budget;
		const float GB_used = gpu_used / (1024 * 1024 * 1024);
		const float GB_budget = gpu_budget / (1024 * 1024 * 1024);
		bar.pc() = gpu_used / gpu_budget;
		std::cout << "[VRAM] " << bar.get_cstr() << " " << std::setprecision(4) << GB_used << "/" << GB_budget<< " GB \n";
		std::cout << std::flush;

		std::this_thread::sleep_for(std::chrono::seconds(1)); // wait a second per log
	};

	// settings
	static const string k_scene_filepath = "E:/Git/Influx/source/misc/rendering/resources/soldier.fbx";
	static const string k_shaders_filepath = "E:/Git/Influx/source/misc/rendering/resources/shaders.hlsl";
	static const string k_albedo_filepath = "E:/Git/Influx/source/misc/rendering/resources/albedo.png";
	static const string k_normal_filepath = "E:/Git/Influx/source/misc/rendering/resources/normals.png";

	static const math::float4 k_clear_colour = math::float4{ 1,0,0,1 };

	// camera
	static math::float3 s_camera_startpos = { 0,0,5 };
	static math::float3 s_camera_lookatpos = {};
	static float s_camera_far = 1000.0f;
	static float s_camera_near = 0.001f;
	static float s_camera_fov = 110.0f;
	static math::uint2 s_window_dim = { 640u, 480u };
	static constexpr uint32 k_num_swapchain_buffers = 3u;
	static constexpr shader::e_shader_target k_shadertarget = shader::e_shader_target::_6_6;
	static constexpr uint32 k_max_num_lights = 512u;
	static constexpr uint32 k_max_num_instances = 4096u;
	static constexpr uint32 k_num_gbuffers = 3u;
	static constexpr uint32 k_num_vertices = 3u; // triangle
	static constexpr uint32 k_num_indices = 3u; // triangle
	static constexpr graphics::e_format k_gbuffer_formats[k_num_gbuffers]
	{
		graphics::e_format::rgba_u32,
		graphics::e_format::u32,
		graphics::e_format::u32,
	};
	static const rendergraph::rgname gbuffernames[k_num_gbuffers]
	{
		"gbuffer_a",
		"gbuffer_b",
		"gbuffer_c"
	};
	// -----------

	// load a scene file (.fbx)
	imp::scene_data loaded_scene{};
	{
		bool finished = false;
		artscii::progress_bar bar{};
		bar.bar_length() = 64u;
		std::thread loading = std::thread([&bar, &finished]()
		{
			while (!finished)
			{
				bar.increment(1u);
				std::cout << "\r" << bar.get_cstr() << "loading soldier.fbx ";
				std::this_thread::sleep_for(std::chrono::milliseconds(180));
			}
		});

		imp::scene_load_args scene_load_args{};
		scene_load_args.m_bake_transforms = false;
		scene_load_args.m_pre_scale = 1.0f;
		loaded_scene = imp::load_scene_file(k_scene_filepath, scene_load_args).get();
		finished = true;

		loading.join();
	}

	// load images
	uint64 total_bytesize_loaded_images = 0u;
	vector<imp::image_data> loaded_images{};
	{
		loaded_images.push_back(imp::load_image_file(k_albedo_filepath).get());
		loaded_images.push_back(imp::load_image_file(k_normal_filepath).get());
		total_bytesize_loaded_images += loaded_images[0].m_bytesize;
		total_bytesize_loaded_images += loaded_images[1].m_bytesize;
	}

	// setup camera
	influx::camera camera{};
	math::transform3D cam_transform = math::transform3D::identity();
	{
		cam_transform.look_at(s_camera_lookatpos);
		cam_transform.set_position(s_camera_startpos);
		cam_transform.update_matrix();
		camera.set_farplane(s_camera_far);
		camera.set_nearplane(s_camera_near);
		camera.set_fov(s_camera_fov);
		camera.set_aspect_ratio((float)s_window_dim.x / (float)s_window_dim.y);
	}

	// make platform window
	platform::window_desc window_desc{};
	window_desc.m_dimensions = s_window_dim;
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

	// load shaders / build pipelines
	graphics::rootsignature* signature_basepass = nullptr;
	graphics::rootsignature* signature_shadepass = nullptr;
	graphics::graphics_pipeline* pipeline_basepass = nullptr;
	graphics::compute_pipeline* pipeline_shadepass = nullptr;
	vector<shader::compile_output> compiled_shaders{};
	compiled_shaders.resize(3u);
	std::atomic_bool pipelines_compiled = false;
	auto recompile_shaders = 
	[&dev, &pipeline_basepass, &pipeline_shadepass, &compiled_shaders, &signature_shadepass, &signature_basepass, &pipelines_compiled]()
	{
		pipelines_compiled = false;

		// release old
		if (signature_basepass) dev.release(signature_basepass);
		if (signature_shadepass) dev.release(signature_shadepass);
		if (pipeline_basepass) dev.release(pipeline_basepass);
		if (pipeline_shadepass) dev.release(pipeline_shadepass);

		// compile shaders
		{
			// 1. parse all shaders in file
			shader::compile_args args{};
			args.m_signature.m_target = k_shadertarget; // force the shader target
			auto res = shader::parse_shaders_in_file(k_shaders_filepath);
			influx_assert(res.is_success());

			// 2. for each shader in file, compile
			for (uint32 i = 0u; i < 3u; ++i)
			{
				const auto& parse = res.get()[i];

				// args has already been partially filled in by parsing...
				args.m_signature = parse.m_signature;
				args.m_signature.m_target = k_shadertarget; // force the shader target
				args.m_reflection_enabled = true;
				args.m_debug_level;
				args.m_defines;
				args.m_add_args.push_back("-Wc++11-extensions");
				args.m_include_folder;
				args.m_pbd_enabled;
				args.m_pdb_filename;
				args.m_pdb_folder;
				auto comp_res = shader::compile_shader_in_file(k_shaders_filepath, args);
				influx_assert(comp_res.is_success());
				compiled_shaders[i] = comp_res.get();
			}
		}

		// setup signatures
		{
			graphics::rootsignature_desc basepass_desc{};
			graphics::rootsignature_desc shadepass_desc{};

			// bindless
			basepass_desc.m_direct_indexing = true;
			shadepass_desc.m_direct_indexing = true;

			// reflect shader resources into our signatures
			auto reflect_resource =
			[](graphics::rootsignature_desc& rootsig_desc, const shader::reflection::resource& resource, graphics::e_shader_visibility shader_vis)
			{
				switch (resource.m_type)
				{
				case shader::reflection::resource::e_type::rootvar: // constants
					rootsig_desc.add_root_constants((uint32)resource.m_bytesize / sizeof(uint32), resource.m_shader_register, resource.m_register_space, shader_vis);
					rootsig_desc.name_last_constants(resource.m_name);
					break;
				case shader::reflection::resource::e_type::cbv: // cbv
					rootsig_desc.add_root_resource(graphics::root_param_resource::e_type::cbv,
						resource.m_shader_register, resource.m_register_space, shader_vis);
					rootsig_desc.name_last_resource(resource.m_name);
					break;
				case shader::reflection::resource::e_type::structured: // srv
					rootsig_desc.add_root_range(graphics::root_param_resource_range::e_type::srv,
						resource.m_range_size, resource.m_shader_register, resource.m_register_space, shader_vis);
					rootsig_desc.name_last_resource_table(resource.m_name);
					break;
				case shader::reflection::resource::e_type::texture:
					rootsig_desc.add_root_range(graphics::root_param_resource_range::e_type::srv,
						resource.m_range_size, resource.m_shader_register, resource.m_register_space, shader_vis);
					rootsig_desc.name_last_resource_table(resource.m_name);
					break;
				case shader::reflection::resource::e_type::sampler:
					rootsig_desc.add_root_sampler(resource.m_shader_register, resource.m_register_space, shader_vis);
					rootsig_desc.name_last_sampler(resource.m_name);
					break;
				}
			};
			for (const auto& shader : compiled_shaders)
			{
				const shader::reflection& reflection = shader.m_reflection;
				for (const auto& resource : reflection.m_bound_resources)
				{
					switch (shader.m_signature.m_type)
					{
					case shader::e_shader_type::vs:
					case shader::e_shader_type::ps:
						reflect_resource(basepass_desc, resource, graphics::e_shader_visibility::all);
						break;
					case shader::e_shader_type::cs:
						reflect_resource(shadepass_desc, resource, graphics::e_shader_visibility::all);
						break;
					}
				}
			}

			signature_basepass = dev.create_rootsignature(basepass_desc);
			signature_shadepass = dev.create_rootsignature(shadepass_desc);
		}

		// setup pipelines
		{
			graphics::graphics_pipeline_desc desc{};

			// [input layout] (parsed from reflection)
			const shader::reflection& vs_reflection = compiled_shaders[0].m_reflection;
			for (uint32 i = 0u; i < vs_reflection.m_input_params.size(); ++i)
			{
				const shader::reflection::input_param& param = vs_reflection.m_input_params[i];

				// derive the format
				graphics::e_format format;
				switch (param.m_num_floats)
				{
				case 1u: format = graphics::e_format::r32; break;
				case 2u: format = graphics::e_format::rg32; break;
				case 3u: format = graphics::e_format::rgb32; break;
				case 4u: format = graphics::e_format::rgba32; break;
				default:
					influx_assert(false);
					break;
				}

				desc.add_input_element(
					param.m_semantic_name,
					param.m_semantic_index,
					format,
					0u,
					false,
					0u);
			}

			// [rasterizer]
			desc.m_prim_type = graphics::e_primitive_topology_type::triangle;
			desc.m_rasterizer.m_cullmode = graphics::e_cull_mode::nocull;
			desc.m_rasterizer.m_fillmode = graphics::e_fill_mode::solid;
			desc.m_rasterizer.m_forced_samplecount = 0u;
			desc.m_rasterizer.m_front_ccw = true;
			desc.m_rasterizer.m_multisample = false;
			desc.m_rasterizer.m_antialiased_line = false;
			desc.m_rasterizer.m_depth_clip_enable = false;
			desc.m_rasterizer.m_conservative = false;
			desc.m_rasterizer.m_depth_bias = 0;
			desc.m_rasterizer.m_depth_bias_clamp = 0.0f;
			desc.m_rasterizer.m_slope_depth_bias = 0.0f;

			// [sampler]
			desc.m_sample_mask = 0xffffffff;
			desc.m_sample_count = 1u;

			// [output merger]
			desc.m_depth_stencil.m_depth_enable = true;
			desc.m_depth_stencil.m_stencil_enable = false;
			desc.m_depth_stencil.m_depth_func = graphics::e_comparison_func::less;
			desc.m_depth_stencil.m_format = graphics::e_format::d32;
			for (uint32 i = 0u; i < 8u; ++i)
			{
				if (i < k_num_gbuffers)
				{
					desc.m_rtvs[i].m_enabled = true;
					desc.m_rtvs[i].m_format = k_gbuffer_formats[i];
					desc.m_blends[i].m_write_mask = 15u; // write-all
				}
				else desc.m_rtvs[i].m_enabled = false;
			}

			// [shaders]
			graphics::compute_pipeline_desc compute_desc{};
			for (const auto& shader : compiled_shaders)
			{
				switch (shader.m_signature.m_type)
				{
				case shader::e_shader_type::vs:
					desc.m_shaders.set(graphics::e_graphics_shader_slots::vs, shader.m_bytecode);
					break;

				case shader::e_shader_type::ps:
					desc.m_shaders.set(graphics::e_graphics_shader_slots::ps, shader.m_bytecode);
					break;

				case shader::e_shader_type::cs:
					compute_desc.m_shaders.set(graphics::e_compute_shader_slots::cs, shader.m_bytecode);
					break;
				}
			}
			pipeline_basepass = dev.create_graphics_pipeline(signature_basepass, desc);
			pipeline_shadepass = dev.create_compute_pipeline(signature_shadepass, compute_desc);
		}
		
		if (signature_basepass && signature_shadepass && pipeline_basepass && pipeline_shadepass)
			pipelines_compiled = true;
	};
	recompile_shaders();

	// create (& upload) non-rg textures
	vector<graphics::resource*> textures{};
	textures.reserve(loaded_images.size());
	{
		graphics::buffer_desc upload_desc{};
		upload_desc.m_bytesize = total_bytesize_loaded_images;
		upload_desc.m_init_state = graphics::e_resource_state::gen_read;
		graphics::resource* uploadheap = dev.create_resource(upload_desc, graphics::heap_desc::shared_heap());

		// create textures & srvs
		graphics::tex2D_desc desc{};
		desc.m_allow_uav = false;
		desc.m_arraysize = 1u;
		desc.m_bindflags = graphics::e_bind_flags::srv;
		desc.m_num_mips = 1u;
		desc.m_sample_count = 1u;
		for (uint64 i = 0u; i < loaded_images.size(); ++i)
		{
			const auto& image = loaded_images[i];
			desc.m_dimensions = image.m_dimensions;
			desc.m_format = graphics::e_format::rgba8;
			desc.m_init_state = graphics::e_resource_state::copy_dst;
			textures.push_back(dev.create_resource(desc, graphics::heap_desc{}));
		}

		cmdlist.start(&dev);
		for (uint64 i = 0u; i < loaded_images.size(); ++i)
		{
			const imp::image_data& img_data = loaded_images[i];
			const range<size_t> upload_subrange{ (i * img_data.m_bytesize), img_data.m_bytesize };

			// write to upload
			graphics::map_args args{};
			args.m_begin = upload_subrange.get_start();
			args.m_end = upload_subrange.get_end();
			uploadheap->map([&img_data](void* target)
			{
				memcpy(target, img_data.m_pixels.data(), img_data.m_bytesize);
			}, args);

			// copy upload->GPU
			graphics::copy_texture_args copy_args{};
			copy_args.m_src.m_range = upload_subrange;
			copy_args.m_dest.m_range = textures[i]->get_full_range();
			cmdlist.copy_texture(
				uploadheap, textures[i], copy_args);
		}
		cmdlist.end();
		queue.submit({ &cmdlist });
		cmdlist.wait_for_completion();
	}

	// create non-rg buffers
	graphics::resource* buff_instances = nullptr;
	graphics::resource* buff_lights = nullptr;
	graphics::resource* buff_vertices = nullptr;
	graphics::resource* buff_indices = nullptr;
	{
		// all buffers are stored on shared for upload convenience
		graphics::heap_desc heap_desc = graphics::heap_desc::shared_heap();
		
		// instance buffer
		{
			graphics::buffer_desc desc{};
			desc.m_bytestride = sizeof(frontend::per_instance);
			desc.m_bytesize = desc.m_bytestride * k_max_num_instances;
			desc.m_init_state = graphics::e_resource_state::gen_read;
			buff_instances = dev.create_resource(desc, heap_desc);
		}
		// lightbuffer
		{
			graphics::buffer_desc desc{};
			desc.m_bytestride = sizeof(frontend::per_dirlight);
			desc.m_bytesize = desc.m_bytestride * k_max_num_lights;
			buff_lights = dev.create_resource(desc, heap_desc);
		}
		// vertexbuffer
		{
			graphics::buffer_desc desc{};
			desc.m_bytestride = sizeof(frontend::per_vertex);
			desc.m_bytesize = desc.m_bytestride * k_num_vertices;
			buff_vertices = dev.create_resource(desc, heap_desc);
		}
		// indexbuffer
		{
			graphics::buffer_desc desc{};
			desc.m_format = graphics::e_format::u32;
			desc.m_bytestride = sizeof(uint32);
			desc.m_bytesize = desc.m_bytestride * k_num_indices;
			buff_indices = dev.create_resource(desc, heap_desc);
		}
	}

	// upload / update non-rg buffers
	{
		buff_instances->map<frontend::per_instance>([](frontend::per_instance* instances)
		{
			instances[0].m_colour = {1,1,1,1};
			instances[0].set_albedo_index(0u);
			instances[0].set_normal_index(1u);
			instances[0].m_transform = math::transform3D::identity().get_matrix();
		});
		buff_lights->map<frontend::per_dirlight>([](frontend::per_dirlight* lights)
		{
			lights[0].m_colour;
		});
		buff_vertices->map<frontend::per_vertex>([](frontend::per_vertex* vertices)
		{
			vertices[0].m_position = {0,0,0};
			vertices[1].m_position = {1,0,0};
			vertices[2].m_position = {0,1,0};
		});
		buff_indices->map<uint32>([](uint32* indices)
		{
			indices[0] = 0;
			indices[1] = 1;
			indices[2] = 2;
		});
	}

	// create final target
	graphics::resource* final_target = nullptr;
	{
		graphics::tex2D_desc desc{};
		desc.m_allow_uav = true;
		desc.m_arraysize = 1u;
		desc.m_bindflags = graphics::e_bind_flags::rtv | graphics::e_bind_flags::uav;
		desc.m_dimensions = window_desc.m_dimensions;
		desc.m_format = graphics::e_format::rgba8;
		desc.m_init_state = graphics::e_resource_state::render_target;
		desc.m_num_mips = 1u;
		desc.m_sample_count = 1u;
		final_target = dev.create_resource(desc);
		final_target->set_name("final_target");
	}

	// build frame rendergraph (deferred renderer)
	rendergraph::rendergraph graph{ {}, dev };
	{
		using namespace influx::rendergraph;
		graph.import_texture(final_target);
		graph.import_texture("tex_albedo", textures[0]);
		graph.import_texture("tex_normal", textures[1]);
		graph.import_buffer("buff_indices", buff_indices);
		graph.import_buffer("buff_vertices", buff_vertices);
		graph.import_buffer("srv_instances", buff_instances);
		
		// 1. graphics basepass
		auto basepass = graph.add_pass(e_rgpass_type::graphics,
			// [build]
			[&final_target](rgpass_builder& builder)
			{
				const math::uint2& target_dim = { final_target->get_width(), final_target->get_height() };

				// declare gbuffer rendertargets
				{
					texture_desc gbuffer_desc{};
					gbuffer_desc.m_width = target_dim.x;
					gbuffer_desc.m_heigth = target_dim.y;
					gbuffer_desc.m_format = graphics::e_format::rgba_u32;
					builder.declare_texture(gbuffernames[0], gbuffer_desc);
					gbuffer_desc.m_format = graphics::e_format::u32;
					builder.declare_texture(gbuffernames[1], gbuffer_desc);
					builder.declare_texture(gbuffernames[2], gbuffer_desc);
				}

				// declare cbvs
				{
					buffer_desc desc{};
					desc.m_shared_heap = true; // cpu can write to this
					desc.m_bytestride = desc.m_bytesize = sizeof(frontend::per_view);
					builder.declare_buffer("cb_per_view", desc);
					desc.m_bytestride = desc.m_bytesize = sizeof(frontend::per_material);
					builder.declare_buffer("cb_per_material", desc);
					desc.m_bytestride = desc.m_bytesize = sizeof(frontend::per_draw);
					builder.declare_buffer("cb_per_draw", desc);
				}

				// declare depth buffer
				{
					texture_desc dbuffer_desc{};
					dbuffer_desc.m_allow_uav = false;
					dbuffer_desc.m_array_size = 1;
					dbuffer_desc.m_bindflags = graphics::e_bind_flags::dsv;
					dbuffer_desc.m_depth = 1;
					dbuffer_desc.m_format = graphics::e_format::d32;
					dbuffer_desc.m_width = target_dim.x;
					dbuffer_desc.m_heigth = target_dim.y;
					dbuffer_desc.m_init_state = graphics::e_resource_state::depth_target;
					dbuffer_desc.m_num_mips = 1u;
					dbuffer_desc.m_sample_count = 1u;
					builder.declare_texture("depth_target", dbuffer_desc);
				}

				// finally, declare the rendergraph layout
				builder.read_constbuffer("cb_per_view").get();
				builder.read_constbuffer("cb_per_material").get();
				builder.read_constbuffer("cb_per_draw").get();
				builder.read_buffer("srv_instances").get();
				builder.read_texture("tex_albedo").get();
				builder.read_texture("tex_normal").get();

				rgaccess access = rgaccess::clear_and_keep({});
				builder.write_rendertarget(gbuffernames[0], access);
				builder.write_rendertarget(gbuffernames[1], access);
				builder.write_rendertarget(gbuffernames[2], access);
				builder.write_depthtarget("depth_target", access);
			},
			// [execute]
			[&signature_basepass, &pipeline_basepass, 
			buff_vertices, buff_indices, &camera, &cam_transform, &dev](rgpass_context& ctx)
			{
				graphics::commandlist& cmdlist = ctx.get_commandlist();
				graphics::descriptor_heap& gpu_resource_descheap = ctx.get_descheap_gpu(e_gpu_descheap::resource);
				graphics::descriptor_heap& gpu_sampler_descheap = ctx.get_descheap_gpu(e_gpu_descheap::sampler);

				// copy the cpu descriptor into the gpu-visible descriptor
				{
					auto tex_albedo = ctx.get_read_texture("tex_albedo").get();
					auto tex_normal = ctx.get_read_texture("tex_normal").get();
					auto buff_instance = ctx.get_read_buffer("srv_instances").get();
					graphics::descriptor_handle gpu_albedo = gpu_resource_descheap.get_cpu(frontend::k_albedo_id).get();
					graphics::descriptor_handle gpu_normal = gpu_resource_descheap.get_cpu(frontend::k_normals_id).get();
					graphics::descriptor_handle gpu_instance = gpu_resource_descheap.get_cpu(frontend::k_instancebuffer_id).get();
					dev.copy_descriptors(tex_albedo.m_descriptor, gpu_albedo, rhi_descheap_type::rsc);
					dev.copy_descriptors(tex_normal.m_descriptor, gpu_normal, rhi_descheap_type::rsc);
					dev.copy_descriptors(buff_instance.m_descriptor, gpu_instance, rhi_descheap_type::rsc);

					static bool done_once = false;
					graphics::descriptor_handle gpu_sampler = gpu_sampler_descheap.get_cpu(frontend::k_sampler_id).get();
					if (!done_once)
					{
						dev.create_sampler_view(gpu_sampler, nullptr);
						done_once = true;
					}
				}
				
				// bind the gpu descriptor heaps
				cmdlist.set_descriptorheaps(
				{
					&gpu_resource_descheap,
					&gpu_sampler_descheap
				});

				cmdlist.set_rootsignature(signature_basepass);
				cmdlist.set_pipeline(pipeline_basepass);
				cmdlist.set_primitive_topology(graphics::e_primitive_topology::trilist);
				
				graphics::resource* constbuffers[3] =
				{
					ctx.get_constbuffer("cb_per_view").get().m_resource,
					ctx.get_constbuffer("cb_per_material").get().m_resource,
					ctx.get_constbuffer("cb_per_draw").get().m_resource
				};

				constbuffers[0]->map<frontend::per_view>([&camera, &cam_transform](frontend::per_view* view)
				{
					auto vp = math::matrix4x4f::make_viewprojection_RH(
						cam_transform.get_position(),
						{ 0,0,-1 },
						camera.get_fov(),
						camera.get_aspect_ratio(),
						camera.get_nearplane(),
						camera.get_farplane());
					view->m_viewprojection = vp;
					view->m_other = { 0.5,0.5,0.5,0.5 };
					// view->m_viewprojection = vp;
				});
				constbuffers[1]->map<frontend::per_material>([](frontend::per_material* mat)
				{
					mat->m_colour = { 0,1,0,1 };
				});
				constbuffers[2]->map<frontend::per_draw>([](frontend::per_draw* draw)
				{
					draw->m_base_instance = 0u;
				});

				// bind cbvs
				for (uint32 i = 0u; i < 3u; ++i)
				{
					cmdlist.set_root_cbv(constbuffers[i], i, graphics::e_pipeline_type::graphics);
				}

				cmdlist.set_vertexbuffer(buff_vertices);
				cmdlist.set_indexbuffer(buff_indices);
				cmdlist.draw_indexed({
					.m_num_indexes_per_instance = k_num_indices,
					.m_num_instances = 1u,
					.m_start_index = 0u,
					.m_start_vertex = 0u,
					.m_start_instance = 0u
				});
			});
		basepass->set_name("basepass");
		
		// 2. compute shadepass
		auto shadepass = graph.add_pass(e_rgpass_type::compute,
			// [build]
			[&final_target](rgpass_builder& builder)
			{
				const math::uint2& target_dim = { final_target->get_width(), final_target->get_height() };

				// declare shading args CBV
				{
					buffer_desc desc{};
					desc.m_bytestride = sizeof(uint32); // ??
					desc.m_bytesize = sizeof(frontend::cs_shading_args);
					desc.m_shared_heap = true; // CPU can write
					builder.declare_buffer("cb_shading_args", desc);
				}

				// declare renderpass layout
				builder.read_constbuffer("cb_shading_args");
				builder.write_rendertarget(final_target, rgaccess::keep_and_keep());
				builder.write_texture("final_target");

				for (uint32 i = 0; i < k_num_gbuffers; ++i)
					builder.read_texture(gbuffernames[i]).get();
			},
			// [execute]
			[&final_target, &signature_shadepass, &pipeline_shadepass, &dev](rgpass_context& ctx)
			{
				graphics::commandlist& cmdlist = ctx.get_commandlist();
				graphics::descriptor_heap& gpu_resource_descheap = ctx.get_descheap_gpu(e_gpu_descheap::resource);
				graphics::descriptor_heap& gpu_sampler_descheap = ctx.get_descheap_gpu(e_gpu_descheap::sampler);
				{
					auto target_uav = ctx.get_write_texture("final_target").get();
					graphics::descriptor_handle gpu_target = gpu_resource_descheap.get_cpu(frontend::k_final_target_id).get();
					dev.copy_descriptors(target_uav.m_descriptor, gpu_target, rhi_descheap_type::rsc);
				}

				cmdlist.set_rootsignature(signature_shadepass, graphics::e_pipeline_type::compute);
				cmdlist.set_pipeline(pipeline_shadepass);
				const math::uint2& target_dim = { final_target->get_width(), final_target->get_height() };

				for (uint32 i = 0; i < k_num_gbuffers; ++i)
				{
					auto gbuffer = ctx.get_read_texture(gbuffernames[i]).get();
					graphics::descriptor_handle gpu_gbuffer = gpu_resource_descheap.get_cpu(frontend::k_gbuffer_id + i).get();
					dev.copy_descriptors(gbuffer.m_descriptor, gpu_gbuffer, rhi_descheap_type::rsc);
				}
					
				// update & bind cb_shading_args
				graphics::resource* constbuffer = ctx.get_constbuffer("cb_shading_args").get().m_resource;
				constbuffer->map<frontend::cs_shading_args>([](frontend::cs_shading_args* args)
				{
					args->m_buffer_desc_indices;
					args->m_camera_position;
					args->m_inv_projection;
					args->m_inv_viewprojection;
					args->m_num_lights = 0u;
					args->m_screen_size = { 1,1,0,0 };
					args->m_skybox_indices;
					args->m_texture_desc_indices;
				});
				cmdlist.set_root_cbv(constbuffer, 0u, graphics::e_pipeline_type::compute);

				graphics::dispatch_args args{};
				args.m_threadgroup_count.x = target_dim.x / THREAD_GROUP_SIZE;
				args.m_threadgroup_count.y = target_dim.y / THREAD_GROUP_SIZE;
				args.m_threadgroup_count.z = 1u;
				cmdlist.dispatch(args);
			});
		shadepass->set_name("shadepass");

		graph.build();
	}
	
	bool is_quit = false;
	uint64 frame = 0u;
#if 0
	std::thread log_thread = std::thread([&is_quit, &log_timings]()
	{
		while (!is_quit) log_timings();
	});
#endif
	while (!is_quit)
	{
		auto gpu_mem = dev.get_memory_info().get();
		gpu_memory_budget = gpu_mem.m_gpu_budget;
		gpu_memory_used = gpu_mem.m_gpu_usage;

		scope sc_frame{ "frame" };
		{
			scope sc_poll{ "poll" };
			window->poll_events(is_quit);
		}
		{
			scope sc_cmdstart{ "cmd_start" };
			cmdlist.start(&dev).get();
		}
		{
			scope sc_exe{ "graph_exe" };
			graph.execute(cmdlist, dev).get();
		}
		{
			scope sc_copy{ "cmd_finalcopy " };
			// copy final target into backbuffer
			graphics::resource* backbuffer = swapchain.get_current_backbuffer_resource().get();
			backbuffer->transition(cmdlist, graphics::e_resource_state::copy_dst);
			final_target->transition(cmdlist, graphics::e_resource_state::copy_src);
			graphics::copy_texture_args copy_args{};
			cmdlist.copy_texture(final_target, backbuffer, copy_args);
			backbuffer->transition(cmdlist, graphics::e_resource_state::present);
		}
		{
			scope sc_submit{ "cmd_submit" };
			cmdlist.end().get();
			queue.submit({ &cmdlist }).get();
		}
		{
			scope sc_present{ "cmd_present" };
			swapchain.present({}).get();
		}

		if (frame == 64u * 12u) 
			timings.reset();
		++frame;
	}
}