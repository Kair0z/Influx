#pragma once
#include "influx_graphics/base.h"
#include "core/basetypes.h"

namespace influx::graphics
{
	enum class e_descriptor_heap_type : uint8
	{
		rtv,
		dsv,
		cbv,
		sampler,
		count
	};

	using descriptor_handle = void*;

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
			bool m_shader_visible;
		};

		virtual descriptor_handle allocate_cpu() = 0;
		virtual descriptor_handle allocate_gpu() = 0;

		virtual void free_cpu(descriptor_handle handle) = 0;
		virtual void free_gpu(descriptor_handle handle) = 0;
	
		inline uint32 get_capacity() const
		{
			return m_create_args.m_capacity;
		}

	protected:
		descriptor_heap(const create_args& args)
			: m_create_args{ args } {}

	protected:
		create_args m_create_args{};
	};
}