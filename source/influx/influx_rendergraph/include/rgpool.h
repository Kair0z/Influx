#pragma once

// influx::core
#include "core/basetypes.h"
#include "core/container/vector.h"

// influx::graphics
namespace influx::graphics
{
	class device;
	class resource;
}

namespace influx::rendergraph
{
	struct texture_desc;
	struct buffer_desc;

	// allocates descriptors
	class view_manager final
	{
	public:
		view_manager(graphics::device* device)
		{
			graphics::descriptor_heap::create_args args{};
			// shader heaps:
			args.m_shader_visible = false;

			args.m_capacity = 8u;
			args.m_type = graphics::e_descriptor_heap_type::sampler;
			m_sampler_heap = device->create_descriptor_heap(args);

			args.m_capacity = 64u;
			args.m_type = graphics::e_descriptor_heap_type::srv;
			m_resource_heap = device->create_descriptor_heap(args);

			// non-shader heaps:
			args.m_shader_visible = false;

			args.m_capacity = 32;
			args.m_type = graphics::e_descriptor_heap_type::rtv;
			m_rtv_heap = device->create_descriptor_heap(args);
			
			args.m_capacity = 32;
			args.m_type = graphics::e_descriptor_heap_type::dsv;
			m_dsv_heap = device->create_descriptor_heap(args);
		}

		~view_manager()
		{
			end_frame();
		}

		result<graphics::descriptor_handle> alloc_cpu_handle(rgdescriptor_type type)
		{
			switch (type)
			{
			case rgdescriptor_type::render_target: return m_rtv_heap->allocate_cpu();
			case rgdescriptor_type::depth_target: return m_dsv_heap->allocate_cpu();
			case rgdescriptor_type::read_only: return m_resource_heap->allocate_cpu();
			case rgdescriptor_type::read_write: return m_resource_heap->allocate_cpu();
			}

			return {};
		}

		result<graphics::descriptor_handle> alloc_gpu_resource()
		{
			return m_resource_heap->allocate_gpu();
		}

		result<graphics::descriptor_handle> alloc_gpu_sampler()
		{
			return m_sampler_heap->allocate_gpu();
		}

		void end_frame()
		{
			m_resource_heap->free_all_gpu();
			m_resource_heap->free_all_cpu();
			m_sampler_heap->free_all_cpu();
			m_sampler_heap->free_all_gpu();
			m_rtv_heap->free_all_cpu();
			m_rtv_heap->free_all_gpu();
			m_dsv_heap->free_all_gpu();
			m_dsv_heap->free_all_cpu();
		}

	private:
		graphics::descriptor_heap* m_resource_heap;
		graphics::descriptor_heap* m_sampler_heap;
		graphics::descriptor_heap* m_rtv_heap;
		graphics::descriptor_heap* m_dsv_heap;
	};

	class rgpool final
	{
		friend class rendergraph;
	private:
		rgpool(graphics::device* device)
		{
			graphics::descriptor_heap::create_args args{};
			// shader heaps:
			args.m_shader_visible = false;

			args.m_capacity = 8u;
			args.m_type = graphics::e_descriptor_heap_type::sampler;
			m_sampler_heap = device->create_descriptor_heap(args);

			args.m_capacity = 64u;
			args.m_type = graphics::e_descriptor_heap_type::srv;
			m_resource_heap = device->create_descriptor_heap(args);

			// non-shader heaps:
			args.m_shader_visible = false;

			args.m_capacity = 32;
			args.m_type = graphics::e_descriptor_heap_type::rtv;
			m_rtv_heap = device->create_descriptor_heap(args);
			
			args.m_capacity = 32;
			args.m_type = graphics::e_descriptor_heap_type::dsv;
			m_dsv_heap = device->create_descriptor_heap(args);
		}
		
		~rgpool()
		{
			end_frame();
		}

		struct pooled_resource final
		{
			graphics::resource* m_resource;
			uint64 m_last_used_frame;
			bool m_is_active;
		};

		void tick();
		void end_frame()
		{
			m_resource_heap->free_all_gpu();
			m_resource_heap->free_all_cpu();
			m_sampler_heap->free_all_cpu();
			m_sampler_heap->free_all_gpu();
			m_rtv_heap->free_all_cpu();
			m_rtv_heap->free_all_gpu();
			m_dsv_heap->free_all_gpu();
			m_dsv_heap->free_all_cpu();
		}

		result<graphics::descriptor_handle> alloc_cpu_handle(rgdescriptor_type type)
		{
			switch (type)
			{
			case rgdescriptor_type::render_target: return m_rtv_heap->allocate_cpu();
			case rgdescriptor_type::depth_target: return m_dsv_heap->allocate_cpu();
			case rgdescriptor_type::read_only: return m_resource_heap->allocate_cpu();
			case rgdescriptor_type::read_write: return m_resource_heap->allocate_cpu();
			}

			return {};
		}

		result<graphics::descriptor_handle> alloc_gpu_resource()
		{
			return m_resource_heap->allocate_gpu();
		}

		result<graphics::descriptor_handle> alloc_gpu_sampler()
		{
			return m_sampler_heap->allocate_gpu();
		}

		/* (de)allocating new resources */
		graphics::resource* allocate_texture_resource(const texture_desc& args);
		graphics::resource* allocate_buffer_resource(const buffer_desc& args);
		bool release_texture(graphics::resource* resource);
		bool release_buffer(graphics::resource* resource);

		graphics::device* m_device;
		uint64 m_frame = 0u;
		vector<pooled_resource> m_texture_pool;
		vector<pooled_resource> m_buffer_pool;

		graphics::descriptor_heap* m_resource_heap;
		graphics::descriptor_heap* m_sampler_heap;
		graphics::descriptor_heap* m_rtv_heap;
		graphics::descriptor_heap* m_dsv_heap;
	};
}