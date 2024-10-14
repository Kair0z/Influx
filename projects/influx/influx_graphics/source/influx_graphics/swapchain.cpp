#include "graphics_pch.h"
#include "influx_graphics/swapchain.h"
#include "influx_graphics/resource.h"

#include "core/platform/win32/win32_window.h"

namespace influx::graphics
{
	swapchain::swapchain(const swapchain_desc& desc, const swapchain_dependencies& dependencies)
		: m_desc{ desc }
		, m_current_dimensions{ desc.m_dimensions }
		, mp_parent_device{ dependencies.mp_device }
		, mp_queue{ dependencies.mp_queue }
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

	void swapchain::resize(const math::vectoru2& new_dimensions)
	{
		// destroy resources & rtvs
		for (resource* res : mp_buffer_resources)
		{
			delete res;
			res = nullptr;
		}

		// update current dimensions
		m_current_dimensions = new_dimensions;

		// call impl creation function
		mp_buffer_resources = create_resources();
	}

	void swapchain::resize(const platform::window& window)
	{
		const auto& rect = window.get_rect_client();
		resize(rect.m_width_height);
	}

	resource* swapchain::get_backbuffer_resource(uint8 at_index) const
	{
		influx_assert(at_index < mp_buffer_resources.size());
		return mp_buffer_resources[at_index];
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
		const auto& rect = window.get_rect_client();
		return m_current_dimensions != rect.get_dimensions();
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

