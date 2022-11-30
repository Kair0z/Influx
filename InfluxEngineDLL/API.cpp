
#ifndef FLX_ENGINE_API
#define FLX_ENGINE_API extern "C" __declspec(dllexport)
#endif

#include "Engine/Runtime/Engine/Engine.h"

using namespace Influx;

namespace
{
	struct transform_component
	{
		float test;
	};
}

FLX_ENGINE_API int Test()
{
	return 1;
}

FLX_ENGINE_API int CreateTransformComponent_DLL()
{
	return 2;
}