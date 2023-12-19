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

		class editor_frame final
		{
		public:
			editor_frame() = default;
			editor_frame(uint64 frame_id, void* frame_data)
				: m_frame_id{frame_id}, m_frame_data{frame_data}
			{

			}

			uint64 m_frame_id = 0u;
			void* m_frame_data = nullptr; // ImGuiDrawData
		};

		bool push_frame(const game_frame& frame)
		{
			return m_frames.push(frame);
		}

		bool pop_frame(game_frame& out_frame)
		{
			return m_frames.pop(out_frame);
		}

		bool push_frame(const editor_frame& frame)
		{
			if (m_editor_frames.is_full()) return false;
			return m_editor_frames.push(frame);
		}

		bool pop_frame(editor_frame& frame)
		{
			return m_editor_frames.pop(frame);
		}

	private:
		ringbuffer<game_frame, 2u> m_frames{};
		ringbuffer<editor_frame, 1u> m_editor_frames{};
	};
}


