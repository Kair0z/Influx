#pragma once

// influx::graphics
#include "influx_graphics/common.h"
#include "influx_graphics/base.h"
#include "influx_graphics/queue.h"
#include "influx_graphics/commandlist.h"
#include "influx_graphics/pipeline.h"
#include "influx_graphics/fence.h"
#include "influx_graphics/swapchain.h"
#include "influx_graphics/resource.h"
#include "influx_graphics/descriptors.h"
#include "influx_graphics/rootsignature.h"

// influx::core
#include "core/platform/window.h"

namespace influx::graphics
{
	struct device_desc final
	{
	public:
		device_desc() = default;

		string m_app_name		= "";
		string m_engine_name	= "influx engine";

		bool m_has_graphics_queue;
		bool m_has_compute_queue;
		bool m_has_copy_queue;
	};

	// device class that encapsulates a list of physical devices,
	// and an interface similar to that of logical devices.
	class device
	{
	public:
		INFLUX_GFX_API static device* create(e_api_type type, const device_desc& desc = device_desc{});
	
		void set_api_type(e_api_type type);

		virtual void cleanup() = 0;

		virtual vector<physical_device_info> get_gpu_infos() = 0;

		virtual memory_info get_memory_info() const = 0;

		virtual queue* create_queue(const queue_desc& desc = queue_desc::default_graphics()) = 0;

		virtual swapchain* create_swapchain(queue* queue, const platform::window& window, const swapchain_desc& desc) = 0;

		virtual descriptor_heap* create_descriptor_heap(const descriptor_heap::create_args&) = 0;

		virtual commandlist* create_commandlist(e_commandlist_type type, pipeline* init_state = nullptr) = 0;
		virtual commandlist* create_graphics_commandlist(pipeline* init_state = nullptr) = 0;
		virtual commandlist* create_compute_commandlist(pipeline* init_state = nullptr) = 0;

		virtual fence* create_fence(uint64 init_value = 0u) = 0;

		virtual resource* create_resource(const struct tex2D_desc& desc, const heap_desc& heap_desc = {}) = 0;
		virtual resource* create_resource(const struct buffer_desc& desc, const heap_desc& heap_desc = {}) = 0;

		// adds external resources to the RHI
		virtual resource* create_external_resource(void* native_ptr) = 0;

		virtual render_target_view* create_rtv(descriptor_heap* rtv_heap, resource* resource) = 0;
		virtual render_target_view* create_rtv(descriptor_handle handle, resource* resource) = 0;

		virtual depth_stencil_view* create_dsv(descriptor_heap* dsv_heap, resource* resource) = 0;
		virtual depth_stencil_view* create_dsv(descriptor_handle handle, resource* resource) = 0;

		virtual shader_resource_view* create_srv(descriptor_heap* irv_heap, resource* resource) = 0;
		virtual shader_resource_view* create_srv(descriptor_handle cpu_handle, descriptor_handle gpu_handle, resource* resource) = 0;
		virtual shader_resource_view* create_buffer_srv(descriptor_heap* srv_heap, resource* resource) = 0;
		virtual shader_resource_view* create_buffer_srv(descriptor_handle cpu_handle, descriptor_handle gpu_handle, resource* resource) = 0;

		virtual sampler_view* create_sampview(descriptor_heap* samp_heap, resource* resource) = 0;
		virtual sampler_view* create_sampview(descriptor_handle handle, resource* resource) = 0;

		virtual rootsignature* create_rootsignature(const rootsignature_desc& desc) = 0;
		virtual pipeline* create_pipeline(rootsignature* rootsig, const pipeline_desc& desc) = 0;

		virtual void copy_descriptors(const descriptor_range& source, const descriptor_range& dest,
			const graphics::e_descriptor_heap_type& heap_type) = 0;

		virtual void* get_native() = 0;

		bool is_initialized() const;

		virtual ~device() = default;

	private:
		vector<base*> mp_children = {};
		e_api_type m_type{};
		bool m_is_initialized = false;

	protected:
		device(const device_desc& desc)
			: m_desc{ desc } 
			, m_is_initialized{ true } {}

		device_desc m_desc{};

		void set_initialized(bool initialized);

		graphics::queue* m_compute_queue;
		graphics::queue* m_graphics_queue;
		graphics::queue* m_copy_queue;
	};
}