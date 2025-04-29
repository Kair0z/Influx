#pragma once

#include "influx_renderer/texture.h"

// heavily inspired by
// https://simonstechblog.blogspot.com/2019/06/d3d12-descriptor-heap-management.html

namespace influx::graphics
{
	class commandlist;
}

namespace influx::renderer
{
	class descriptor_manager final
	{
	public:
		descriptor_manager(graphics::device* device);
		virtual ~descriptor_manager();

		void start_frame();

		// binds all gpu heaps
		void start_commandlist(graphics::commandlist* commandlist);

		// unstages all gpu heaps
		void end_frame();

		graphics::descriptor_handle create_rtv(graphics::resource* resource);
		graphics::descriptor_handle create_dsv(graphics::resource* resource);
		graphics::descriptor_handle create_srv(graphics::resource* resource);
		graphics::descriptor_handle create_buffer_srv(graphics::resource* resource);
		graphics::descriptor_handle create_sampler();

		// stages descriptors into the appropriate shader-visible descriptor heap
		// and returns the address of that range of descriptors
		graphics::descriptor_range stage(const vector<graphics::descriptor_handle>& cpu_descriptors);
		graphics::descriptor_range stage(const graphics::descriptor_handle& cpu_descriptor);
		graphics::descriptor_range stage(const vector<texture2D*>& textures);
		graphics::descriptor_range stage(texture2D* texture);

		graphics::descriptor_range stage_sampler(graphics::descriptor_handle handle);
		graphics::descriptor_range stage_samplers(const vector<graphics::descriptor_handle>& samplers);

		void cleanup_rtv(graphics::descriptor_handle rtv);
		void cleanup_dsv(graphics::descriptor_handle dsv);
		void cleanup_srv(graphics::descriptor_handle srv);

	private:
		// GPU heaps (shader-visible)
		graphics::descriptor_heap* mp_srv_gpu_heap;
		graphics::descriptor_heap* mp_samp_gpu_heap;

		// CPU heaps (storage)
		graphics::descriptor_heap* mp_rtv_heap;
		graphics::descriptor_heap* mp_dsv_heap;
		graphics::descriptor_heap* mp_srv_heap;
		graphics::descriptor_heap* mp_sampler_heap;

		graphics::device* mp_device;
	};
}