#pragma once

#ifdef PLATFORM_WINDOWS

namespace Influx
{
	/* Base Window Class */
	/* [FOR NOW, WINDOWS ONLY...]*/
	class Window
	{
	public:
		struct CreateDesc
		{
			int Width{};
			int Height{};
			String name{};
		};
		static Ptr<Window> Create(const CreateDesc& desc);
		static Ptr<Window> Create(const CreateDesc& desc, HINSTANCE winInstance, WNDPROC appWindowsProc);

		HWND GetWindowsHandle() const;

		int GetWidth() const;
		int GetHeight() const;
		const String& GetName() const;

		bool IsFullScreen() const;
		void SetFullScreen(bool fullscreenEnable);

		void Show();
		void Hide();

	private:
		Window() = default;

		HWND mHandle{};
		int mWidth{};
		int mHeight{};
		String mName{};

		bool mIsFullScreen = false;

		RECT mWindowRect;

	protected:
		virtual void OnWindowResize(int newWidth, int newHeight);
	};
}
#endif


