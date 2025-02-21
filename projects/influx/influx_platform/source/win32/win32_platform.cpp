#include "win32_platform.h"

// Include Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <commdlg.h>

namespace influx::platform
{
	instance_handle platform::get_current_instance()
	{
		return ::GetModuleHandleW(NULL);
	}

	string platform::get_current_directory()
	{
		wchar_t buff[MAX_PATH];
		::GetCurrentDirectory(MAX_PATH, buff);

		return to_string(buff);
	}

	void platform::set_current_directory(const string& path)
	{
		::SetCurrentDirectory(to_wstring(path).c_str());
	}

	math::vectoru2 platform::get_cursor_screenpos()
	{
		POINT mouse_screen_pos;
		bool has_mouse_screen_pos = ::GetCursorPos(&mouse_screen_pos) != 0;

		return math::vectoru2(mouse_screen_pos.x, mouse_screen_pos.y);
	}

	window_handle platform::find_window_from_cursor()
	{
		POINT mouse_screen_pos;
		bool has_mouse_screen_pos = ::GetCursorPos(&mouse_screen_pos) != 0;
		if (has_mouse_screen_pos)
		{
			return (window_handle)::WindowFromPoint(mouse_screen_pos);
		}
		
		return nullptr;
	}

	file_dialog_result platform::open_file_dialog(const string& path)
	{
		file_dialog_result result{};
		
		::OPENFILENAME ofn;       // Common dialog box structure
		wchar_t filePath[MAX_PATH] = L""; // Buffer to store the selected file path

		// Initialize OPENFILENAME structure
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;  // No parent window
		ofn.lpstrFilter = L"FBX Files\0*.fbx\0All Files\0*.*\0"; // Filter for .fbx files
		ofn.lpstrFile = filePath; // File path buffer
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST; // Ensure the file exists
		ofn.lpstrDefExt = L"fbx"; // Default extension

		// Open the file dialog
		if (GetOpenFileName(&ofn)) 
		{
			result.m_has_selected = true;
			result.m_selection = to_string(filePath);
		}
		else 
		{
			result.m_has_selected = false;
		}

		return result;
	}
}