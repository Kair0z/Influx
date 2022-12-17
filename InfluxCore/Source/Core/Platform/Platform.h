#pragma once

#ifndef _CORE_PLATFORM_H_
#define _CORE_PLATFORM_H_

#include "Core/BasicTypes.h"
#include "Core/Geometry/Rect.h"
#include "Core/String.h"
#include "Core/Function.h"

namespace Influx::Platform
{
	// Memory 
	template <typename _T>
	_T* Allocate(const uint64 numBytes);

	template <typename _T>
	void Free(_T* address);

	// Process & Window
	using ProcessHandle = void*;
	using InstanceHandle = void*;
	using WindowHandle = void*;

	ProcessHandle GetCurrentProcess();
	InstanceHandle GetCurrentInstance();
	WindowHandle GetCurrentWindowHandle();
	void QuitCurrentInstance();

	namespace Window
	{
		struct Settings final
		{
			int w;
			int h;
			Influx::String Name;
		};

		WindowHandle Create(const Settings& settings);
		void Destroy(WindowHandle = GetCurrentWindowHandle());
		WindowHandle GetCurrentHandle() { return GetCurrentWindowHandle(); }

		template <typename _T>
		Math::Rect<_T> GetFullRect(WindowHandle = GetCurrentWindowHandle());
		template <typename _T>
		Math::Rect<_T> GetClientRect(WindowHandle = GetCurrentWindowHandle());

		bool IsVisible(WindowHandle = GetCurrentWindowHandle());
	}
	
	namespace Console
	{
		enum class EColourAttribute : uint16
		{
			Green = 2,
			Red   = 4,
			Purple = 5,
			BG_Green = 12,
			BG_Red = 14,
			BG_Purple = 15,
			Max
		};

		template <EColourAttribute _A>
		void SetColourAttribute();
	}
}

#if PLATFORM_WINDOWS
#include "WindowsPlatform.h"
#else
#include "NullPlatform.h"
#endif

#endif