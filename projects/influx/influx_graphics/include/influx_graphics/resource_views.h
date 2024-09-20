#pragma once
#include "influx_graphics/base.h"
#include "influx_graphics/descriptorheap.h"

namespace influx::graphics
{
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