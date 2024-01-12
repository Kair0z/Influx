#pragma once
#include "influx_graphics/base.h"
#include "influx_graphics/common.h"

#include "core/math/vector.h"

namespace influx::graphics
{
	class resource;

	struct swapchain_desc final
	{
		e_format m_format = e_format::rgba8;
		math::vectoru2 m_dimensions{};
		uint8 m_num_buffers = 2u;
	};

	struct present_args final
	{
		bool m_vsync = false;
	};

	class swapchain : public base
	{
	public:
		virtual void present(const present_args& args) = 0;

		virtual resource* get_backbuffer_resource(uint8 at_index) const = 0;

		virtual uint8 get_current_backbuffer_index() const = 0;

		inline uint8 get_num_backbuffers() const
		{
			return m_desc.m_num_buffers;
		}

	protected:
		swapchain(const swapchain_desc& desc)
			: m_desc{desc}
		{

		}

		inline const swapchain_desc& get_desc() const
		{
			return m_desc;
		}

	private:
		swapchain_desc m_desc{};
	};
}