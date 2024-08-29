#pragma once
#include "influx_graphics/base.h"
#include "core/basetypes.h"

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
	};

	class descriptor_heap : public base
	{
	public:
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
}