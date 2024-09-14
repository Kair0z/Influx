project "influx_rendergraph"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    g_project_dir = g_dir_projects_engine .. "/%{prj.name}/"

    targetdir(g_dir_binaries .. "/%{prj.name}")
    objdir(g_dir_int .. "/%{prj.name}")

    files
    {
        g_project_dir .. "**.h",
        g_project_dir .. "**.hpp",
        g_project_dir .. "**.cpp",
        g_project_dir .. "**.lua"
    }

    pchheader "rendergraph_pch.h"
    pchsource "source/rendergraph_pch.cpp"

    defines
    {
        
    }

    includedirs
    {
        "source",
        "include",
        g_dir_core_include,
        g_dir_graphics_include
    }

    links
    {
        "influx_graphics"
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
