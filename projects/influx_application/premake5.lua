project "influx_application"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    g_project_dir = g_dir_projects .. "/%{prj.name}/"

    targetdir(g_dir_binaries .. "/%{prj.name}")
    objdir(g_dir_int .. "/%{prj.name}")

    files
    {
        g_project_dir .. "**.h",
        g_project_dir .. "**.cpp"
    }

    pchheader "app_pch.h"
    pchsource "app_pch.cpp"

    defines
    {
        
    }

    includedirs
    {
        "source",
        "include",
        g_dir_core_include,
        g_dir_async_include,
        g_dir_render_include,
        g_dir_vendor_include
    }

    links
    {
        "influx_renderer",
        "influx_async",
        "influx_vendor"
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
        links { "assimp-vc142-mtd.lib" }
    
    filter "configurations:release"
        defines "INFLUX_RELEASE"
        runtime "Release"
        optimize "on"
        links {"assimp-vc142-mt.lib"}

    filter "configurations:profile"
        defines "INFLUX_PROFILE"
        runtime "Release"
        symbols "on"
        optimize "on"
        links {"assimp-vc142-mt.lib"}
