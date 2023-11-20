#include "app_pch.h"
#include "rendersync.h"

namespace influx::application
{
	bool rendersync::push(const game_frame& frame)
	{
		return m_game_frame_stack.push(frame);
	}

	bool rendersync::pop(game_frame& out_frame)
	{
		return m_game_frame_stack.pop(out_frame);
	}
}

