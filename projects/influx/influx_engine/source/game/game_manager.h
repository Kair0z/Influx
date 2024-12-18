#pragma once

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

	private:
		state m_state;
	};
}