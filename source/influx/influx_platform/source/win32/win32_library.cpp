#include "win32_library.h"

// influx::core
#include "core/container/vector.h"
#include "core/string.h"

// Include Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h> // GET_X_LPARAM(), GET_Y_LPARAM()

::HINSTANCE m_instance;
::HMODULE m_module;

namespace influx::platform
{
	library* library::load(const string& path)
	{
		return new win32_library(path);
	}

	void library::free(library* lib)
	{
		delete lib;
		lib = nullptr;
	}

	win32_library::win32_library(const string& path)
	{
		wstring wpath = to_wstring(path);
		m_instance = ::LoadLibrary(wpath.c_str());
		if (!m_instance)
		{
			// error!
		}

		// enumerate functions
		vector<string> functions{};
		m_module = LoadLibraryEx(wpath.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);
		if (m_module)
		{
			// find the export directory
			auto dosHeader = (PIMAGE_DOS_HEADER)m_module;
			auto ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)m_module + dosHeader->e_lfanew);
			auto exportDirectoryRVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
			if (exportDirectoryRVA != 0)
			{
				auto exportDirectory = (PIMAGE_EXPORT_DIRECTORY)((BYTE*)m_module + exportDirectoryRVA);
				auto functionNamesRVA = (DWORD*)((BYTE*)m_module + exportDirectory->AddressOfNames);

				for (size_t i = 0; i < exportDirectory->NumberOfNames; ++i)
				{
					auto functionName = (char*)((BYTE*)m_module + functionNamesRVA[i]);
					functions.emplace_back(functionName);
				}
			}
		}
		m_functions = functions;
	}

	void* win32_library::get_func_address(const string& func_name)
	{
		auto contains = [&func_name](const string& str) -> bool
		{
			return (str.find(func_name) != std::string::npos);
		};
		auto found = std::find_if(m_functions.cbegin(), m_functions.cend(), contains);

		if (found != m_functions.cend())
		{
			::FARPROC func_address = GetProcAddress(m_instance, found->c_str());
			if (func_address)
			{
				return func_address;
			}
		}

		return nullptr;
	}

	void win32_library::call(const string& func_name)
	{
		void* fn_address = get_func_address(func_name);
		if (fn_address)
		{
			typedef void(__stdcall* function)();
			((function)fn_address)();
		}
	}

	win32_library::~win32_library()
	{
		::FreeLibrary(m_instance);
		::FreeLibrary(m_module);
	}
}
