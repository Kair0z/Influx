
#include "core/basetypes.h"

// SDK 1.614.1
extern "C" { __declspec(dllexport) extern const influx::uint32 D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

// influx::platform
#include "influx_platform/window.h"
#include "influx_platform/monitor.h"

// influx::graphics
#include "influx_graphics/device.h"

// influx::core
#include "core/math/vectortools.h"
#include "core/math/random.h"
#include "core/time.h"

// influx::import
#include "influx_import.h"

using namespace influx;

void compile_shaders(graphics::raytracing_pipeline_desc& out_desc)
{
	vector<imp::shader_data> loaded_shaders{};

	shader::compile_args compile_args{};
	compile_args.set_target(shader::e_shader_target::_6_6);
	compile_args.m_reflection = false;

	compile_args.m_include_folder = "D:/Git/Influx/assets/engine/shaders/include/";
	bool load_success = imp::load_shader_file("D:/Git/Influx/assets/engine/shaders/source/raytracing.hlsl", loaded_shaders, compile_args);
	influx_assert(load_success);

	for (const imp::shader_data& shader : loaded_shaders)
	{
		out_desc.m_shaders.set(shader.m_type, shader.m_compile_result.m_bytecode);
	}
}

struct pipeline final
{
	graphics::rootsignature* m_rootsig = nullptr;
	graphics::raytracing_pipeline* m_pipeline = nullptr;
};
void create_pipeline(graphics::device& device, pipeline& out_pipeline)
{
	graphics::rootsignature_desc rootsig_desc{};
	rootsig_desc.add_root_range(graphics::root_param_resource_range::e_type::uav, 1u, 0u); // output uav
	// rootsig_desc.add_root_resource(graphics::root_param_resource::e_type::srv, 0u); // tlas srv

	graphics::raytracing_pipeline_desc ray_pipeline_desc{};
	ray_pipeline_desc.m_hitgroups.push_back({});
	compile_shaders(ray_pipeline_desc);
	out_pipeline.m_rootsig = device.create_rootsignature(rootsig_desc);
	out_pipeline.m_pipeline = device.create_raytracing_pipeline(out_pipeline.m_rootsig, ray_pipeline_desc);
}

struct mesh_buffers final
{
	graphics::resource* m_quad_vtx_buffer = nullptr;
	graphics::resource* m_cube_vtx_buffer = nullptr;
	graphics::resource* m_cube_idx_buffer = nullptr;
};
void create_meshbuffers(graphics::device& device, mesh_buffers& out_buffers)
{
	constexpr float quadVtx[] = { -1, 0, -1, -1, 0,  1, 1, 0, 1,
							 -1, 0, -1,  1, 0, -1, 1, 0, 1 };
	constexpr float cubeVtx[] = { -1, -1, -1, 1, -1, -1, -1, 1, -1, 1, 1, -1,
								 -1, -1,  1, 1, -1,  1, -1, 1,  1, 1, 1,  1 };
	constexpr short cubeIdx[] = { 4, 6, 0, 2, 0, 6, 0, 1, 4, 5, 4, 1,
								 0, 2, 1, 3, 1, 2, 1, 3, 5, 7, 5, 3,
								 2, 6, 3, 7, 3, 6, 4, 5, 6, 7, 6, 5 };

	auto make_and_upload = [&device](auto& data) -> graphics::resource*
	{
		graphics::buffer_desc buffer_desc{};
		buffer_desc.m_bytesize = sizeof(data);
		buffer_desc.m_bytestride = sizeof(data[0]);
		graphics::resource* resource = device.create_resource(buffer_desc, graphics::heap_desc::shared_heap());
		resource->map([&data](void* target)
		{
			memcpy(target, data, sizeof(data));
		});
		return resource;
	};

	out_buffers.m_quad_vtx_buffer = make_and_upload(quadVtx);
	out_buffers.m_cube_vtx_buffer = make_and_upload(cubeVtx);
	out_buffers.m_cube_idx_buffer = make_and_upload(cubeIdx);
}

int main()
{
	using namespace influx;

	// create window
	vector<platform::monitor> monitors = platform::monitor::query_monitors();
	platform::window_desc window_desc{};
	window_desc.m_dimensions = { 640u, 480u };
	const math::vectoru2& windowsize = window_desc.m_dimensions;
	const math::vectoru2 window_half_size = windowsize / 2;
	window_desc.m_name = "raytracing";
	platform::window& window = *platform::window::create(window_desc);

	// create graphics stuff
	graphics::device& device = *graphics::device::create(graphics::e_api_type::dx12);
	graphics::commandlist& commandlist = *device.create_graphics_commandlist();
	graphics::queue& queue = *device.create_queue();
	graphics::swapchain& swapchain = *device.create_swapchain(&queue, window);
	
	// we need only 1 single rtv allocated (backbuffer)
	graphics::descriptor_heap& rtv_heap = *device.create_descriptor_heap(graphics::descriptor_heap::create_rtv_heap(1u));
	graphics::descriptor_heap& uav_heap = *device.create_descriptor_heap(graphics::descriptor_heap::create_uav_heap(1u));
	graphics::descriptor_handle rtv_handle = rtv_heap.allocate_cpu();
	graphics::descriptor_handle uav_cpu_handle = uav_heap.allocate_cpu();
	graphics::descriptor_handle uav_gpu_handle = uav_heap.allocate_gpu();

	// create raytracing target
	graphics::resource* raytracing_target = nullptr;
	{
		graphics::tex2D_desc desc{};
		desc.m_allow_uav = true;
		desc.m_dimensions = swapchain.get_dimensions();
		desc.m_bindflags = graphics::e_bind_flags::uav;
		desc.m_init_state = graphics::e_resource_state::cs_uav;
		raytracing_target = device.create_resource(desc);
		device.create_texture_uav(uav_cpu_handle, raytracing_target);
	}

	// create mesh buffers
	mesh_buffers mesh_buffers{};
	create_meshbuffers(device, mesh_buffers);

	// create pipeline
	pipeline pipeline{};
	create_pipeline(device, pipeline);

	graphics::present_args present_args{};
	present_args.m_vsync = false;
	time::point time_last_tick = time::get_now();
	float delta_seconds = 0.0f;
	float seconds = 0.0f;
	bool is_quit = false;
	while (is_quit == false)
	{
		delta_seconds = time::get_ms_since<float>(time_last_tick) * 0.001f;
		time_last_tick = time::get_now();
		seconds += delta_seconds;

		window.poll_events(is_quit);

		commandlist.start(&device);
		
		// set viewport & rect
		commandlist.set_vp_and_rect
		(
			{ 0.0f, 0.0f },			// min
			{ 640.0f , 480.0f }		// max
		);

		graphics::resource* backbuffer = swapchain.get_current_backbuffer_resource().get();
		// backbuffer->transition(&commandlist, graphics::e_resource_state::render_target);
		// create backbuffer rtv (ideally don't recreate each frame, but on DX12, this is cheap)
		// device.create_rtv(rtv_handle, backbuffer);
		// commandlist.set_rtv(rtv_handle, nullptr);
		// commandlist.clear_rtv(rtv_handle, { 1,0,0,1 });

		// dispatch rays
		raytracing_target->transition(&commandlist, graphics::e_resource_state::cs_uav);
		commandlist.set(pipeline.m_rootsig, graphics::e_pipeline_type::raytracing);
		commandlist.set(pipeline.m_pipeline);
		commandlist.set(&uav_heap);
		commandlist.set(uav_gpu_handle, 0u, graphics::e_pipeline_type::raytracing);
		commandlist.dispatch_rays(pipeline.m_pipeline,
			raytracing_target->get_width(), raytracing_target->get_height());

		// copy raytracetarget -> backbuffer
		backbuffer->transition(&commandlist, graphics::e_resource_state::copy_dst);
		raytracing_target->transition(&commandlist, graphics::e_resource_state::copy_src);
		commandlist.copy_resource(raytracing_target, backbuffer);

		backbuffer->transition(&commandlist, graphics::e_resource_state::present);

		commandlist.submit(&queue);
		swapchain.present(present_args);
	}

	// cleanup..
}