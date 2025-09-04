// influx::core
#include "core/math/vector.h"
#include "core/string.h"
#include "core/scene/camera.h"
#include "core/math/transform.h"
#pragma region includes
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
// SDK 1.614.1
extern "C" { __declspec(dllexport) extern const influx::uint32 D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }
// STL
#include <iostream>
// shader frontend
namespace frontend
{
#include "E:/Git/Influx/source/misc/rendering/resources/shaders.hlsl"
}
#pragma endregion

// ==================================================================================================================
// Sample
// Rendergraph
// todo: GPU Skinning
// Instanced Rendering
// todo: skybox cubemapping
// todo: PBR lighting
// ==================================================================================================================

#define ENABLE_STATS 0

// [constants & settings]
using namespace influx;
class constants final
{
public:
	// https://sketchfab.com/3d-models/silver-soldier-animated-a8f0d843735047b2999fbe4a9d7a1245#download
	inline static const char* m_scene_filepath			= "E:/Git/Influx/source/misc/rendering/resources/soldier.fbx";
	inline static const char* m_shaders_filepath		= "E:/Git/Influx/source/misc/rendering/resources/shaders.hlsl";
	inline static const char* m_albedo_filepath			= "E:/Git/Influx/source/misc/rendering/resources/albedo.png";
	inline static const char* m_normal_filepath			= "E:/Git/Influx/source/misc/rendering/resources/normals.png";
	inline static const char* m_skybox_filepath_base	= "E:/Git/Influx/source/misc/rendering/resources/graycloud_";
	inline static const char* m_skybox_files[]			= { "bk", "dn", "ft", "lf", "rt", "up" };

	inline static const math::uint2 m_window_dim = { 640u, 480u };
	static const uint32 k_num_swapchain_buffers = 3u;
	static const shader::e_shader_target k_shadertarget = shader::e_shader_target::_6_6;
	
	static const uint32 k_max_num_lights = 512u;
	static const uint32 k_max_num_instances = 4096u;

	static const uint32 k_num_gbuffers = 3u;
	inline static const graphics::e_format k_gbuffer_formats[k_num_gbuffers]
	{
		graphics::e_format::rgba_u32,
		graphics::e_format::u32,
		graphics::e_format::u32,
	};


	inline static const rendergraph::rgname k_graph_tex_gbuffers[k_num_gbuffers]
	{
		"tex_gbufferA",
		"tex_gbufferB",
		"tex_gbufferC"
	};
	inline static const char* k_graph_cb_per_view = "cb_per_view";
	inline static const char* k_graph_cb_per_material = "cb_per_material";
	inline static const char* k_graph_cb_per_draw = "cb_per_draw";
	inline static const char* k_graph_cb_bones = "cb_bones";
	inline static const char* k_graph_cb_shade_args = "cb_shade_args";
	inline static const char* k_graph_srv_instances = "srv_instances";
	inline static const char* k_graph_tex_normals = "tex_normals";
	inline static const char* k_graph_tex_albedo = "tex_albedo";
	inline static const char* k_graph_cube_skybox = "cube_skybox";
	inline static const char* k_graph_srv_pointlights = "srv_pointlights";
	inline static const char* k_graph_srv_dirlights = "srv_dirlights";
	inline static const char* k_graph_tex_finaltarget = "tex_finaltarget";
	inline static const char* k_graph_buff_indices = "buff_indices";
	inline static const char* k_graph_buff_vertices = "buff_vertices";
};
struct settings final
{
	inline static float m_camera_distance = 10.0f;
	inline static math::rotation m_camera_rotation = {};
	inline static float m_camera_far = 1000.0f;
	inline static float m_camera_near = 0.001f;
	inline static float m_camera_fov = 110.0f;

	inline static math::float4 m_clear_colour{};

	inline static uint32 m_num_instances = 128u;

	static math::float3 get_camera_position()
	{
		return -get_camera_forward() * m_camera_distance;
	}
	static math::float3 get_camera_forward()
	{
		const math::float3 forward = m_camera_rotation.rotate({ 0,0,-1 }, 0.0f, math::float3::up());
		return forward;
	}
};

