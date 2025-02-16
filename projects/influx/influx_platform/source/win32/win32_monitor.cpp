#include "win32_monitor.h"

// windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace influx::platform
{
    typedef enum { MDT_EFFECTIVE_DPI = 0, MDT_ANGULAR_DPI = 1, MDT_RAW_DPI = 2, MDT_DEFAULT = MDT_EFFECTIVE_DPI } MONITOR_DPI_TYPE;
    typedef HRESULT(WINAPI* PFN_GetDpiForMonitor)(HMONITOR, MONITOR_DPI_TYPE, UINT*, UINT*);        // Shcore.lib + dll, Windows 8.1+

    static BOOL _IsWindowsVersionOrGreater(WORD major, WORD minor, WORD)
    {
        typedef LONG(WINAPI* PFN_RtlVerifyVersionInfo)(OSVERSIONINFOEXW*, ULONG, ULONGLONG);
        static PFN_RtlVerifyVersionInfo RtlVerifyVersionInfoFn = nullptr;
        if (RtlVerifyVersionInfoFn == nullptr)
            if (HMODULE ntdllModule = ::GetModuleHandleA("ntdll.dll"))
                RtlVerifyVersionInfoFn = (PFN_RtlVerifyVersionInfo)GetProcAddress(ntdllModule, "RtlVerifyVersionInfo");
        if (RtlVerifyVersionInfoFn == nullptr)
            return FALSE;

        RTL_OSVERSIONINFOEXW versionInfo = { };
        ULONGLONG conditionMask = 0;
        versionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
        versionInfo.dwMajorVersion = major;
        versionInfo.dwMinorVersion = minor;
        VER_SET_CONDITION(conditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
        VER_SET_CONDITION(conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
        return (RtlVerifyVersionInfoFn(&versionInfo, VER_MAJORVERSION | VER_MINORVERSION, conditionMask) == 0) ? TRUE : FALSE;
    }
#define _IsWindows8Point1OrGreater() _IsWindowsVersionOrGreater(HIBYTE(0x0603), LOBYTE(0x0603), 0) // _WIN32_WINNT_WINBLUE

    inline static float get_dpi(HMONITOR monitor)
    {
        UINT xdpi = 96, ydpi = 96;
        if (_IsWindows8Point1OrGreater())
        {
            static HINSTANCE shcore_dll = ::LoadLibraryA("shcore.dll"); // Reference counted per-process
            static PFN_GetDpiForMonitor GetDpiForMonitorFn = nullptr;
            if (GetDpiForMonitorFn == nullptr && shcore_dll != nullptr)
                GetDpiForMonitorFn = (PFN_GetDpiForMonitor)::GetProcAddress(shcore_dll, "GetDpiForMonitor");
            if (GetDpiForMonitorFn != nullptr)
            {
                GetDpiForMonitorFn((HMONITOR)monitor, MDT_EFFECTIVE_DPI, &xdpi, &ydpi);
                influx_assert(xdpi == ydpi); // Please contact me if you hit this assert!
                return xdpi / 96.0f;
            }
        }
#ifndef NOGDI
        const HDC dc = ::GetDC(nullptr);
        xdpi = ::GetDeviceCaps(dc, LOGPIXELSX);
        ydpi = ::GetDeviceCaps(dc, LOGPIXELSY);
        influx_assert(xdpi == ydpi); // Please contact me if you hit this assert!
        ::ReleaseDC(nullptr, dc);
#endif
        return xdpi / 96.0f;
    }

	vector<monitor> monitor::query_monitors()
	{
		static vector<monitor> result{};
        result.clear();

        ::EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR mon, HDC, LPRECT, LPARAM) -> BOOL
        {
            MONITORINFO info = {};
            info.cbSize = sizeof(MONITORINFO);
            if (!::GetMonitorInfo(mon, &info))
            {
                return TRUE;
            }

            monitor new_monitor{};
            new_monitor.m_mainpos   = math::vectorf2((float)info.rcMonitor.left, (float)info.rcMonitor.top);
            new_monitor.m_mainsize  = math::vectoru2((uint32)(info.rcMonitor.right - info.rcMonitor.left), (uint32)(info.rcMonitor.bottom - info.rcMonitor.top));
            new_monitor.m_workpos   = math::vectorf2((float)info.rcWork.left, (float)info.rcWork.top);
            new_monitor.m_worksize  = math::vectoru2((uint32)(info.rcWork.right - info.rcWork.left), (uint32)(info.rcWork.bottom - info.rcWork.top));
            new_monitor.m_dpi_scale = get_dpi(mon);
            new_monitor.m_is_primary = info.dwFlags & MONITORINFOF_PRIMARY;
            new_monitor.m_platform_handle = (void*)mon;

            if (new_monitor.m_dpi_scale <= 0.0f)
                return TRUE; // Some accessibility applications are declaring virtual monitors with a DPI of 0, see #7902.
            
            result.push_back(new_monitor);

            return TRUE;
        }, 0);

		return result;
	}
}
