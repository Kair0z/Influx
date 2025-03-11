#pragma once

#if _DLL
#define INFLUX_SHATOY_API __declspec(dllexport)
#else
#define INFLUX_SHATOY_API __declspec(dllimport)
#endif


namespace influx::shadertoy
{

}