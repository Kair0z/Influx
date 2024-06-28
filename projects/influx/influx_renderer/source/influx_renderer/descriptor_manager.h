#pragma once

namespace influx::graphics
{
	class descriptor_heap;
	class device;
	class render_target_view;
}

namespace influx::renderer
{
	class descriptor_manager final
	{
	public:
		descriptor_manager(graphics::device* device);
		virtual ~descriptor_manager();

		graphics::descriptor_heap* get_rtv_heap() const;

	private:
		graphics::descriptor_heap* mp_rtv_heap;
		graphics::descriptor_heap* mp_dsv_heap;
		graphics::descriptor_heap* mp_sampler_heap;
		graphics::descriptor_heap* mp_cbv_heap;
	};
}