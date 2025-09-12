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
		rsc,
		sampler,
		count
	};
	static constexpr uint32 k_num_descriptor_heap_types = static_cast<uint32>(e_descriptor_heap_type::count);

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
		uint32 m_heap_index = 0u;
	};

	using descriptor_id = uint32;

	class descriptor_heap : public base
	{
	public:
		virtual ~descriptor_heap() = default;

		/* creation utillities */
		struct create_args final
		{
			create_args() = default;
			inline create_args(e_descriptor_heap_type type, uint32 capacity, bool is_shader_visible)
				: m_type{type}
				, m_capacity{capacity}
				, m_shader_visible{is_shader_visible}
			{

			}

			inline void set_capacity(uint32 capacity)
			{
				m_capacity = capacity;
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
		inline static create_args create_dsv_heap(uint32 capacity)
		{
			create_args args{};
			args.m_capacity = capacity;
			args.m_type = e_descriptor_heap_type::dsv;
			args.m_shader_visible = false;
			return args;
		}
		inline static create_args create_uav_heap(uint32 capacity)
		{
			create_args args{};
			args.m_capacity = capacity;
			args.m_type = e_descriptor_heap_type::rsc;
			args.m_shader_visible = true;
			return args;
		}

		/* allocate descriptors */
		virtual result<descriptor_id> allocate() = 0;
		inline result<descriptor_id> allocate_range(uint32 num_descriptors)
		{
			using result_type = result<descriptor_id>;

			auto res = allocate();
			if (res.is_unex()) 
				return result_type::make_error("error: failed allocating first descriptor!");

			descriptor_id first = res.get();
			for (uint32 i = 0u; i < num_descriptors - 1u; ++i)
			{
				res = allocate();
				if (res.is_unex()) 
					return result_type::make_error("error: failed allocating nth descriptor!");
			}

			return first;
		}
		virtual result<> free(descriptor_id handle) = 0;
		virtual result<> free_all() = 0;

		/* get the handles */
		virtual result<descriptor_handle> get_cpu(descriptor_id handle) const = 0;
		virtual result<descriptor_handle> get_gpu(descriptor_id handle) const = 0;
		virtual result<descriptor_id> get_id(descriptor_handle handle) const = 0;

		inline result<descriptor_handle> allocate_cpu()
		{
			using result_type = result<descriptor_handle>;
			auto alloc = allocate();
			if (!alloc.is_success())
				return result_type::make_error("failed allocating a descriptor!");

			return get_cpu(alloc.get());
		}
		inline result<descriptor_handle> allocate_gpu()
		{
			using result_type = result<descriptor_handle>;
			auto alloc = allocate();
			if (!alloc.is_success())
				return result_type::make_error("failed allocating a descriptor!");

			return get_gpu(alloc.get());
		}
		inline result<> free(descriptor_handle handle)
		{
			auto handle_to_id = get_id(handle);
			if (handle_to_id.is_success()) 
				return free(handle_to_id.get());
			else
				return result<>::make_error("handle does not belong to this GPU heap!");
		}

		inline uint32 get_capacity() const
		{
			return m_create_args.m_capacity;
		}

		inline e_descriptor_heap_type get_type() const
		{
			return m_create_args.m_type;
		}

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