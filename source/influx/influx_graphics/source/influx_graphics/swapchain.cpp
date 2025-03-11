#include "graphics_pch.h"

// influx::graphics
#include "influx_graphics/swapchain.h"
#include "influx_graphics/resource.h"

namespace influx::graphics
{
	swapchain::swapchain(const swapchain_desc& desc, const swapchain_dependencies& dependencies)
		: m_desc{ desc }
		, m_current_dimensions{ desc.m_dimensions }
		, mp_parent_device{ dependencies.mp_device }
		, mp_queue{ dependencies.mp_queue }
	{

	}

	swapchain::~swapchain()
	{
		
	}

	void swapchain::update_backbuffer_index(uint8 new_index)
	{
		m_current_backbuffer_index = new_index;
	}

	device* swapchain::get_parent_device()
	{
		return mp_parent_device;
	}

	vector<resource*>& swapchain::get_resources()
	{
		return mp_resources;
	}

	void swapchain::resize(device* device, const math::vectoru2& new_dimensions)
	{
		// destroy previous
		destroy_resources(device);

		// resize
		const math::vectoru2 old_dimensions = m_current_dimensions;
		m_current_dimensions = new_dimensions;
		resize_impl(old_dimensions, new_dimensions);

		// call impl creation function
		create_resources(device);
	}

	void swapchain::resize(device* device, const platform::window& window)
	{
		resize(device, window.get_dimensions(platform::window::e_space::client));
	}

	resource* swapchain::get_backbuffer_resource(uint8 at_index) const
	{
		influx_assert(at_index < mp_resources.size());
		return mp_resources[at_index];
	}

	resource* swapchain::get_current_backbuffer_resource() const
	{
		return get_backbuffer_resource(get_current_backbuffer_index());
	}

	uint8 swapchain::get_num_backbuffers() const
	{
		return m_desc.m_num_buffers;
	}

	const swapchain_desc& swapchain::get_desc() const
	{
		return m_desc;
	}

	bool swapchain::needs_recreate(const platform::window& window) const
	{
		// if swapchain dimensions != current window dimensions
		const auto& dimensions = window.get_dimensions(platform::window::e_space::client);
		return m_current_dimensions != dimensions;
	}

	uint8 swapchain::get_current_backbuffer_index() const
	{
		return m_current_backbuffer_index;
	}

	const math::vectoru2& swapchain::get_dimensions() const
	{
		return m_current_dimensions;
	}

	e_format swapchain::get_format() const
	{
		return m_desc.m_format;
	}
}

