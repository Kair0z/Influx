#pragma once

#if _DLL
#define INFLUX_ENGINE_API __declspec(dllexport)
#else
#define INFLUX_ENGINE_API __declspec(dllimport)
#endif

#include "core/result.h"
#include "core/plugin.h"

namespace influx::engine
{
	template <typename _t = char>
	using result = result<_t, const char*>;

	INFLUX_ENGINE_API result<> run_editor(int argc = 0, char* argv[] = nullptr);

	INFLUX_ENGINE_API result<> run_game(int argc = 0, char* argv[] = nullptr);

	class INFLUX_ENGINE_API plugin final : plugin_interface
	{
	public:
		virtual void load(const plugin_load_args& args)
		{
			run_game();
		}

		virtual void unload() {}
	};
}
