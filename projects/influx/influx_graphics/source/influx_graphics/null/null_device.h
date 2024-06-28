#include "graphics_pch.h"
#include "influx_graphics/device.h"

// null includes
// ...

namespace influx::graphics
{
	class null_device final
		: public device
	{
	public:
		inline virtual vector<physical_device_info> get_gpu_infos()
		{
			return {};
		}

		inline virtual command_queue* create_command_queue(const command_queue_desc& desc)
		{
			return nullptr;
		}

		inline virtual swapchain* create_swapchain(command_queue* queue, const platform::window_handle& window)
		{
			return nullptr;
		}

		inline virtual descriptor_heap* create_descriptor_heap(const descriptor_heap::create_args&)
		{
			return nullptr;
		}

		inline virtual command_allocator* create_graphics_allocator()
		{
			return nullptr;
		}

		inline virtual command_list* create_graphics_command_list(command_allocator* allocator, pipeline_state* init_state = nullptr)
		{
			return nullptr;
		}

		inline virtual fence* create_fence()
		{
			return nullptr;
		}

		inline virtual resource* create_resource(const tex2D_desc& desc)
		{
			return nullptr;
		}

		inline virtual render_target_view* create_rtv()
		{
			return nullptr;
		}
	};
}