#pragma once
#include "influx_graphics/resource.h"
#include "vk_headers.h"

namespace influx::graphics
{
	class vk_resource final : public resource
	{
	public:
		vk_resource() = default;
		vk_resource(const vk::Image& image);

	private:
		vk::Image m_vk_image;
	};
}