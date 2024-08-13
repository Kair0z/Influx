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
		struct descriptor_couple final
		{
			graphics::descriptor_handle* m_cpu_handle;
			graphics::descriptor_handle* m_gpu_handle;
		};

	public:
		descriptor_manager(graphics::device* device);
		virtual ~descriptor_manager();

		graphics::descriptor_heap* get_rtv_heap() const;
		graphics::descriptor_heap* get_samp_heap() const;
		graphics::descriptor_heap* get_srv_heap() const;
		graphics::descriptor_heap* get_dsv_heap() const;

		vector<descriptor_couple> allocate_srv(uint64 num_descriptors);
		void free_srv(uint64 num_descriptors);

	private:
		graphics::descriptor_heap* mp_rtv_heap;
		graphics::descriptor_heap* mp_dsv_heap;
		graphics::descriptor_heap* mp_sampler_heap;
		graphics::descriptor_heap* mp_cbv_heap;

		ringbuffer<descriptor_couple, 512u> m_srv_allocation_buffer;
	};
}