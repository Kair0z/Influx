project "influx_graphics"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    g_project_dir = g_dir_projects_engine .. "/%{prj.name}/"
    g_compile_vulkan = false
    g_compile_d3d12 = true

    targetdir(g_dir_binaries .. "/%{prj.name}")
    objdir(g_dir_int .. "/%{prj.name}")

    files
    {
        g_project_dir .. "**.h",
        g_project_dir .. "**.cpp",
        g_project_dir .. "**.lua",
        g_project_dir .. "**.hpp"
    }

    removefiles
    {
        iif(g_compile_vulkan ~= true, "**/vulkan/**", ""),
        iif(g_compile_d3d12 ~= true, "**/d3d12/**", "")
    }

    pchheader "graphics_pch.h"
    pchsource "source/graphics_pch.cpp"

    defines
    {
        iif(g_compile_vulkan, "INFLUX_VULKAN=1", "INFLUX_VULKAN=0"),
        iif(g_compile_d3d12, "INFLUX_DX12=1", "INFLUX_DX12=0")
    }

    includedirs
    {
        "source",
        "include",
        "foreign",
        "foreign/vulkan/",
        g_dir_core_include
    }

    links
    {
        "d3d12", "dxgi",
        --"vulkan-1"
    }

    filter "system:windows"
        systemversion "latest"
        defines
        {
            "INFLUX_PLATFORM_WINDOWS"
        }

    filter "configurations:debug"
        defines "INFLUX_DEBUG"
        runtime "Debug"
        symbols "on"
    
    filter "configurations:release"
        defines "INFLUX_RELEASE"
        runtime "Release"
        optimize "on"

    filter "configurations:profile"
        defines "INFLUX_PROFILE"
        runtime "Release"
        symbols "on"
        optimize "on"
