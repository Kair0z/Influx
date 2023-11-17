#include "app_pch.h"
#include "gamethread.h"
#include "application/application.h"

#include "influx_async.h"
#pragma comment(lib, "InfluxAsync.lib")

#pragma region imgui
#include "foreign/ImGui/imgui.h"
#if INFLUX_APP_USES_WINDOWS
#include "foreign/ImGui/imgui_impl_win32.h"
#endif
#pragma endregion

namespace influx::application
{
	inline application& get_application()
	{
		return application::get_instance();
	}

	void gamethread::static_initialize()
	{
		// start
		// temp: create entities
		constexpr uint64 k_num_entities = 4096u;
		m_entities.reserve(k_num_entities);
		for (uint64 i = 0u; i < k_num_entities; ++i)
		{
			m_entities.push_back(i);
		}
	}

	void gamethread::static_tick()
	{
		if (k_jobify)
		{
			vector<async::task_handle> update_job_handles =
				async::dispatch_for(m_entities.size(), [this](uint64 i)
					{
						m_entities[i].m_transform = math::transform3D(
							random::get_random_unit_vectorf3() * 5.0f,
							math::quaternion::identity(),
							math::vectorf3::one());
					});

			update_job_handles.push_back(async::dispatch([this]()
				{
					m_camera_entity.m_transform.set_position({ 0.0f, 0.0f, 10.0f });
					m_camera_entity.m_transform.set_forward({ 0.0f, 0.0f, -1.0f });
				}));

			async::wait_for(update_job_handles);
		}
		else
		{
			for (entity& e : m_entities)
			{
				e.m_transform = math::transform3D(
					random::get_random_unit_vectorf3() * 5.0f,
					math::quaternion::identity(),
					math::vectorf3::one());
			}
		}
	}

	void gamethread::static_cleanup()
	{

	}
}

