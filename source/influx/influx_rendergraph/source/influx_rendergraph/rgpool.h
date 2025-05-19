#pragma once

// influx::core
#include "core/basetypes.h"
#include "core/container/vector.h"

// influx::rendergraph
#include "rgcommon.h"

// influx::graphics
#include "influx_graphics/descriptors.h"
namespace influx::graphics
{
	class device;
	class resource;
	class descriptor_heap;
}

namespace influx::rendergraph
{
	/*
		helper class that allocates resources & recycles them based on an expiry frame date
		also keeps descriptor heaps to help allocate views
	*/
	class rgpool final
	{
		struct pooled_buffer final
		{
			buffer_desc m_desc;
			graphics::resource* m_resource;
			uint64 m_last_used_frame;
			bool m_is_active;
		};
		struct pooled_texture final
		{
			texture_desc m_desc;
			graphics::resource* m_resource;
			uint64 m_last_used_frame;
			bool m_is_active;
		};

		uint64 m_frame = 0u;
		vector<pooled_texture> m_texture_pool;
		vector<pooled_buffer> m_buffer_pool;
		global_config m_config;

		graphics::descriptor_heap* m_srv_heap;
		graphics::descriptor_heap* m_sampler_heap;
		graphics::descriptor_heap* m_rtv_heap;
		graphics::descriptor_heap* m_dsv_heap;

		friend class rendergraph;
	private:
		rgpool(graphics::device& device, const global_config& config);
		~rgpool();

		void create_descriptor_heaps(graphics::device& device, const global_config&);
		void free_all_descriptors();
		void free_all_gpu_descriptors();
		void cleanup(graphics::device& device);

		// views
		result<graphics::descriptor_handle> alloc_cpu_handle(rgdescriptor_type type);
		result<graphics::descriptor_handle> alloc_gpu_srv();
		result<graphics::descriptor_handle> alloc_gpu_sampler();
		result<> free_cpu_handle(rgdescriptor_type type, graphics::descriptor_handle handle);

		/* (de)allocating new resources */
		result<graphics::resource*> allocate_texture_resource(graphics::device&, const texture_desc& args);
		result<graphics::resource*> allocate_buffer_resource(graphics::device&, const buffer_desc& args);
		result<> release_texture(graphics::device&, graphics::resource& resource);
		result<> release_buffer(graphics::device&, graphics::resource& resource);
	};
}