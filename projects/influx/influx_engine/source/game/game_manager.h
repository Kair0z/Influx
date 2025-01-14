#pragma once

// influx::core
#include "core/container/vector.h"

// influx::engine
#include "world/entity.h"

namespace influx::engine
{
	class game_manager final
	{
		enum class state
		{
			idle,
			running
		};

	public:
		void start();
		void tick();
		void end();

		entity create_entity();

	private:
		state m_state;
		vector<entity> m_entities{};

		void setup_camera();
		void setup_swords();
	};
}