class stats
{
public:
	umap<const char*, float>	m_cpu_records{};
	umap<const char*, uint32>	m_cpu_record_nums{};
	uint64 m_gpu_memory_budget = 0u;
	uint64 m_gpu_memory_used = 0u;

	void log_cpu_timings()
	{
		std::cout << "\033[H\033[J"; // clear prev
		using pair = std::pair<const char*, float>;
		vector<pair> averages{};
		averages.reserve(m_cpu_records.size());

		// gather avgs
		for (const auto& pair : m_cpu_records)
		{
			const uint32 num = m_cpu_record_nums[pair.first];
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
		const float gpu_used = (float)m_gpu_memory_budget;
		const float gpu_budget = (float)m_gpu_memory_used;
		const float GB_used = gpu_used / (1024 * 1024 * 1024);
		const float GB_budget = gpu_budget / (1024 * 1024 * 1024);
		bar.pc() = gpu_used / gpu_budget;
		std::cout << "[VRAM] " << bar.get_cstr() << " " << std::setprecision(4) << GB_used << "/" << GB_budget << " GB \n";
		std::cout << std::flush;
		// std::this_thread::sleep_for(std::chrono::seconds(1)); // wait a second per log
	}
	void reset_cpu_timings()
	{
		for (const auto& pair : m_cpu_records)
		{
			m_cpu_records[pair.first] = 0.0f;
			m_cpu_record_nums[pair.first] = 0u;
		}
	}
};
static stats gstats{};
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
		gstats.m_cpu_records[m_name] += ms;
		gstats.m_cpu_record_nums[m_name]++;
	}
};
class content final
{
	umap<const char*, imp::scene_data> m_loaded_scenes{};
	umap<const char*, imp::image_data> m_loaded_images{};
	umap<const char*, graphics::resource*> m_uploaded_textures{};

	struct geometry_buffers { graphics::resource* m_indexbuffer{}; graphics::resource* m_vertexbuffer{}; };
	using mesh_vector = vector<geometry_buffers>;
	umap<const char*, mesh_vector> m_uploaded_meshes{};

public:
	const umap<const char*, imp::image_data>& get_images() const { return m_loaded_images; }

	result<imp::image_data> load(const char* filepath, const imp::image_load_args& args)
	{
		using result_type = result<imp::scene_data>;
		m_loaded_images[filepath] = imp::load_image_file(filepath, args).get();
		return m_loaded_images[filepath];
	}
	result<imp::scene_data> load(const char* filepath, const imp::scene_load_args& args)
	{
		using result_type = result<imp::scene_data>;
		m_loaded_scenes[filepath] = imp::load_scene_file(filepath, args).get();
		return m_loaded_scenes[filepath];
	}
	result<> register_mesh(const char* filepath, graphics::resource* indexbuffer, graphics::resource* vertexbuffer, uint32 index)
	{
		const uint64 old_size = m_uploaded_meshes[filepath].size();
		m_uploaded_meshes[filepath].resize(std::min(old_size, (uint64)index + 1));
		m_uploaded_meshes[filepath][index].m_indexbuffer = indexbuffer;
		m_uploaded_meshes[filepath][index].m_vertexbuffer = vertexbuffer;
	}
	result<> register_texture(const char* filepath, const graphics::resource* resource)
	{
		m_uploaded_textures[filepath] = resource;
	}
};
static content gcontent{};

