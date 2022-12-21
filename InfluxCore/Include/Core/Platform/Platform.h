#pragma once

#ifndef _CORE_PLATFORM_H_
#define _CORE_PLATFORM_H_

#include "Core/BasicTypes.h"
#include "Core/Math/Vector.h"
#include "Core/Geometry/Rect.h"
#include "Core/String.h"
#include "Core/Function.h"

namespace Influx::Platform
{
	using ProcessHandle = void*;
	using InstanceHandle = void*;
	using WindowHandle = void*;

	namespace Window
	{
		struct Settings final
		{
			Settings() = default;
			Settings(const Math::Vectori2& dimensions, const Influx::String& name) : Width{ dimensions.x }, Heigth{	dimensions.y }, Name{name} {}

			int Width;
			int Heigth;
			Influx::String Name;
		};

		enum class Event
		{
			Activate,
			Quit,
			Max,
			Unknown = Max
		};
	}

	namespace Console
	{
		enum class EColourAttribute : uint16
		{
			Green = 2,
			Red = 4,
			Purple = 5,
			BG_Green = 12,
			BG_Red = 14,
			BG_Purple = 15,
			Max
		};
	}
}

#if PLATFORM_WINDOWS
#include "WindowsPlatform.h"
#else
static_assert(false, "[Influx Core Platform ERROR] Non-Supported Platform defined! (Supported Platforms: PLATFORM_WINDOWS)");
#include "NullPlatform.h"
#endif
#endif