#pragma once
#include "core/container/ringBuffer.h"

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
		graphics::descriptor_heap* get_samp_heap() const;
		graphics::descriptor_heap* get_srv_heap() const;
		graphics::descriptor_heap* get_dsv_heap() const;

	private:
		graphics::descriptor_heap* mp_rtv_heap;
		graphics::descriptor_heap* mp_dsv_heap;
		graphics::descriptor_heap* mp_sampler_heap;
		graphics::descriptor_heap* mp_cbv_heap;

		using descriptor_couple = std::pair<graphics::descriptor_handle*, graphics::descriptor_handle*>;
		ringbuffer<descriptor_couple> m_srv_allocation_buffer;
	};
}