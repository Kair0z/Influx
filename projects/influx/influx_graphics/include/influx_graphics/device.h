#pragma once
#include "influx_graphics/common.h"
#include "influx_graphics/base.h"
#include "influx_graphics/commandqueue.h"
#include "influx_graphics/commandlist.h"
#include "influx_graphics/commandallocator.h"
#include "influx_graphics/pipeline.h"
#include "influx_graphics/fence.h"
#include "influx_graphics/swapchain.h"
#include "influx_graphics/resource.h"
#include "influx_graphics/resource_views.h"
#include "influx_graphics/descriptorheap.h"
#include "influx_graphics/rootsignature.h"
#include "core/platform/window.h"

namespace influx::graphics
{
	struct tex2D_desc;

	// device class that encapsulates a list of physical devices,
	// and an interface similar to that of logical devices.
	class device
	{
	public:
		static device* create(e_api_type type);

		void set_api_type(e_api_type type);

		// interface:
		virtual vector<physical_device_info> get_gpu_infos() = 0;

		virtual command_queue* create_command_queue(const command_queue_desc& desc) = 0;

		virtual swapchain* create_swapchain(command_queue* queue, const platform::window_handle& window, const swapchain_desc& desc) = 0;

		virtual descriptor_heap* create_descriptor_heap(const descriptor_heap::create_args&) = 0;

		virtual command_allocator* create_graphics_allocator() = 0;

		virtual command_list* create_graphics_command_list(command_allocator* allocator, pipeline* init_state = nullptr) = 0;

		virtual fence* create_fence(uint64 init_value = 0u) = 0;

		virtual resource* create_resource(const tex2D_desc& desc, const heap_desc& heap_desc = {}) = 0;

		virtual resource* create_resource(const buffer_desc& desc, const heap_desc& heap_desc = {}) = 0;

		virtual render_target_view* create_rtv(descriptor_heap* rtv_heap, resource* resource) = 0;
		virtual render_target_view* create_rtv(descriptor_handle handle, resource* resource) = 0;

		virtual input_resource_view* create_srv(descriptor_heap* irv_heap, resource* resource) = 0;
		virtual input_resource_view* create_srv(descriptor_handle cpu_handle, descriptor_handle gpu_handle, resource* resource) = 0;

		virtual sampler_view* create_sampview(descriptor_heap* samp_heap, resource* resource) = 0;
		virtual sampler_view* create_sampview(descriptor_handle handle, resource* resource) = 0;

		virtual rootsignature* create_rootsignature(const rootsignature_desc& desc) = 0;

		virtual pipeline* create_pipeline(rootsignature* rootsig, const pipeline_desc& desc) = 0;

	private:
		vector<base*> mp_children = {};
		e_api_type m_type{};

	protected:
		device() = default;
	};
}