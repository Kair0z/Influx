#pragma once

#include "vulkan.hpp"

#if INFLUX_PLATFORM_WINDOWS
#include <Windows.h>
#include <windef.h>
#include "vulkan/vulkan/vulkan_win32.h" // VK_KHR_win32_surface
#endif