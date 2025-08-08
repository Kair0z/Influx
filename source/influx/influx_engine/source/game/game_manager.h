#pragma once

// influx::core
#include "core/container/vector.h"

// influx::engine
#include "world/entity.h"

// influx::platform
namespace influx::platform
{
	class library;
}

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
		game_manager();

		void start();
		void tick();
		void end();

		entity create_entity();
		~game_manager();
		
	private:
		state m_state = state::idle;
		vector<entity> m_entities = {};
		platform::library* m_game_library = nullptr;

		void setup_camera();
		void setup_swords();
		void setup_cafe();
		void setup_unitcube();
	};
}