#pragma once
#include "influx_graphics/resource.h"
#include "vk_headers.h"

namespace influx::graphics
{
	class vk_resource final : public resource
	{
		virtual void* map(const map_args& args) override;
		virtual void unmap(const map_args& args) override;

	public:
		vk_resource() = default;
		vk_resource(const vk::Image& image);

	private:
		vk::Image m_vk_image;

	protected:
#if _DEBUG
		virtual void set_name_impl(const string& new_name) override;
#endif
	};
}