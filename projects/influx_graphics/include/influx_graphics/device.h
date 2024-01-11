#pragma once
#include "influx_graphics.h"
#include "influx_graphics/base.h"
#include "influx_graphics/commandqueue.h"
#include "influx_graphics/commandlist.h"
#include "influx_graphics/commandallocator.h"
#include "influx_graphics/pipelinestate.h"
#include "influx_graphics/fence.h"
#include "influx_graphics/swapchain.h"
#include "influx_graphics/resource.h"

#include "core/platform/window.h"

namespace influx::graphics
{
	// device class that encapsulates a list of physical devices,
	// and an interface similar to that of logical devices.
	class device
	{
	public:
		static device* create(e_api_type type);

		void set_api_type(e_api_type type);

		// get info about physical devices:
		inline virtual vector<physical_device_info> get_gpu_infos() {}

		// get interface to graphics object creation:
		inline virtual command_queue* create_command_queue(const command_queue_desc& desc) {}

		inline virtual swapchain* create_swapchain(command_queue* queue, const platform::window_handle& window) {}

		inline virtual command_allocator* create_graphics_allocator() {}

		inline virtual command_list* create_graphics_command_list(command_allocator* allocator, pipeline_state* init_state = nullptr) {}

		inline virtual fence* create_fence() {}

		inline virtual resource* create_resource(const tex2D_desc& desc) {}

	private:
		vector<base*> mp_children = {};
		e_api_type m_type{};

	protected:
		device() = default;
	};
}