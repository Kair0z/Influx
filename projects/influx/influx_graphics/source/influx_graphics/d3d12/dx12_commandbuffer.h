#pragma once
#include "influx_graphics/commandbuffer.h"

namespace influx::graphics
{
	class dx12_commandqueue;
	class dx12_fence;
}

namespace influx::graphics
{
	class dx12_commandbuffer final : public commandbuffer
	{
	public:
		dx12_commandbuffer(graphics::dx12_commandqueue* queue, graphics::dx12_fence* fence);

	private:
		virtual void submit() override;
		virtual void submit(commandqueue* queue) override;

		virtual e_state get_state() const override;

		void set_state(e_state new_state);

	private:
		graphics::dx12_commandqueue* m_queue;
		graphics::dx12_fence* m_fence;
		uint32 m_finished_value = 0u;
	};
}