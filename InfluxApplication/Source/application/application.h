#pragma once

#include "influx_application.h"

#include "core/singleton/singleton.h"
#include "core/platform/platform.h"
#include "Core/Container/Vector.h"
#include "core/Math/Matrix.h"
#include "Core/Math/Transform.h"
#include "Core/Container/RingBuffer.h"

#include <atomic>
#include <thread>

namespace influx::renderer
{
	struct material_data;
	struct scene_proxy;
}

namespace influx::application
{
	constexpr static bool k_render_scene = true;
	constexpr static bool k_jobify = false;
	constexpr static influx::uint8 k_max_thread_frame_difference = 1u;

	class application final 
		: public singleton<application>
	{
		struct entity final
		{
			entity() = default;
			entity(uint64 id) : m_id{ id } {}

			uint64 m_id = 0u;
			math::transform3D m_transform = math::transform3D::identity();
		};
		
	public:
		void run(const run_args& args);
		void request_quit();

		string get_resource_directory() const;
		run_args get_run_arguments() const;

	private:
		void run_mainthread();
		void run_renderthread();
		void run_gamethread();

		void mainthread_log();

		platform::window_handle m_windowhandle = nullptr;
		platform::instance_handle m_instancehandle = nullptr;
		std::atomic_bool m_is_quit_requested = false;

		vector<class dedicated_thread*> m_dedicated_threads{};

		vector<entity> m_entities{};
		entity m_camera_entity{};

		run_args m_run_args{};
	};
}


