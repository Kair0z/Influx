// influx::core
#include "core/container/map.h"
// influx::platform
#include "influx_platform/window.h"
// influx::rhi
#include "influx_rhi.h"

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
		rhi::descheap m_descheaps[k_num_heaptypes];
		umap<rhi::object_native, descriptor_slots> m_resource_to_descriptors{};

	public:
		static constexpr rhi::e_descriptor_heap_type k_descheap_types[k_num_heaptypes]
		{
			rhi::e_descriptor_heap_type::rtv,
			rhi::e_descriptor_heap_type::dsv,
			rhi::e_descriptor_heap_type::sampler,
			rhi::e_descriptor_heap_type::resource,
			rhi::e_descriptor_heap_type::resource,
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
			rhi::descriptor& get_cpu(e_descriptor desc)
			{
				return m_cpu_descriptors[static_cast<uint32>(desc)];
			}
			rhi::descriptor& get_gpu(e_descriptor desc)
			{
				return m_gpu_descriptors[static_cast<uint32>(desc)];
			}
			void set_created(e_descriptor desc, bool created)
			{
				m_descriptor_created[static_cast<uint32>(desc)] = created;
			}
			rhi::descriptor m_gpu_descriptors[k_num_descriptortypes]{};
			rhi::descriptor m_cpu_descriptors[k_num_descriptortypes]{};
			bool m_descriptor_created[k_num_descriptortypes] = { false, false, false, false };
		};

		descriptor_manager(rhi::device& device)
		{
			// create descriptor heaps
			for (uint32 i = 0u; i < k_num_heaptypes; ++i)
			{
				e_descriptorheap type = static_cast<e_descriptorheap>(i);
				rhi::descheap::create_args heap_desc{};
				heap_desc.m_type = k_descheap_types[i];
				heap_desc.m_shader_visible = k_is_descheap_shader_visible[i];
				heap_desc.m_num_descriptors = k_capacities[i];
				m_descheaps[i] = device.create(heap_desc).get();
			}
		}

		rhi::descheap& get_heap(e_descriptorheap type)
		{
			return m_descheaps[static_cast<uint32>(type)];
		}

		rhi::descriptor get_or_create_descriptor(
			rhi::device& device, 
			rhi::resource& resource,
			e_descriptor type, 
			bool recreate = false)
		{
			rhi::object_native native_resource = resource.get_native_resource();
			if (!m_resource_to_descriptors.contains(native_resource))
			{
				m_resource_to_descriptors[native_resource] = {};
			}

			if (m_resource_to_descriptors[native_resource].is_created(type))
			{
				// already created, return cpu
				return m_resource_to_descriptors[native_resource].get_cpu(type);
			}
			else
			{
				rhi::descriptor cpu_descriptor = m_resource_to_descriptors[native_resource].get_cpu(type);
				switch (type)
				{
				case e_descriptor::rtv:
					cpu_descriptor = get_heap(e_descriptorheap::rtv).allocate(1u).get();
					if (resource.is_texture()) device.create_rtv(native_resource, cpu_descriptor);
					break;
				case e_descriptor::dsv:
					cpu_descriptor = get_heap(e_descriptorheap::dsv).allocate(1u).get();
					if (resource.is_texture()) device.create_dsv(native_resource, cpu_descriptor);
					break;
				case e_descriptor::srv:
					cpu_descriptor = get_heap(e_descriptorheap::srv_uav_cpu).allocate(1u).get();
					if (resource.is_texture()) device.create_srv_texture(native_resource, cpu_descriptor);
					else						device.create_srv_buffer(native_resource, cpu_descriptor);
					break;
				case e_descriptor::uav:
					cpu_descriptor = get_heap(e_descriptorheap::srv_uav_cpu).allocate(1u).get();
					if (resource.is_texture()) device.create_uav_texture(native_resource, cpu_descriptor);
					else						device.create_uav_buffer(native_resource, cpu_descriptor);
					break;
				}

				// flag as created
				m_resource_to_descriptors[native_resource].set_created(type, true);

				return cpu_descriptor;
			}
		}
	};

private:
	rhi::device m_device;
	rhi::queue m_queue;
	rhi::commandlist m_commandlist;
	rhi::swapchain m_swapchain;
	descriptor_manager* m_descmanager;

public:
	graphics_manager(const platform::window& window)
	{
		rhi::e_api type = rhi::e_api::d3d12;
		m_device = rhi::device::create().get();
		m_queue = m_device.create(rhi::queue::default_graphics()).get();
		
		rhi::swapchain_create_args swapchain_args{};
		swapchain_args.m_device = &m_device;
		swapchain_args.m_dimensions = window.get_dimensions();
		swapchain_args.m_format = rhi::pixelformat::rgba_8_unorm();
		swapchain_args.m_num_buffers = 3u;
		swapchain_args.m_own_descriptors = true;
		swapchain_args.m_queue = &m_queue;
		swapchain_args.m_window = window.get_platform_handle();
		m_swapchain = m_device.create(swapchain_args).get();

		m_descmanager = new descriptor_manager(m_device);
	}
	~graphics_manager() {} // whatever
	
	void render_start()
	{
		if (!m_commandlist.is_valid())
			m_commandlist = m_device.create(rhi::commandlist::default_graphics()).get();
		
		m_commandlist.start(m_device);
	}

	rhi::descriptor get_or_create_descriptor(
		rhi::resource& resource,
		e_descriptor type,
		bool recreate = false)
	{
		return m_descmanager->get_or_create_descriptor(m_device, resource, type, recreate);
	}

	rhi::resource& backbuffer()
	{
		return m_swapchain.get_backbuffer_resource().get();
	}

	rhi::descriptor backbuffer_rtv()
	{
		return m_swapchain.get_or_create_backbuffer_rtv(m_device).get();
	}

	rhi::commandlist& commandlist()
	{
		return m_commandlist;
	}

	void render_finish()
	{
		m_commandlist.end();
		m_queue.submit({ &m_commandlist });
	}

	void present(bool vsync)
	{
		m_swapchain.present({});
	}
};

int main()
{
	platform::window* window	= create_window();
	graphics_manager graphics	= graphics_manager(*window);

	bool is_exit = false;
	while (!is_exit)
	{
		window->poll_events(is_exit);

		graphics.render_start();

		graphics.backbuffer().transition(graphics.commandlist(), rhi::e_resource_state::rendertarget);
		
		// get or create an rtv
		rhi::descriptor rtv_handle = graphics.backbuffer_rtv();

		graphics.commandlist().clear_rtv(rtv_handle, rhi::clear::colour({ 1,0,0,1 }));
		graphics.backbuffer().transition(graphics.commandlist(), rhi::e_resource_state::present);

		graphics.render_finish();
		graphics.present(false);
	}
}