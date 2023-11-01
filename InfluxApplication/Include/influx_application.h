
// define api
#if _DLL
	#define INFLUX_APP_API __declspec(dllexport)
#else
	#define INFLUX_APP_API __declspec(dllimport)
#endif

#define INFLUX_APP_USES_WINDOWS 1

#include "Core/basetypes.h"
#include "Core/String.h"

namespace influx::application
{
	struct run_args final
	{
		run_args(int argc = 0, char** argv = nullptr)
		{
			// ...
		}

		const char* m_name = "";
		string m_resources_dir = "";

		bool m_commandlet = false;
		bool m_enable_scenerender = false;
		bool m_enable_editor = false;
		bool m_vsync = false;
		uint8 m_max_thread_frame_difference = 3u;

		uint32 m_window_width = 640u;
		uint32 m_window_height = 480u;
	};

	void INFLUX_APP_API run(const run_args& args);

	inline void run(int argc = 0, char** argv = nullptr)
	{
		run(run_args{ argc, argv });
	}

	void INFLUX_APP_API quit();
}