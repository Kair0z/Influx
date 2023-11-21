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
			uint64 m_frame_id = 0u;
			vector<entity> m_entities{};
			entity m_camera_entity{};
		};

		bool push_frame(const game_frame& frame)
		{
			return m_frames.push(frame);
		}

		bool pop_frame(game_frame& out_frame)
		{
			return m_frames.pop(out_frame);
		}

	private:
		ringbuffer<game_frame, 3u> m_frames{};
	};
}


