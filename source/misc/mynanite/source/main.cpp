// influx::core
#include "core/container/map.h"
// influx::platform
#include "influx_platform/window.h"
// influx::graphics
#include "influx_graphics/device.h"

// SDK 1.614.1
extern "C" { __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 614u; }
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
		graphics::descriptor_heap* m_descheaps[k_num_heaptypes];
		umap<graphics::resource*, descriptor_slots> m_resource_to_descriptors{};

	public:
		static constexpr graphics::e_descriptor_heap_type k_descheap_types[k_num_heaptypes]
		{
			graphics::e_descriptor_heap_type::rtv,
			graphics::e_descriptor_heap_type::dsv,
			graphics::e_descriptor_heap_type::sampler,
			graphics::e_descriptor_heap_type::srv,
			graphics::e_descriptor_heap_type::srv,
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
			graphics::descriptor_handle& get_cpu(e_descriptor desc)
			{
				return m_cpu_descriptors[static_cast<uint32>(desc)];
			}
			graphics::descriptor_handle& get_gpu(e_descriptor desc)
			{
				return m_gpu_descriptors[static_cast<uint32>(desc)];
			}
			void set_created(e_descriptor desc, bool created)
			{
				m_descriptor_created[static_cast<uint32>(desc)] = created;
			}
			graphics::descriptor_handle m_gpu_descriptors[k_num_descriptortypes]{};
			graphics::descriptor_handle m_cpu_descriptors[k_num_descriptortypes]{};
			bool m_descriptor_created[k_num_descriptortypes] = { false, false, false, false };
		};

		descriptor_manager(graphics::device& device)
		{
			// create descriptor heaps
			for (uint32 i = 0u; i < k_num_heaptypes; ++i)
			{
				e_descriptorheap type = static_cast<e_descriptorheap>(i);
				graphics::descriptor_heap::create_args heap_desc{};
				heap_desc.m_type = k_descheap_types[i];
				heap_desc.m_shader_visible = k_is_descheap_shader_visible[i];
				heap_desc.m_capacity = k_capacities[i];
				m_descheaps[i] = device.create_descriptor_heap(heap_desc);
			}
		}

		graphics::descriptor_heap& get_heap(e_descriptorheap type)
		{
			return *m_descheaps[static_cast<uint32>(type)];
		}

		graphics::descriptor_handle get_or_create_descriptor(
			graphics::device& device, 
			graphics::resource* resource,
			e_descriptor type, 
			bool recreate = false)
		{
			if (!m_resource_to_descriptors.contains(resource))
			{
				m_resource_to_descriptors[resource] = {};
			}

			if (m_resource_to_descriptors[resource].is_created(type))
			{
				// already created, return cpu
				return m_resource_to_descriptors[resource].get_cpu(type);
			}
			else
			{
				graphics::descriptor_handle& gpu_descriptor = m_resource_to_descriptors[resource].get_gpu(type);
				graphics::descriptor_handle& cpu_descriptor = m_resource_to_descriptors[resource].get_cpu(type);
				switch (type)
				{
				case e_descriptor::rtv:
					cpu_descriptor = get_heap(e_descriptorheap::rtv).allocate_cpu().get();
					if (resource->is_texture()) device.create_rtv(cpu_descriptor, resource);
					break;
				case e_descriptor::dsv:
					cpu_descriptor = get_heap(e_descriptorheap::dsv).allocate_cpu().get();
					if (resource->is_texture()) device.create_dsv(cpu_descriptor, resource);
					break;
				case e_descriptor::srv:
					cpu_descriptor = get_heap(e_descriptorheap::srv_uav_cpu).allocate_gpu().get();
					if (resource->is_texture()) device.create_texture_srv(cpu_descriptor, resource);
					else						device.create_buffer_srv(cpu_descriptor, resource);
					break;
				case e_descriptor::uav:
					cpu_descriptor = get_heap(e_descriptorheap::srv_uav_cpu).allocate_cpu().get();
					if (resource->is_texture()) device.create_texture_uav(cpu_descriptor, resource);
					else						device.create_buffer_uav(cpu_descriptor, resource);
					break;
				}

				// flag as created
				m_resource_to_descriptors[resource].set_created(type, true);

				return cpu_descriptor;
			}
		}
	};

private:
	graphics::device* m_device = nullptr;
	graphics::queue* m_queue = nullptr;
	graphics::commandlist* m_commandlist = nullptr;
	graphics::swapchain* m_swapchain = nullptr;
	descriptor_manager* m_descmanager = nullptr;

public:
	graphics_manager(const platform::window& window)
	{
		graphics::e_api_type type = graphics::e_api_type::dx12;
		m_device = graphics::device::create(type, {});
		m_queue = m_device->create_queue(graphics::queue_desc::default_graphics());
		m_swapchain = m_device->create_swapchain(m_queue, window);

		m_descmanager = new descriptor_manager(*m_device);
	}
	~graphics_manager() {} // whatever
	
	void render_start()
	{
		if (!m_commandlist)
			m_commandlist = m_device->create_graphics_commandlist();
		
		m_commandlist->start(m_device);
	}

	graphics::descriptor_handle get_or_create_descriptor(
		graphics::resource* resource,
		e_descriptor type,
		bool recreate = false)
	{
		return m_descmanager->get_or_create_descriptor(*m_device, resource, type, recreate);
	}

	graphics::resource& backbuffer()
	{
		return *m_swapchain->get_current_backbuffer_resource().get();
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
		m_swapchain->present(graphics::present_args{ .m_vsync = vsync });
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

		graphics.backbuffer().transition(&graphics.commandlist(), graphics::e_resource_state::render_target);
		
		// get or create an rtv
		graphics::descriptor_handle rtv_handle 
			= graphics.get_or_create_descriptor(&graphics.backbuffer(), graphics_manager::e_descriptor::rtv);

		graphics.commandlist().clear_rtv(rtv_handle, {1,0,0,1});
		graphics.backbuffer().transition(&graphics.commandlist(), graphics::e_resource_state::present);

		graphics.render_finish();
		graphics.present(false);
	}
}