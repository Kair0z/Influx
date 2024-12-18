#pragma once
#include <iostream>

#if _DLL
#define INFLUX_GAME_API __declspec(dllexport)
#else
#define INFLUX_GAME_API __declspec(dllimport)
#endif

class INFLUX_GAME_API game final
{
public:
	static void start();
	static void tick();
	static void end();
};