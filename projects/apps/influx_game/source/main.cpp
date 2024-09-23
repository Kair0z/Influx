
#include <cstdint>

extern "C" { __declspec(dllexport) extern const uint32_t D3D12SDKVersion = 614u; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ""; }

#include "influx_application.h"

class actor
{
public:
	bool should_update = false;
	bool should_hide = false;
};

class scene
{
public:
	// get all actors
	actor const* get_actors();

	// find an actor by name
	actor const* get_actor(const string& name);

	uint32 get_num_actors();
};

class world
{
public:
	scene const* get_scenes();
	actor const* get_actors();
};

class game
{
public:
	// entrypoint when the engine initializes
	void initialize()
	{
		bool uses_input = false;
		bool uses_3D = false;
	}

	void start()
	{
		// spawn all actors
		// ...
	}

	void update(void* world, void* scene)
	{
		// world->get_actors();
		// scene->get_actors();
		// world->get_scenes();
	}

	void end()
	{

	}

	// exit when the engine is shutting down
	void cleanup()
	{

	}
};

int main(int argc, char** argv)
{
	using namespace influx;

	application::run_args arguments{};
	arguments.m_commandlet = false;
	arguments.m_single_threaded = false;
	arguments.m_vsync = false;
	arguments.m_enable_editor = true;
	arguments.m_name = "Influx Game";
	arguments.m_window_clear_colour = math::float4{ 0.2f, 0.2f, 0.2f, 1.0f };
	arguments.m_staged = false;

	arguments.m_window_width = 1280u;
	arguments.m_window_height = 720u;

	application::run(arguments);
}