int main()
{
	// setup core graphics objects
	graphics::device& dev = *graphics::device::create(graphics::e_api_type::dx12);

	// upload heap
	graphics::buffer_desc upload_desc{};
	upload_desc.m_bytesize = 1024u * 1024 * 8u; // 8MB should be fine...
	upload_desc.m_init_state = graphics::e_resource_state::gen_read;
	graphics::resource* uploadheap = dev.create_resource(upload_desc, graphics::heap_desc::shared_heap());

	std::thread loading_thread = std::thread([&dev, &uploadheap]()
	{
		imp::scene_load_args scene_load_args{};
		scene_load_args.m_bake_transforms = false;
		scene_load_args.m_pre_scale = 1.0f;
		auto scene = gcontent.load(constants::m_scene_filepath, scene_load_args).get();
		gcontent.load(constants::m_albedo_filepath, imp::image_load_args{}).get();
		gcontent.load(constants::m_normal_filepath, imp::image_load_args{}).get();
		for (uint32 i = 0u; i < 6u; ++i)
		{
			const string filepath = string(constants::m_skybox_filepath_base) + constants::m_skybox_files[i] + ".png";
			gcontent.load(filepath.c_str(), imp::image_load_args{}).get();
		}

		umap<graphics::resource*, range<uint64>> resource_to_upload_range{};

		// upload to GPU & returns a range inside the upload heap
		uint32* upload_base = reinterpret_cast<uint32*>(uploadheap->map(graphics::map_args{}));
		auto push_to_upload = [&upload_base](void* memory, const uint64 bytesize) -> influx::range<uint64>
			{
				memcpy(upload_base, memory, bytesize);
				upload_base += bytesize;
			};

		for (uint64 i = 0u; i < scene.get_meshes().size(); ++i)
		{
			const imp::scene_data::mesh& mesh = scene.get_meshes()[i];
			const uint64 num_vertices = mesh.m_positions.size();
			const uint64 num_indices = mesh.m_indices.size();
			const uint64 vertex_bytesize = num_vertices * sizeof(frontend::per_vertex);
			const uint64 index_bytesize = num_indices * sizeof(uint32);
			graphics::buffer_desc desc{}; desc.m_bytesize = vertex_bytesize; desc.m_bytestride = sizeof(frontend::per_vertex);
			graphics::resource* vertbuffer = dev.create_resource(desc, {});
			desc.m_format = graphics::e_format::u32;
			desc.m_bytestride = sizeof(uint32); desc.m_bytesize = desc.m_bytestride * num_indices;
			graphics::resource* indexbuffer = dev.create_resource(desc, {});
			gcontent.register_mesh(constants::m_scene_filepath, indexbuffer, vertbuffer, i);

			vector<frontend::per_vertex> vertex_data{};
			for (uint64 i = 0u; i < num_vertices; ++i) vertex_data.push_back({});
			resource_to_upload_range[vertbuffer] = push_to_upload((void*)vertex_data.data(), vertex_bytesize);

			vector<uint32> index_data{};
			for (uint64 i = 0u; i < num_indices; ++i) index_data.push_back({});
			resource_to_upload_range[indexbuffer] = push_to_upload((void*)index_data.data(), index_bytesize);
		}

		for (const auto& pair : gcontent.get_images())
		{
			const auto& image = pair.second;
			graphics::tex2D_desc desc{};
			desc.m_allow_uav = false;
			desc.m_arraysize = 1u;
			desc.m_bindflags = graphics::e_bind_flags::srv;
			desc.m_num_mips = 1u;
			desc.m_sample_count = 1u;
			desc.m_dimensions = image.m_dimensions;
			desc.m_format = graphics::e_format::rgba8;
			desc.m_init_state = graphics::e_resource_state::copy_dst; // upload -> GPU
			graphics::resource* resource = dev.create_resource(desc, {});
			gcontent.register_texture(pair.first, resource);
			resource_to_upload_range[resource] = push_to_upload((void*)image.m_pixels.data(), image.m_bytesize);
		}
	
		uploadheap->unmap({});

		//auto cmdlist = dev.create_graphics_commandlist();
	});
	
	loading_thread.join();

	// setup camera
	influx::camera camera{};
	math::transform3D cam_transform = math::transform3D::identity();
	{
		cam_transform.set_forward(settings::get_camera_forward());
		cam_transform.set_position(settings::get_camera_position());
		cam_transform.update_matrix();
		camera.set_farplane(settings::m_camera_far);
		camera.set_nearplane(settings::m_camera_near);
		camera.set_fov(settings::m_camera_fov);
		camera.set_aspect_ratio((float)constants::m_window_dim.x / (float)constants::m_window_dim.y);
	}

	// make platform window
	platform::window* window = nullptr;
	platform::window_desc window_desc{};
	{
		window_desc.m_dimensions = constants::m_window_dim;
		window_desc.m_name = "rendering";
		window = platform::window::create(window_desc);
	}

	graphics::queue& queue = *dev.create_queue();
	graphics::commandlist& cmdlist = *dev.create_graphics_commandlist();
	graphics::swapchain_desc swapchain_desc{};
	swapchain_desc.m_dimensions = window_desc.m_dimensions;
	swapchain_desc.m_format = graphics::e_format::rgba8;
	swapchain_desc.m_num_buffers = constants::k_num_swapchain_buffers;
	graphics::swapchain& swapchain = *dev.create_swapchain(&queue, *window, swapchain_desc);

	// load shaders / build pipelines
	vector<shader::compile_output> compiled_shaders{};
	compiled_shaders.resize(3u);
	std::atomic_bool pipelines_compiled = false;
	auto recompile_shaders = 
	[&dev, &loaded, &compiled_shaders, &pipelines_compiled]()
	{
		pipelines_compiled = false;

		// release old
		if (loaded.signature_basepass) dev.release(loaded.signature_basepass);
		if (loaded.signature_shadepass) dev.release(loaded.signature_shadepass);
		if (loaded.pipeline_basepass) dev.release(loaded.pipeline_basepass);
		if (loaded.pipeline_shadepass) dev.release(loaded.pipeline_shadepass);

		// compile shaders
		{
			// 1. parse all shaders in file
			shader::compile_args args{};
			args.m_signature.m_target = constants::k_shadertarget; // force the shader target
			auto res = shader::parse_shaders_in_file(constants::m_shaders_filepath);
			influx_assert(res.is_success());

			// 2. for each shader in file, compile
			for (uint32 i = 0u; i < 3u; ++i)
			{
				const auto& parse = res.get()[i];

				// args has already been partially filled in by parsing...
				args.m_signature = parse.m_signature;
				args.m_signature.m_target = constants::k_shadertarget; // force the shader target
				args.m_reflection_enabled = true;
				args.m_debug_level;
				args.m_defines;
				args.m_add_args.push_back("-Wc++11-extensions");
				args.m_include_folder;
				args.m_pbd_enabled;
				args.m_pdb_filename;
				args.m_pdb_folder;
				auto comp_res = shader::compile_shader_in_file(constants::m_shaders_filepath, args);
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

			loaded.signature_basepass = dev.create_rootsignature(basepass_desc);
			loaded.signature_shadepass = dev.create_rootsignature(shadepass_desc);
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
				default: break;
				}
#if 0
				switch (param.m_num_uints)
				{
				case 1u: format = graphics::e_format::r32; break;
				case 2u: format = graphics::e_format::; break;
				case 3u: format = graphics::e_format::rgb32; break;
				case 4u: format = graphics::e_format::rgba32; break;
				default: break;
				}
#endif
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
				if (i < constants::k_num_gbuffers)
				{
					desc.m_rtvs[i].m_enabled = true;
					desc.m_rtvs[i].m_format = constants::k_gbuffer_formats[i];
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
			loaded.pipeline_basepass = dev.create_graphics_pipeline(loaded.signature_basepass, desc);
			loaded.pipeline_shadepass = dev.create_compute_pipeline(loaded.signature_shadepass, compute_desc);
		}
		
		if (loaded.signature_basepass && loaded.signature_shadepass && loaded.pipeline_basepass && loaded.pipeline_shadepass)
			pipelines_compiled = true;
	};
	recompile_shaders();

	// create (& upload) non-rg textures
	vector<graphics::resource*> textures{};
	textures.reserve(loaded.loaded_images.size());
	{
		// create textures
		graphics::tex2D_desc desc{};
		desc.m_allow_uav = false;
		desc.m_arraysize = 1u;
		desc.m_bindflags = graphics::e_bind_flags::srv;
		desc.m_num_mips = 1u;
		desc.m_sample_count = 1u;
		for (uint64 i = 0u; i < loaded.loaded_images.size(); ++i)
		{
			const auto& image = loaded.loaded_images[i];
			desc.m_dimensions = image.m_dimensions;
			desc.m_format = graphics::e_format::rgba8;
			desc.m_init_state = graphics::e_resource_state::copy_dst;
			textures.push_back(dev.create_resource(desc, graphics::heap_desc{}));
		}

		cmdlist.start(&dev);
		for (uint64 i = 0u; i < loaded.loaded_images.size(); ++i)
		{
			const imp::image_data& img_data = loaded.loaded_images[i];
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
	textures[0]->set_name(constants::k_graph_tex_albedo);
	textures[1]->set_name(constants::k_graph_tex_normals);
	
	// create (& upload) skybox texture
	graphics::resource* skybox_texture = nullptr;
	{
		// upload resource
		graphics::buffer_desc upload_desc{};
		upload_desc.m_bytesize = loaded.total_bytesize_cubemap;
		upload_desc.m_init_state = graphics::e_resource_state::copy_src;
		graphics::resource* uploadheap = dev.create_resource(upload_desc, graphics::heap_desc::shared_heap());

		graphics::cubemap_desc desc{};
		desc.m_allow_uav = false;
		desc.m_bindflags;
		desc.m_dimensions = loaded.loaded_cubemap.m_dimensions;
		desc.m_format = graphics::e_format::rgba8;
		desc.m_init_state = graphics::e_resource_state::copy_dst;
		desc.m_num_mips = 1u;
		desc.m_sample_count = 1u;
		skybox_texture = dev.create_resource(desc);

		const uint64 bytesize_per_side = loaded.total_bytesize_cubemap / 6u;

		cmdlist.start(&dev);
		for (uint64 i = 0u; i < 6u; ++i)
		{
			const range<size_t> upload_subrange{ (i * bytesize_per_side), bytesize_per_side };

			// write to upload
			graphics::map_args args{};
			args.m_begin = upload_subrange.get_start();
			args.m_end = upload_subrange.get_end();
			uploadheap->map([&loaded, &bytesize_per_side](void* target)
			{
				memcpy(target, loaded.loaded_cubemap.m_pixels.data(), bytesize_per_side);
			}, args);

			// copy upload->GPU
			graphics::copy_texture_args copy_args{};
			copy_args.m_src.m_range = upload_subrange;
			copy_args.m_dest.m_range = upload_subrange;
			cmdlist.copy_texture(uploadheap, skybox_texture, copy_args).get();
		}
		cmdlist.end();
		queue.submit({ &cmdlist });
		cmdlist.wait_for_completion();
	}
	skybox_texture->set_name(constants::k_graph_cube_skybox);

	// create non-rg buffers
	graphics::resource* buff_vertices = nullptr;
	graphics::resource* buff_indices = nullptr;
	graphics::resource* buff_lights = nullptr;
	graphics::resource* buff_instances = nullptr;
	graphics::resource** all_buffers[] = { &buff_vertices, &buff_indices, &buff_lights, &buff_instances };
	{
		graphics::resource* buff_instances_upload = nullptr;
		graphics::resource* buff_lights_upload = nullptr;
		graphics::resource* buff_vertices_upload = nullptr;
		graphics::resource* buff_indices_upload = nullptr;
		{
			// all buffers are stored on shared for upload convenience
			graphics::heap_desc heap_desc = graphics::heap_desc::shared_heap();
			graphics::buffer_desc desc{};
			desc.m_init_state = graphics::e_resource_state::gen_read;

			// instance buffer
			{
				desc.m_bytestride = sizeof(frontend::per_instance);
				desc.m_bytesize = desc.m_bytestride * constants::k_max_num_instances;
				buff_instances_upload = dev.create_resource(desc, heap_desc);
			}
			// lightbuffer
			{
				desc.m_bytestride = sizeof(frontend::per_dirlight);
				desc.m_bytesize = desc.m_bytestride * constants::k_max_num_lights;
				buff_lights_upload = dev.create_resource(desc, heap_desc);
			}
			// vertexbuffer
			{
				desc.m_bytestride = sizeof(frontend::per_vertex);
				desc.m_bytesize = desc.m_bytestride * loaded.total_num_vertices;
				buff_vertices_upload = dev.create_resource(desc, heap_desc);
			}
			// indexbuffer
			{
				desc.m_format = graphics::e_format::u32;
				desc.m_bytestride = sizeof(uint32);
				desc.m_bytesize = desc.m_bytestride * loaded.total_num_indices;
				buff_indices_upload = dev.create_resource(desc, heap_desc);
			}
		}

		// upload / update non-rg buffers
		{
			buff_instances_upload->map<frontend::per_instance>([](frontend::per_instance* instances)
			{
				for (uint32 i = 0u; i < settings::m_num_instances; ++i)
				{
					instances[i].m_colour = { 1,1,1,1 };
					instances[i].set_albedo_index(0u);
					instances[i].set_normal_index(1u);
					instances[i].m_transform = math::transform3D::identity().get_matrix();

					// rotate & translate a bit
					instances[i].m_transform[1][1] = 0.0f;
					instances[i].m_transform[2][2] = 0.0f;
					instances[i].m_transform[2][1] = 1.0f;
					instances[i].m_transform[1][2] = -1.0f;

					const uint32 x = i / 64u;
					const uint32 z = i % 64u;
					instances[i].m_transform.set_translation({ x * 20.0f, -20.0f, -50 - z * 20.0f });
				}
			});
			buff_lights_upload->map<frontend::per_dirlight>([](frontend::per_dirlight* lights)
			{
				lights[0].m_colour;
			});
			buff_vertices_upload->map<frontend::per_vertex>([&loaded](frontend::per_vertex* vertices)
			{
				uint64 vertex_offset = 0u;
				for (const auto& mesh : loaded.loaded_scene.get_meshes())
				{
					for (uint64 i = 0u; i < mesh.m_positions.size(); ++i)
					{
						vertices[vertex_offset + i].m_position = mesh.m_positions[i];
						vertices[vertex_offset + i].m_normal = mesh.m_normals[i];
					}
					vertex_offset += mesh.m_positions.size();
				}
			});
			buff_indices_upload->map<uint32>([&loaded](uint32* indices)
			{
				uint64 index_offset = 0u;
				for (const auto& mesh : loaded.loaded_scene.get_meshes())
				{
					for (uint64 i = 0u; i < mesh.m_indices.size(); ++i)
					{
						indices[index_offset + i] = mesh.m_indices[i];
					}
					index_offset = mesh.m_indices.size();
				}
			});
		}

		// upload->GPU the static geometry buffers
		graphics::resource* all_upload_buffers[] = { buff_vertices_upload, buff_indices_upload, buff_lights_upload, buff_instances_upload };
		{
			// create buffers onto GPU heap (default)
			graphics::heap_desc heap_desc = graphics::heap_desc{};
			for (uint32 i = 0u; i < _countof(all_upload_buffers); ++i)
			{
				graphics::buffer_desc desc = {};
				desc.m_bytestride = all_upload_buffers[i]->get_bytestride();
				desc.m_bytesize = all_upload_buffers[i]->get_bytesize();
				desc.m_format = all_upload_buffers[i]->get_format();
				(*all_buffers[i]) = dev.create_resource(desc, heap_desc);
			}

			// GPU copy the data once
			cmdlist.start(&dev);
			for (uint32 i = 0u; i < _countof(all_buffers); ++i)
			{
				cmdlist.copy_buffer(all_upload_buffers[i], (*all_buffers[i]), (uint32)(*all_buffers[i])->get_bytesize());
			}
			cmdlist.end();
			queue.submit({ &cmdlist });
			cmdlist.wait_for_completion();
		}
	}

	buff_instances->set_name(constants::k_graph_srv_instances);
	buff_lights->set_name(constants::k_graph_srv_dirlights);
	buff_indices->set_name(constants::k_graph_buff_indices);
	buff_vertices->set_name(constants::k_graph_buff_vertices);

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

	// build frame rendergraph
	rendergraph::rendergraph graph{ {}, dev };
	{
		using namespace influx::rendergraph;

		// 1. graphics basepass
		auto basepass = graph.add_pass(e_rgpass_type::graphics,
			/*[BUILD]*/[](rgpass_builder& builder)
			{
				// [CBVs]
				buffer_desc desc{}; desc.m_shared_heap = true; // cpu can write to these
				desc.m_bytestride = desc.m_bytesize = sizeof(frontend::per_view);
				builder.read_constbuffer(constants::k_graph_cb_per_view, desc).get();
				desc.m_bytestride = desc.m_bytesize = sizeof(frontend::per_material);
				builder.read_constbuffer(constants::k_graph_cb_per_material, desc).get();
				desc.m_bytestride = desc.m_bytesize = sizeof(frontend::per_draw);
				builder.read_constbuffer(constants::k_graph_cb_per_material, desc).get();
				desc.m_bytestride = sizeof(frontend::cbones);
				desc.m_bytesize = desc.m_bytestride * MAX_NUM_BONES;
				builder.read_constbuffer(constants::k_graph_cb_bones, desc).get();
				
				// [SRVs]
				builder.read_buffer(constants::k_graph_srv_instances).get();
				builder.read_texture(constants::k_graph_tex_albedo).get();
				builder.read_texture(constants::k_graph_tex_normals).get();

				// [Rendertargets & Depthtarget]
				const math::uint2& target_dim = constants::m_window_dim;
				rgaccess access = rgaccess::clear_and_keep({});
				texture_desc gbuffer_desc{};
				gbuffer_desc.m_width = target_dim.x; gbuffer_desc.m_heigth = target_dim.y;
				gbuffer_desc.m_format = graphics::e_format::rgba_u32;
				builder.write_rendertarget(constants::k_graph_tex_gbuffers[0], gbuffer_desc, access);
				gbuffer_desc.m_format = graphics::e_format::u32;
				builder.write_rendertarget(constants::k_graph_tex_gbuffers[1], gbuffer_desc, access);
				builder.write_rendertarget(constants::k_graph_tex_gbuffers[2], gbuffer_desc, access);
			},
			/*[EXECUTE]*/[buff_vertices, buff_indices, &camera, &cam_transform, &dev, &loaded](rgpass_context& ctx)
			{
				graphics::commandlist& cmdlist = ctx.get_commandlist();
				graphics::descriptor_heap& gpu_resource_descheap = ctx.get_descheap_gpu(e_gpu_descheap::resource);
				graphics::descriptor_heap& gpu_sampler_descheap = ctx.get_descheap_gpu(e_gpu_descheap::sampler);

				// update the CBV data (mapped on each frame)
				{
					graphics::resource* constbuffers[] =
					{
						ctx.get_constbuffer("cb_per_view").get().m_resource,
						ctx.get_constbuffer("cb_per_material").get().m_resource,
						ctx.get_constbuffer("cb_per_draw").get().m_resource,
						ctx.get_constbuffer("cb_bones").get().m_resource
					};
					constbuffers[0]->map<frontend::per_view>([&camera, &cam_transform](frontend::per_view* view)
					{
						auto vp = math::matrix4x4f::make_viewprojection_RH(
							settings::get_camera_position(),
							settings::get_camera_forward(),
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
					constbuffers[3]->map<frontend::cbones>([](frontend::cbones* bones)
					{
						for (uint32 i = 0u; i < MAX_NUM_BONES; ++i)
						{
							bones->m_matrices[i] = math::matrix4x4f::identity();
						}
					});
					for (uint32 i = 0u; i < _countof(constbuffers); ++i)
					{
						cmdlist.set_root_cbv(constbuffers[i], i, graphics::e_pipeline_type::graphics);
					}
				}

				// copy the cpu descriptor into the gpu-visible descriptor
				{
					auto tex_albedo = ctx.get_read_texture("tex_albedo").get();
					auto tex_normal = ctx.get_read_texture("tex_normals").get();
					auto buff_instance = ctx.get_read_buffer("buff_instances").get();
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
				
				cmdlist.set_descriptorheaps({&gpu_resource_descheap, &gpu_sampler_descheap });
				cmdlist.set_rootsignature(loaded.signature_basepass);
				cmdlist.set_pipeline(loaded.pipeline_basepass);
				cmdlist.set_primitive_topology(graphics::e_primitive_topology::trilist);
				cmdlist.set_vertexbuffer(buff_vertices);
				cmdlist.set_indexbuffer(buff_indices);

				int vertex_offset = 0; uint32 index_offset = 0u;
				for (const auto& mesh : loaded.loaded_scene.get_meshes())
				{
					const uint32 num_inds	= (uint32)mesh.m_indices.size();
					const uint32 num_verts	= (uint32)mesh.m_positions.size();
					cmdlist.draw_indexed({
						.m_num_indexes_per_instance = num_inds,
						.m_num_instances = settings::m_num_instances,
						.m_start_index = index_offset,
						.m_start_vertex = vertex_offset,
						.m_start_instance = 0u
					});
					index_offset += num_inds;
					vertex_offset += num_verts;
				}
			});
		basepass->set_name("basepass");
		
		// 2. compute shadepass
		auto shadepass = graph.add_pass(e_rgpass_type::compute,
			/*[BUILD]*/ [](rgpass_builder& builder)
			{
				buffer_desc desc{};
				desc.m_bytestride = desc.m_bytesize = sizeof(frontend::cs_shading_args);
				desc.m_shared_heap = true; // CPU can write with map
				builder.read_constbuffer(constants::k_graph_cb_shade_args, desc).get();
				builder.read_buffer(constants::k_graph_srv_pointlights).get();
				builder.read_buffer(constants::k_graph_srv_dirlights).get();
				builder.read_texture(constants::k_graph_cube_skybox).get();
				builder.write_texture(constants::k_graph_tex_finaltarget).get();

				for (uint32 i = 0; i < constants::k_num_gbuffers; ++i)
				{
					builder.read_texture(constants::k_graph_tex_gbuffers[i]).get();
				}
			},
			/*[EXECUTE]*/[&final_target, &loaded, &dev, &skybox_texture](rgpass_context& ctx)
			{
				graphics::commandlist& cmdlist = ctx.get_commandlist();
				graphics::descriptor_heap& gpu_resource_descheap = ctx.get_descheap_gpu(e_gpu_descheap::resource);
				graphics::descriptor_heap& gpu_sampler_descheap = ctx.get_descheap_gpu(e_gpu_descheap::sampler);
				{
					auto target_uav = ctx.get_write_texture(constants::k_graph_tex_finaltarget).get();
					graphics::descriptor_handle gpu_target = gpu_resource_descheap.get_cpu(frontend::k_final_target_id).get();
					dev.copy_descriptors(target_uav.m_descriptor, gpu_target, rhi_descheap_type::rsc);
				}
				{
					auto skybox = ctx.get_read_texture(constants::k_graph_cube_skybox).get();
					graphics::descriptor_handle gpu_skybox = gpu_resource_descheap.get_cpu(frontend::k_skybox_id).get();
					dev.copy_descriptors(skybox.m_descriptor, gpu_skybox, rhi_descheap_type::rsc);
				}
				for (uint32 i = 0; i < constants::k_num_gbuffers; ++i)
				{
					auto gbuffer = ctx.get_read_texture(constants::k_graph_tex_gbuffers[i]).get();
					graphics::descriptor_handle gpu_gbuffer = gpu_resource_descheap.get_cpu(frontend::k_gbuffer_id + i).get();
					dev.copy_descriptors(gbuffer.m_descriptor, gpu_gbuffer, rhi_descheap_type::rsc);
				}

				cmdlist.set_rootsignature(loaded.signature_shadepass, graphics::e_pipeline_type::compute);
				cmdlist.set_pipeline(loaded.pipeline_shadepass);
				const math::uint2& target_dim = { final_target->get_width(), final_target->get_height() };

				// update & bind cb_shading_args
				graphics::resource* constbuffer = ctx.get_constbuffer(constants::k_graph_cb_shade_args).get().m_resource;
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
#if ENABLE_STATS
	std::thread log_thread = std::thread([&is_quit, &log_timings]()
	{
		while (!is_quit) log_timings();
	});
#endif
	while (!is_quit)
	{
		auto gpu_mem = dev.get_memory_info().get();
		gstats.m_gpu_memory_budget = gpu_mem.m_gpu_budget;
		gstats.m_gpu_memory_used = gpu_mem.m_gpu_usage;

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
			gstats.reset_cpu_timings();
		++frame;
	}
}