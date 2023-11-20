#include "app_pch.h"
#include "gamethread.h"
#include "application/application.h"

#include "influx_async.h"
#pragma comment(lib, "InfluxAsync.lib")

#include "Core/Math/Random.h"

namespace influx::application
{
	inline application& get_application()
	{
		return application::get_instance();
	}

	void gamethread::initialize()
	{
		m_entities.clear();
		m_entities.reserve(k_num_entities);
		for (uint64 i = 0u; i < k_num_entities; ++i)
		{
			m_entities.push_back(entity{ i });
		}

		m_camera_entity.m_transform.set_position({ 0.0f, 0.0f, 10.0f });
		m_camera_entity.m_transform.set_forward({ 0.0f, 0.0f, -1.0f });
	}

	void gamethread::tick()
	{
		auto entity_update = [](entity& entity)
		{
			entity.m_transform = math::transform3D(
				random::get_random_unit_vectorf3() * 5.0f,
				math::quaternion::identity(),
				math::vectorf3::one());
		};

		if (k_jobify)
		{
			vector<async::task_handle> update_job_handles =
				async::dispatch_for(m_entities.size(), 
					[this, entity_update](uint64 i) 
				{ 
					entity_update(m_entities[i]); 
				});

			async::wait_for(update_job_handles);
		}
		else
		{
			for (entity& e : m_entities)
			{
				entity_update(e);
			}
		}

		// push a game frame
		// if unsuccesful, that means we're full -> stall until render pops a frame...
		rendersync::game_frame result_frame = {};
		result_frame.m_frame_id = get_frame();
		result_frame.m_entities = m_entities;
		result_frame.m_camera_entity = m_camera_entity;

		mark_sync_start();
		while (!application::get_render_sync().push_frame(result_frame))
		{
			// ...
		}
		mark_sync_end();
	}

	void gamethread::cleanup()
	{

	}
}

