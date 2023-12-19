#pragma once
#include "threads.h"

namespace influx::renderer
{
	struct scene_proxy;
	struct imgui_proxy;
}

namespace influx::application
{
	class renderthread final : public dedicated_thread
	{
	public:
		virtual e_dedicated_thread get_thread_type() const override
		{
			return e_dedicated_thread::renderthread;
		}

	private:
		virtual void initialize() override;
		virtual void tick() override;
		virtual void cleanup() override;

		renderer::scene_proxy* build_scene_proxy(const rendersync::game_frame& game_frame);
		void sync_to_gamethread(rendersync::game_frame& game_frame);

		renderer::scene_proxy* mp_scene_proxy{};
		renderer::imgui_proxy* mp_imgui_proxy{};
	};
}