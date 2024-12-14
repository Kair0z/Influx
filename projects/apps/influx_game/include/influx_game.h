#pragma once

#if _DLL
#define INFLUX_GAME_API __declspec(dllexport)
#else
#define INFLUX_GAME_API __declspec(dllimport)
#endif

class INFLUX_GAME_API game final
{
public:
	static void on_start();
	static void on_end();
};