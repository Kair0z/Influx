#include "pch.h"
#include "Window.h"

#ifdef PLATFORM_WINDOWS

namespace Influx
{
	Ptr<Window> Window::Create(const CreateDesc& desc)
	{
		return Create(desc, NULL, NULL);
	}

	Ptr<Window> Window::Create(const CreateDesc& desc, HINSTANCE winInstance, WNDPROC appWindowsProc)
	{
		Ptr<Window> newWindow = new Window();

		newWindow->mHandle = WindowsPlatform::CreateWindow(desc.Width, desc.Height, ToWString(desc.name).c_str(), appWindowsProc, winInstance);
		newWindow->mWidth = desc.Width;
		newWindow->mHeight = desc.Height;
		newWindow->mName = desc.name;

		return newWindow;
	}

	HWND Window::GetWindowsHandle() const
	{
		return mHandle;
	}

	int Window::GetWidth() const
	{
		return mWidth;
	}

	int Window::GetHeight() const
	{
		return mHeight;
	}

	const String& Window::GetName() const
	{
		return mName;
	}

	bool Window::IsFullScreen() const
	{
		return mIsFullScreen;
	}

	void Window::SetFullScreen(bool fullscreenEnable)
	{
		mIsFullScreen = fullscreenEnable;
	}

	void Window::Show()
	{
		::ShowWindow(mHandle, SW_RESTORE);
	}

	void Window::Hide()
	{
		::ShowWindow(mHandle, SW_HIDE);
	}

	void Window::OnWindowResize(int newWidth, int newHeight)
	{

	}
}

#endif

