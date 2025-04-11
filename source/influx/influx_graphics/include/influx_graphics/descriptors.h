#pragma once
#include "influx_graphics/base.h"

// influx::core
#include "core/basetypes.h"
#include "core/math/vector.h"

namespace influx::graphics
{
	enum class e_descriptor_heap_type : uint8
	{
		rtv,
		dsv,
		srv,
		sampler,
		count
	};

	using descriptor_handle = void*;

	// continuous range of descriptors
	struct descriptor_range final
	{
		descriptor_range() = default;
		descriptor_range(const descriptor_handle& handle)
			: m_start{handle}
			, m_num_descriptors{ 1u }{}

		descriptor_handle m_start;
		uint32 m_num_descriptors;
		uint32 m_start_idx;
	};

	class descriptor_heap : public base
	{
	public:
		virtual ~descriptor_heap() = default;

		struct create_args final
		{
			create_args() = default;
			inline create_args(e_descriptor_heap_type type, uint32 capacity, bool is_shader_visible)
				: m_type{type}
				, m_capacity{capacity}
				, m_shader_visible{is_shader_visible}
			{

			}

			e_descriptor_heap_type m_type{};
			uint32 m_capacity{};
			bool m_shader_visible = false;
		};
		inline static create_args create_rtv_heap(uint32 capacity)
		{
			create_args args{};
			args.m_capacity = capacity;
			args.m_type = e_descriptor_heap_type::rtv;
			args.m_shader_visible = false;
			return args;
		}

		virtual descriptor_handle allocate_cpu() = 0;
		virtual descriptor_handle allocate_gpu() = 0;
		
		inline descriptor_range allocate_range_cpu(uint32 num_descriptors)
		{
			descriptor_range range{};
			range.m_start = allocate_cpu();
			range.m_num_descriptors = num_descriptors;
			for (uint32 i = 0u; i < num_descriptors - 1u; ++i)
			{
				allocate_cpu();
			}
			return range;
		}
		inline descriptor_range allocate_range_gpu(uint32 num_descriptors)
		{
			descriptor_range range{};
			range.m_start = allocate_gpu();
			range.m_num_descriptors = num_descriptors;
			for (uint32 i = 0u; i < num_descriptors - 1u; ++i)
			{
				allocate_gpu();
			}
			return range;
		}

		virtual void free_cpu(descriptor_handle handle) = 0;
		virtual void free_gpu(descriptor_handle handle) = 0;

		virtual void free_cpu(uint32 at_index) = 0;
		virtual void free_gpu(uint32 at_index) = 0;

		virtual uint32 get_heap_index_cpu(descriptor_handle handle) const = 0;
		virtual uint32 get_heap_index_gpu(descriptor_handle handle) const = 0;

		inline uint32 get_capacity() const
		{
			return m_create_args.m_capacity;
		}

		virtual void free_all_cpu() = 0;
		virtual void free_all_gpu() = 0;

	protected:
		descriptor_heap(const create_args& args)
			: m_create_args{ args } {}

	protected:
		create_args m_create_args{};
	};

	struct resource_info final
	{
		math::vectorf2 m_dimensions;
	};

	class resource_view : public base
	{
	public:
		inline resource_view(descriptor_handle cpu_handle, descriptor_handle gpu_handle, const resource_info& res_info = {})
			: m_cpu_handle{ cpu_handle }
			, m_gpu_handle{ gpu_handle }
			, m_res_info{ res_info } {}

		INFLUX_GFX_API descriptor_handle get_cpu_handle() const;
		INFLUX_GFX_API descriptor_handle get_gpu_handle() const;

		inline const math::vectorf2& get_dimensions() const
		{
			return m_res_info.m_dimensions;
		}

	private:
		descriptor_handle m_cpu_handle;
		descriptor_handle m_gpu_handle;
		resource_info m_res_info;
	};

	class render_target_view : public resource_view
	{
	public:
		render_target_view(descriptor_handle cpu_handle, descriptor_handle gpu_handle, const resource_info& res_info = {})
			: resource_view(cpu_handle, gpu_handle, res_info) {}
	};

	class depth_stencil_view : public resource_view
	{
	public:
		depth_stencil_view(descriptor_handle cpu_handle, descriptor_handle gpu_handle)
			: resource_view(cpu_handle, gpu_handle) {}
	};

	class vertex_buffer_view : public resource_view
	{
	public:
		vertex_buffer_view(descriptor_handle cpu_handle, descriptor_handle gpu_handle)
			: resource_view(cpu_handle, gpu_handle) {}
	};

	class index_buffer_view : public resource_view
	{
	public:
		index_buffer_view(descriptor_handle cpu_handle, descriptor_handle gpu_handle)
			: resource_view(cpu_handle, gpu_handle) {}
	};

	class sampler_view : public resource_view
	{
	public:
		sampler_view(descriptor_handle cpu_handle, descriptor_handle gpu_handle)
			: resource_view(cpu_handle, gpu_handle) {}
	};

	class shader_resource_view : public resource_view
	{
	public:
		shader_resource_view(descriptor_handle cpu_handle, descriptor_handle gpu_handle)
			: resource_view(cpu_handle, gpu_handle) {}
	};
}