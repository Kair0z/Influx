#pragma once
#include "influx_graphics/resource_views.h"
#include "vk_headers.h"

namespace influx::graphics
{
	class vk_render_target_view : public render_target_view
	{
	public:
		vk_render_target_view(const vk::ImageView& vk_view);

	private:
		vk::ImageView m_vk_view;
	};
}