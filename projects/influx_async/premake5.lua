project "influx_async"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    g_project_dir = g_dir_projects .. "/influx_async/"

    targetdir(g_dir_binaries .. "/%{prj.name}")
    objdir(g_dir_int .. "/%{prj.name}")

    files
    {
        g_project_dir .. "include/influx_async.h",
        g_project_dir .. "source/async/**.h",
        g_project_dir .. "source/async/**.cpp",
    }

    pchheader "async_pch.h"
    pchsource "async_pch.cpp"

    defines
    {
        
    }

    includedirs
    {
        "source",
        "include",
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
