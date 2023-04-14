#pragma once

#ifndef __CORE_PLATFORM_H_
#define __CORE_PLATFORM_H_

#include "Core/BasicTypes.h"
#include "Core/Math/Vector.h"
#include "Core/Geometry/Rect.h"
#include "Core/String.h"
#include "Core/Function.h"
#include "Core/Singleton/Singleton.h"

#ifdef CreateWindow
#undef CreateWindow
#endif

// [TYPES]
namespace Influx::Platform
{
	using ProcessHandle = void*;
	using InstanceHandle = void*;
	using WindowHandle = void*;

	// [IO]
	typedef void(*WindowEventCallback)();
	typedef void(*WindowEvent_MousePos)(const float x, const float y);
	typedef void(*WindowEvent_MouseButton)(int button, bool isDown);
	typedef void(*WindowEvent_MouseWheel)(const float w_x, const float w_y);
	typedef void(*WindowEvent_Focus)(bool isFocussed);

	struct WindowSettings final
	{
		WindowSettings() = default;
		WindowSettings(const Math::Vectori2& dimensions, const Influx::String& name) 
			: Width{ dimensions.x }, Heigth{ dimensions.y }, Name{ name } {}

		int Width;
		int Heigth;
		
		Influx::String Name;
	};

	enum class WindowEvent
	{
		Activate,
		Quit,
		Max,
		Unknown = Max
	};

	enum class EMessageBoxType : uint8
	{
		Info,
		Warning,
		Error,
		Max
	};
	
	enum class EConsoleColour : uint16
	{
		Green = 2,
		Red = 4,
		Purple = 5,
		BG_Green = 12,
		BG_Red = 14,
		BG_Purple = 15,
		Max
	};

	enum class EWindowVisibility : uint8
	{
		Minimize,
		ShowDefault,
		Maximize
	};
}

// [INTERFACE DECLARATIONS]
namespace Influx::Platform
{
	// [MEMORY]
	void* Allocate(const uint64 size);

	/* Allocates memory for an object of sizeof(_T) */
	template <typename _T>
	_T* Allocate();
	
	/* Allocates memory for an object of _T and calls _T() */
	template <typename _T, typename ..._Args>
	_T* New(_Args&&... args);

	/* Frees memory for an object of sizeof(_T) */
	template <typename _T>
	void Free(_T* address);


	// [APPLICATION]
	ProcessHandle GetCurrentProcess();

	InstanceHandle GetCurrentInstance();

	WindowHandle GetCurrentWindowHandle();

	void QuitCurrentInstance();


	// [WINDOW]
	WindowHandle CreateWindow(const WindowSettings& settings, bool shouldOpen = true);

	WindowSettings GetWindowSettings(const WindowHandle handle);

	void DestroyWindow(const WindowHandle handle);

	/* Returns true if window is as a result visible */
	bool SetWindowVisibility(const WindowHandle windowHandle, const EWindowVisibility command);

	template <typename _T>
	Math::Rect<_T> GetFullWindowRect(const WindowHandle windowHandle);

	template <typename _T>
	Math::Rect<_T> GetClientWindowRect(const WindowHandle windowHandle);

	bool IsWindowVisible(const WindowHandle windowHandle);

	// [IO]


	// [MISC]
	template <EMessageBoxType _T>
	void MessageBox(const String& caption, const String& message, const WindowHandle windowHandle);

	inline void ErrorMessageBox(const String& caption, const String& message, const WindowHandle windowHandle = GetCurrentWindowHandle())
	{
		MessageBox<EMessageBoxType::Error>(caption, message, windowHandle);
	}

	inline void InfoMessageBox(const String& caption, const String& message, const WindowHandle windowHandle = GetCurrentWindowHandle())
	{
		MessageBox<EMessageBoxType::Info>(caption, message, windowHandle);
	}

	inline void WarningMessageBox(const String& caption, const String& message, const WindowHandle windowHandle = GetCurrentWindowHandle())
	{
		MessageBox<EMessageBoxType::Warning>(caption, message, windowHandle);
	}

	template <EConsoleColour _C>
	void SetConsoleColourAttribute();
}

#endif // _CORE_PLATFORM_H_