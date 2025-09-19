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
#include "influx_graphics/raytracing.h"

// influx::core
#include "core/pointer.h"

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
		/* create a device */
		INFLUX_GFX_API 
		static device* create(e_api_type type, const device_desc& desc = device_desc{});

		/* optional */
		INFLUX_GFX_API
		void set_log_function(log_function function);
		
		void set_api_type(e_api_type type);

		virtual void release(base*);

		virtual void cleanup() = 0;

		virtual feature_info get_feature_info() const = 0;
		virtual bool is_device_removed() = 0;

		virtual ptr<queue>				create_queue(const queue_desc& desc = queue_desc::default_graphics()) = 0;
		virtual ptr<swapchain>			create_swapchain(queue* queue, const platform::window& window, const swapchain_desc& desc = swapchain_desc::default_tripple()) = 0;
		virtual ptr<descriptor_heap>	create_descriptor_heap(const descriptor_heap::create_args&) = 0;

		virtual ptr<commandlist> create_graphics_commandlist(graphics_pipeline* init_state = nullptr) = 0;
		virtual ptr<commandlist> create_compute_commandlist(compute_pipeline* init_state = nullptr) = 0;
		virtual ptr<fence> create_fence(uint64 init_value = 0u) = 0;

		/* resources */
		virtual ptr<resource> create_resource(const struct tex3D_desc& desc, const heap_desc& heap_desc = {}) = 0;
		virtual ptr<resource> create_resource(const struct cubemap_desc& desc, const heap_desc& heap_desc = {}) = 0;
		virtual ptr<resource> create_resource(const struct tex2D_desc& desc, const heap_desc& heap_desc = {}) = 0;
		virtual ptr<resource> create_resource(const struct buffer_desc& desc, const heap_desc& heap_desc = {}) = 0;
		virtual ptr<resource> create_resource(const struct acc_str_desc& desc, const heap_desc& heap_desc = {}) = 0;
		virtual result<blas_resources> create_blas(const blas_create_args& args) = 0;
		virtual result<tlas_resources> create_tlas(const tlas_create_args& args) = 0;
		virtual ptr<resource> create_upload_resource(resource* resource) = 0;

		virtual ptr<resource> import_buffer(void* native_ptr, const buffer_desc& desc) = 0;
		virtual ptr<resource> import_texture(void* native_ptr, const tex2D_desc& desc) = 0;

		inline ptr<descriptor_heap> create(const descriptor_heap::create_args& args) { return create_descriptor_heap(args); }

		/* descriptors */
		virtual void create_rtv(descriptor_handle cpu_handle, resource* resource) = 0;
		virtual void create_dsv(descriptor_handle cpu_handle, resource* resource) = 0;
		virtual void create_buffer_srv(descriptor_handle cpu_handle, resource* resource) = 0;
		virtual void create_buffer_uav(descriptor_handle cpu_handle, resource* resource) = 0;
		virtual void create_texture_srv(descriptor_handle cpu_handle, resource* resource) = 0;
		virtual void create_texture_uav(descriptor_handle cpu_handle, resource* resource) = 0;
		virtual void create_sampler_view(descriptor_handle cpu_handle, resource* resource) = 0;
		virtual void create_accstruct_view(descriptor_handle cpu_handle, resource* resource) = 0;
		virtual void create_buffer_cbv(descriptor_handle cpu_handle, resource* resource) = 0;

		virtual void create_rtv(resource* resource, descriptor_handle cpu_handle) { return create_rtv(cpu_handle, resource); }
		virtual void create_dsv(resource* resource, descriptor_handle cpu_handle) { return create_dsv(cpu_handle, resource); }
		virtual void create_srv_texture(resource* resource, descriptor_handle cpu_handle) { return create_texture_srv(cpu_handle, resource); }
		virtual void create_uav_texture(resource* resource, descriptor_handle cpu_handle) { return create_texture_uav(cpu_handle, resource); }
		virtual void create_uav_buffer(resource* resource, descriptor_handle cpu_handle) { return create_buffer_uav(cpu_handle, resource); }
		virtual void create_srv_buffer(resource* resource, descriptor_handle cpu_handle) { return create_buffer_srv(cpu_handle, resource); }
		virtual void create_cbv_buffer(resource* resource, descriptor_handle cpu_handle) { return create_buffer_cbv(cpu_handle, resource); }

		/* pipeline state objects */
		virtual ptr<rootsignature> create_rootsignature(const rootsignature_desc& desc) = 0;
		virtual ptr<graphics_pipeline> create_graphics_pipeline(rootsignature* rootsig, const graphics_pipeline_desc& desc) = 0;
		virtual ptr<compute_pipeline> create_compute_pipeline(rootsignature* rootsig, const compute_pipeline_desc& desc) = 0;
		virtual ptr<raytracing_pipeline> create_raytracing_pipeline(rootsignature* rootsig, const raytracing_pipeline_desc& desc) = 0;
		virtual ptr<mesh_pipeline> create_mesh_pipeline(rootsignature* rootsig, const mesh_pipeline_desc& desc) = 0;
		virtual ptr<graph_pipeline> create_workgraph_pipeline(rootsignature* rootsig, const graph_pipeline_desc& desc) = 0;

		virtual void copy_descriptors(
			const descriptor_range& source, const descriptor_range& dest,
			const graphics::e_descriptor_heap_type& heap_type) = 0;

		virtual void* get_native() = 0;

		virtual vector<physical_device_info> get_gpu_infos() = 0;
		virtual result<memory_info> get_memory_info() const = 0;

		bool is_initialized() const;
		virtual ~device() = default;

	private:
		e_api_type m_type{};
		bool m_is_initialized = false;
		log_function m_log_function = {};

	protected:
		device(const device_desc& desc);
		device_desc m_desc{};

		queue* m_graphics_queue= nullptr;
		queue* m_compute_queue = nullptr;
		queue* m_copy_queue = nullptr;

		void set_initialized(bool initialized);
		void log(e_log, const char*) const;
	};
}