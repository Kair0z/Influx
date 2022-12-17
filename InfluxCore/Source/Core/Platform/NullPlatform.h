#pragma once

#ifndef _CORE_PLATFORM_NULL_H_
#define _CORE_PLATFORM_NULL_H_

#include "Platform.h"

namespace Influx::Platform
{
	// Memory 
	template <typename _T>
	_T* Allocate(const uint64)
	{
		// ...
		return nullptr;
	}

	template <typename _T>
	void Free(_T*)
	{
		// ...
	}

	// Process & Window
	ProcessHandle GetCurrentProcess()
	{
		// ...
		return nullptr;
	}

	InstanceHandle GetCurrentInstance()
	{
		// ...
		return nullptr;
	}

	WindowHandle GetCurrentWindowHandle()
	{
		// ...
		return nullptr;
	}

	void QuitCurrentInstance()
	{
		// ...
	}

	namespace Window
	{
		WindowHandle Create(const Settings&)
		{
			// ...
			return nullptr;
		}

		void Destroy(WindowHandle )
		{
			// ...
		}

		template <typename _T>
		Math::Rect<_T> GetFullRect(WindowHandle)
		{
			// ...
			return {};
		}

		template <typename _T>
		Math::Rect<_T> GetClientRect(WindowHandle)
		{
			// ...
			return {};
		}

		bool IsVisible(WindowHandle)
		{
			// ...
			return false;
		}
	}

	namespace Console
	{
		template <EColourAttribute _A>
		void SetColourAttribute()
		{
			// ...
		}
	}
}

#endif