#pragma once

#include "application/scene/scene.h"
#include "Core/Container/ringbuffer.h"

namespace influx::application
{
	// synchronization object for gamethread & renderthread
	class rendersync final
	{
	public:
		class game_frame final
		{
		public:
			vector<entity> m_entities{};
			entity m_camera_entity{};
		};

		bool push(const game_frame& frame);
		bool pop(game_frame& out_frame);

	private:
		ringbuffer<game_frame, 2u> m_game_frame_stack{};
	};
}


