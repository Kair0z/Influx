
// define api
#if _DLL
	#define INFLUX_APP_API __declspec(dllexport)
#else
	#define INFLUX_APP_API __declspec(dllimport)
#endif

#define INFLUX_APP_USES_WINDOWS 1

#include <cstdint>

namespace influx::application
{
	struct INFLUX_APP_API run_args final
	{
		run_args(int argc = 0, char** argv = nullptr)
		{
			// ...
		}

		const char* m_name = "";

		bool m_commandlet = false;
		bool m_enable_scenerender = false;
		bool m_enable_editor = false;

		uint32_t m_window_width = 640u;
		uint32_t m_window_height = 480u;
	};

	void INFLUX_APP_API run(const run_args& args);

	void run(int argc = 0, char** argv = nullptr)
	{
		run(run_args{ argc, argv });
	}

	void INFLUX_APP_API quit();
}