#pragma once
#include "influx_graphics/base.h"
#include "influx_graphics/descriptorheap.h"

namespace influx::graphics
{
	class resource_view : public base
	{
	public:
		inline resource_view(descriptor_handle handle)
			: m_handle{ handle } {}

		descriptor_handle get_descriptor_handle() const;

	private:
		descriptor_handle m_handle;
	};

	class render_target_view : public resource_view
	{
	public:
		render_target_view(descriptor_handle handle)
			: resource_view(handle) {}
	};

	class vertex_buffer_view : public resource_view
	{
	public:
		vertex_buffer_view(descriptor_handle handle)
			: resource_view(handle) {}
	};

	class index_buffer_view : public resource_view
	{
	public:
		index_buffer_view(descriptor_handle handle)
			: resource_view(handle) {}
	};

	class sampler_view : public resource_view
	{
	public:
		sampler_view(descriptor_handle handle)
			: resource_view(handle) {}
	};

	class input_resource_view : public resource_view
	{
	public:
		input_resource_view(descriptor_handle handle)
			: resource_view(handle) {}
	};
}