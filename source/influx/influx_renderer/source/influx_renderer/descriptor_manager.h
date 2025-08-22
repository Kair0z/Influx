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
		descriptor_manager(rhi_device* device);
		virtual ~descriptor_manager();

		void start_frame();

		// binds all gpu heaps
		void start_commandlist(rhi_commandlist* commandlist);

		// unstages all gpu heaps
		void end_frame();

		rhi_descriptor create_rtv(rhi_resource* resource);
		rhi_descriptor create_dsv(rhi_resource* resource);
		rhi_descriptor create_srv(rhi_resource* resource);
		rhi_descriptor create_buffer_srv(rhi_resource* resource);
		rhi_descriptor create_sampler();

		// stages descriptors into the appropriate shader-visible descriptor heap
		// and returns the address of that range of descriptors
		rhi_descriptor_range stage(const vector<rhi_descriptor>& cpu_descriptors);
		rhi_descriptor_range stage(const rhi_descriptor& cpu_descriptor);
		rhi_descriptor_range stage(const vector<texture2D*>& textures);
		rhi_descriptor_range stage(texture2D* texture);

		rhi_descriptor_range stage_sampler(rhi_descriptor handle);
		rhi_descriptor_range stage_samplers(const vector<rhi_descriptor>& samplers);

		void cleanup_rtv(graphics::descriptor_handle rtv);
		void cleanup_dsv(graphics::descriptor_handle dsv);
		void cleanup_srv(graphics::descriptor_handle srv);

	private:
		// GPU heaps (shader-visible)
		rhi_descheap* mp_srv_gpu_heap;
		rhi_descheap* mp_samp_gpu_heap;

		// CPU heaps (storage)
		rhi_descheap* mp_rtv_heap;
		rhi_descheap* mp_dsv_heap;
		rhi_descheap* mp_srv_heap;
		rhi_descheap* mp_sampler_heap;

		rhi_device* mp_device;
	};
}