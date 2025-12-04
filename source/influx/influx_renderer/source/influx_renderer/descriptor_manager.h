#pragma once

#include "influx_renderer/texture.h"

#include "influx_renderer/rhi.h"

// heavily inspired by
// https://simonstechblog.blogspot.com/2019/06/d3d12-descriptor-heap-management.html

namespace influx::renderer
{
	class descriptor_manager final
	{
	public:
		descriptor_manager(rhi_device& device);
		virtual ~descriptor_manager();

		void reset_gpu_heaps();
		void bind_gpu_heaps(rhi_commandlist& commandlist);

		rhi_descriptor create_rtv(rhi_device& device, rhi_resource& resource);
		rhi_descriptor create_dsv(rhi_device& device, rhi_resource& resource);
		rhi_descriptor create_srv(rhi_device& device, rhi_resource& resource);
		rhi_descriptor create_buffer_srv(rhi_device& device, rhi_resource& resource);
		rhi_descriptor create_sampler(rhi_device& device);

		rhi_descriptor_range stage(rhi_device& device, const vector<rhi_descriptor>& cpu_descriptors);
		rhi_descriptor_range stage(rhi_device& device, const rhi_descriptor& cpu_descriptor);
		rhi_descriptor_range stage(rhi_device& device, const vector<texture2D*>& textures);
		rhi_descriptor_range stage(rhi_device& device, texture2D* texture);
		rhi_descriptor_range stage_sampler(rhi_device& device, rhi_descriptor handle);
		rhi_descriptor_range stage_samplers(rhi_device& device, const vector<rhi_descriptor>& samplers);

		void cleanup_rtv(rhi_descriptor rtv);
		void cleanup_dsv(rhi_descriptor dsv);
		void cleanup_srv(rhi_descriptor srv);

	private:
		// GPU heaps (shader-visible)
		rhi_descheap* mp_srv_gpu_heap	[k_max_in_flight]{};
		rhi_descheap* mp_samp_gpu_heap	[k_max_in_flight]{};

		// CPU heaps (storage)
		rhi_descheap* mp_rtv_heap;
		rhi_descheap* mp_dsv_heap;
		rhi_descheap* mp_srv_heap;
		rhi_descheap* mp_sampler_heap;
	};
}