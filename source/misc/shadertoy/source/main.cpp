// influx::core
#include "core/container/map.h"
// influx::platform
#include "influx_platform/window.h"
// influx::graphics
#include "influx_graphics.h"
// influx::shader
#include "influx_shader.h"

// SDK 1.614.1
extern "C" { __declspec(dllexport) extern const influx::uint32 D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

using namespace influx;
inline platform::window* create_window()
{
	platform::window_desc win_desc{};
	win_desc
		.set_dimensions({ 640u, 480u })
		.set_name("mynanite");
	return platform::window::create(win_desc);
}

class graphics_manager final
{
	using descheap = graphics::descriptor_heap;
	using descheap_ptr = graphics::descriptor_heap*;
	using descriptor = graphics::descriptor_handle;

public:
	enum class e_descriptor : uint8
	{
		rtv, dsv, srv, uav, num
	};
	static constexpr uint32 k_num_descriptortypes = static_cast<uint32>(e_descriptor::num);

	class descriptor_manager final
	{
	public:
		enum class e_descriptorheap : uint8
		{
			rtv,
			dsv,
			sampler,
			srv_uav_cpu,
			srv_uav_gpu,
			num
		};
		static constexpr uint32 k_num_heaptypes = static_cast<uint32>(e_descriptorheap::num);

	private:
		struct descriptor_slots;
		descheap_ptr m_descheaps[k_num_heaptypes];
		umap<graphics::resource*, descriptor_slots> m_resource_to_descriptors{};

	public:
		static constexpr graphics::e_descriptor_heap_type k_descheap_types[k_num_heaptypes]
		{
			graphics::e_descriptor_heap_type::rtv,
			graphics::e_descriptor_heap_type::dsv,
			graphics::e_descriptor_heap_type::sampler,
			graphics::e_descriptor_heap_type::rsc,
			graphics::e_descriptor_heap_type::rsc,
		};
		static constexpr bool k_is_descheap_shader_visible[k_num_heaptypes]
		{
			false,	// rtv
			false,	// dsv
			false,	// sampler
			false,	// cpu_srv
			true	// gpu_srv
		};
		static constexpr uint32 k_capacities[k_num_heaptypes]
		{
			3u,		// rtv
			3u,		// dsv
			1u,		// sampler
			32u,	// cpu_srv
			32u		// gpu_srv
		};

		struct descriptor_slots final
		{
			bool is_created(e_descriptor desc)
			{
				return m_descriptor_created[static_cast<uint32>(desc)];
			}
			descriptor& get_cpu(e_descriptor desc)
			{
				return m_cpu_descriptors[static_cast<uint32>(desc)];
			}
			descriptor& get_gpu(e_descriptor desc)
			{
				return m_gpu_descriptors[static_cast<uint32>(desc)];
			}
			void set_created(e_descriptor desc, bool created)
			{
				m_descriptor_created[static_cast<uint32>(desc)] = created;
			}
			descriptor m_gpu_descriptors[k_num_descriptortypes]{};
			descriptor m_cpu_descriptors[k_num_descriptortypes]{};
			bool m_descriptor_created[k_num_descriptortypes] = { false, false, false, false };
		};

		descriptor_manager(graphics::device& device)
		{
			// create descriptor heaps
			for (uint32 i = 0u; i < k_num_heaptypes; ++i)
			{
				e_descriptorheap type = static_cast<e_descriptorheap>(i);
				descheap::create_args heap_desc{};
				heap_desc.m_type = k_descheap_types[i];
				heap_desc.m_shader_visible = k_is_descheap_shader_visible[i];
				heap_desc.m_capacity = k_capacities[i];
				m_descheaps[i] = device.create_descriptor_heap(heap_desc);
			}
		}

		descheap& get_heap(e_descriptorheap type)
		{
			return *m_descheaps[static_cast<uint32>(type)];
		}

		descriptor get_or_create_descriptor(
			graphics::device& device, 
			graphics::resource& resource,
			e_descriptor type,
			bool get_cpu_descriptor = true,
			bool recreate = false)
		{
			graphics::resource* native_resource = &resource;
			if (!m_resource_to_descriptors.contains(native_resource))
			{
				m_resource_to_descriptors[native_resource] = {};
			}

			if (m_resource_to_descriptors[native_resource].is_created(type))
			{
				// already created, return cpu
				return get_cpu_descriptor ? m_resource_to_descriptors[native_resource].get_cpu(type)
					: m_resource_to_descriptors[native_resource].get_gpu(type);
			}
			else
			{
				descriptor cpu_descriptor = m_resource_to_descriptors[native_resource].get_cpu(type);
				descriptor gpu_descriptor = m_resource_to_descriptors[native_resource].get_gpu(type);
				switch (type)
				{
				case e_descriptor::rtv:
				{
					auto& heap = get_heap(e_descriptorheap::rtv);
					const uint32 index = heap.allocate().get();
					cpu_descriptor = heap.get_cpu(index).get();
					gpu_descriptor = heap.get_gpu(index).get();
					if (resource.is_texture()) device.create_rtv(native_resource, cpu_descriptor);
					break;
				}
				case e_descriptor::dsv:
				{
					auto& heap = get_heap(e_descriptorheap::dsv);
					const uint32 index = heap.allocate().get();
					cpu_descriptor = heap.get_cpu(index).get();
					gpu_descriptor = heap.get_gpu(index).get();
					if (resource.is_texture()) device.create_dsv(native_resource, cpu_descriptor);
					break;
				}
				case e_descriptor::srv:
				{
					auto& heap = get_cpu_descriptor ? get_heap(e_descriptorheap::srv_uav_cpu) : get_heap(e_descriptorheap::srv_uav_gpu);
					const uint32 index = heap.allocate().get();
					cpu_descriptor = heap.get_cpu(index).get();
					gpu_descriptor = heap.get_gpu(index).get();
					if (resource.is_texture()) device.create_srv_texture(native_resource, cpu_descriptor);
					else						device.create_srv_buffer(native_resource, cpu_descriptor);
					break;
				}
				case e_descriptor::uav:
				{
					auto& heap = get_cpu_descriptor ? get_heap(e_descriptorheap::srv_uav_cpu) : get_heap(e_descriptorheap::srv_uav_gpu);
					const uint32 index = heap.allocate().get();
					cpu_descriptor = heap.get_cpu(index).get();
					gpu_descriptor = heap.get_gpu(index).get();
					if (resource.is_texture()) device.create_uav_texture(native_resource, cpu_descriptor);
					else						device.create_uav_buffer(native_resource, cpu_descriptor);
					break;
				}	
				}

				// flag as created
				m_resource_to_descriptors[native_resource].set_created(type, true);

				return get_cpu_descriptor ? cpu_descriptor : gpu_descriptor;
			}
		}
	};

private:
	graphics::device*				m_device;
	graphics::queue*				m_queue;
	graphics::commandlist*			m_commandlist = nullptr;
	graphics::swapchain*			m_swapchain;
	graphics::resource*				m_target;
	graphics::compute_pipeline*		m_pipeline = nullptr;
	graphics::rootsignature*		m_rootsignature = nullptr;
	descriptor_manager*				m_descmanager;

	descriptor m_target_uav{};

public:
	graphics_manager(const platform::window& window)
	{
		graphics::e_api_type type = graphics::e_api_type::dx12;
		m_device = graphics::device::create({});
		m_queue = m_device->create_queue(graphics::queue_desc::default_graphics());
		
		graphics::swapchain_desc swapchain_args{};
		swapchain_args.m_dimensions = window.get_dimensions();
		swapchain_args.m_format = graphics::e_format::rgba8;
		swapchain_args.m_num_buffers = 3u;
		m_swapchain = m_device->create_swapchain(m_queue, window, swapchain_args);
		m_descmanager = new descriptor_manager(*m_device);

		graphics::tex2D_desc desc{};
		desc.m_allow_uav = true;
		desc.m_arraysize = 1u;
		desc.m_bindflags = graphics::e_bind_flags::uav;
		desc.m_dimensions = window.get_dimensions();
		desc.m_format = graphics::e_format::rgba8;
		desc.m_init_state = graphics::e_resource_state::cs_uav;
		desc.m_num_mips = 1u;
		desc.m_sample_count = 1u;
		m_target = m_device->create_resource(desc);

		m_target_uav = get_or_create_descriptor(*m_target, e_descriptor::uav, false);
	}
	~graphics_manager() {} // whatever
	
	void render_start()
	{
		if (m_commandlist == nullptr || !m_commandlist->is_valid())
			m_commandlist = m_device->create_graphics_commandlist();
			
		m_commandlist->wait_for_completion();
		m_commandlist->start(m_device, m_pipeline);
		m_commandlist->set_descriptorheap(&m_descmanager->get_heap(descriptor_manager::e_descriptorheap::srv_uav_gpu));
		m_commandlist->set_rootsignature(m_rootsignature, graphics::e_pipeline_type::compute);
	}

	bool rebuild_pipeline(const shader::compile_output& compiled_shader)
	{
		if (m_pipeline) m_device->release(m_pipeline);
		if (m_rootsignature) m_device->release(m_rootsignature);

		graphics::rootsignature_desc rootsig_desc{};;
		rootsig_desc.add_root_range(graphics::root_param_resource_range::e_type::uav, 1u, 0u);
		m_rootsignature = m_device->create_rootsignature(rootsig_desc);

		influx::graphics::compute_pipeline_desc desc{};
		desc.m_shaders.set(graphics::e_compute_shader_slots::cs, compiled_shader.m_bytecode);
		m_pipeline = m_device->create_compute_pipeline(m_rootsignature, desc);
		return true;
	}

	descriptor get_or_create_descriptor(
		graphics::resource& resource,
		e_descriptor type,
		bool get_cpu = true,
		bool recreate = false)
	{
		return m_descmanager->get_or_create_descriptor(*m_device, resource, type, get_cpu, recreate);
	}

	graphics::resource& target()
	{
		return *m_target;
	}

	graphics::resource& backbuffer()
	{
		return *m_swapchain->get_current_backbuffer_resource().get();
	}

	descriptor target_uav_gpu()
	{
		return m_target_uav;
	}

	graphics::commandlist& commandlist()
	{
		return *m_commandlist;
	}

	void render_finish()
	{
		m_commandlist->end();
		m_queue->submit({ m_commandlist });
	}

	void present(bool vsync)
	{
		m_swapchain->present({});
	}
};

int main()
{
	platform::window* window	= create_window();
	graphics_manager graphics	= graphics_manager(*window);

	bool is_exit = false;
	bool is_pipeline_valid = false;
	std::thread commandline_thread = std::thread(
	[&graphics, &is_exit, &is_pipeline_valid]()
	{
		while (!is_exit)
		{
			string command;
			std::cin >> command;

			if (!command.empty() && str::contains(command, "recomp", false))
			{
				// do the recompile
				is_pipeline_valid = false;

				const string filepath = "C:/Users/arnev/OneDrive/Bureaublad/shader.hlsl";
				auto parsed = shader::parse_shaders_in_file(filepath);
				if (parsed.is_fail())
					continue;

				if (!has_flag(parsed.get().m_found_types, shader::e_shader_type_flags::cs))
					continue;

				const auto& shaders = parsed.get().m_shadermap[shader::e_shader_type::cs];
				shader::compile_args args{};
				args.m_reflection_enabled = true;
				args.m_target = shader::e_shader_target::_6_6;
				auto compiled = shader::compile_shader_in_file(filepath, shaders[0].m_signature, args);
				if (compiled.is_fail())
					continue;

				bool result = graphics.rebuild_pipeline(compiled.get());
				is_pipeline_valid = result;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}
	});

	while (!is_exit)
	{
		window->poll_events(is_exit);

		if (is_pipeline_valid)
		{
			graphics.render_start();
			auto& cmdlist = graphics.commandlist();
			graphics.backbuffer().transition(cmdlist, graphics::e_resource_state::render_target);

			cmdlist.set_descriptor_range(graphics.target_uav_gpu(), 0u, graphics::e_pipeline_type::compute);

			graphics::dispatch_args args{};
			args.m_threadgroup_count = { 32,32,1 };
			cmdlist.dispatch(args);

			graphics.backbuffer().transition(cmdlist, graphics::e_resource_state::copy_dst);
			graphics.target().transition(cmdlist, graphics::e_resource_state::copy_src);
			cmdlist.copy_resource(graphics.target(), graphics.backbuffer());
			graphics.backbuffer().transition(cmdlist, graphics::e_resource_state::present);
			graphics.render_finish();
		}

		graphics.present(false);
	}

	commandline_thread.join();
}