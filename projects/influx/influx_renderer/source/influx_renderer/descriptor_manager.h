#pragma once

#include "influx_graphics/descriptorheap.h"
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
		struct srv_heap final
		{
			graphics::descriptor_heap* mp_cpu_heap;
			graphics::descriptor_heap* mp_online_heap;
		};

		struct sampler_heap final
		{
			graphics::descriptor_heap* mp_cpu_heap;
			graphics::descriptor_heap* mp_online_heap;
		};

	public:
		descriptor_manager(graphics::device* device);
		virtual ~descriptor_manager();

		graphics::descriptor_heap* get_rtv_heap() const;
		graphics::descriptor_heap* get_samp_heap() const;
		graphics::descriptor_heap* get_srv_heap() const;
		graphics::descriptor_heap* get_dsv_heap() const;
		graphics::descriptor_heap* get_heap(graphics::e_descriptor_heap_type type) const;

		graphics::descriptor_handle allocate_cpu(graphics::e_descriptor_heap_type type);
		graphics::descriptor_handle allocate_gpu(graphics::e_descriptor_heap_type type);
		void free_cpu(graphics::descriptor_handle handle, graphics::e_descriptor_heap_type type);
		void free_gpu(graphics::descriptor_handle handle, graphics::e_descriptor_heap_type type);

	private:
		graphics::descriptor_heap* mp_rtv_heap;
		graphics::descriptor_heap* mp_dsv_heap;
		srv_heap m_srv_heap;
		sampler_heap m_samp_heap;
	};
}