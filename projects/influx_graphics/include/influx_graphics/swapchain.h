#pragma once
#include "influx_graphics/base.h"

namespace influx::graphics
{
	struct swapchain_desc final
	{

	};

	struct present_args final
	{
		bool m_vsync = false;
	};

	class swapchain : public base
	{
	public:
		virtual void present(const present_args& args) = 0;

	protected:
		swapchain(const swapchain_desc& desc)
		{

		}
	};
}