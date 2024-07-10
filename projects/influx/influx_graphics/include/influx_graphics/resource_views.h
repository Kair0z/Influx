#pragma once
#include "influx_graphics/base.h"
#include "influx_graphics/descriptorheap.h"

namespace influx::graphics
{
	class resource_view : public base
	{
	public:
		inline resource_view(descriptor_handle cpu_handle, descriptor_handle gpu_handle)
			: m_cpu_handle{ cpu_handle }
			, m_gpu_handle{ gpu_handle } {}

		descriptor_handle get_cpu_handle() const;
		descriptor_handle get_gpu_handle() const;

	private:
		descriptor_handle m_cpu_handle;
		descriptor_handle m_gpu_handle;
	};

	class render_target_view : public resource_view
	{
	public:
		render_target_view(descriptor_handle cpu_handle, descriptor_handle gpu_handle)
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

	class input_resource_view : public resource_view
	{
	public:
		input_resource_view(descriptor_handle cpu_handle, descriptor_handle gpu_handle)
			: resource_view(cpu_handle, gpu_handle) {}
	};
}