project "influx_graphics"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    g_project_dir = g_dir_projects .. "/%{prj.name}/"

    targetdir(g_dir_binaries .. "/%{prj.name}")
    objdir(g_dir_int .. "/%{prj.name}")

    files
    {
        g_project_dir .. "**.h",
        g_project_dir .. "**.cpp",
        g_project_dir .. "**.lua",
        g_project_dir .. "**.hpp"
    }

    pchheader "graphics_pch.h"
    pchsource "source/graphics_pch.cpp"

    defines
    {
        
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
