#pragma once
#if PLATFORM_WINDOWS
#include "Core/Container/List.h"
#include "Core/Memory/Reference.h"
#include "Core/Platform/Windows/WindowsPlatform.h"
#include "Core/Function/Function.h"

#include "Runtime/Window/Window.h"
#include "Runtime/Window/WindowEvents.h"

#include "Core/Singleton/Locator.h"

namespace Influx
{
	class WindowsApp final
	{
		typedef LRESULT(*WindowProcHandler)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	public:
		static Ptr<WindowsApp> Create(const HINSTANCE instance = NULL, const HICON icon = NULL);

		Ptr<Window> GetWindow() const;

		void PollEvents();

		static void AddWindowEventProcHandler(WindowProcHandler handler);

	private:
		WindowsApp() = default;

		/* Constructs the Windows-Window */
		Ptr<Window> MakeWindow(const Window::CreateDesc& desc, const bool show = true);

		HINSTANCE mInstanceHandle;
		Ptr<Window> mpWindow;

		/* Main Window Event Proc Function */
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT mssg, WPARAM wParam, LPARAM lParam);
		static Vector<WindowProcHandler> StatExternalWindowProcHandlers;
	};

	using ApplicationLocator = Locator<WindowsApp>;
}
#endif
