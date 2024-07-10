project "influx_assets"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    g_project_dir = g_dir_projects_engine .. "/influx_assets/"
    g_source_dir = g_project_dir .. "/source/"

    targetdir(g_dir_binaries .. "/%{prj.name}")
    objdir(g_dir_int .. "/%{prj.name}")

    files
    {
        g_project_dir .. "**.h",
        g_project_dir .. "**.cpp",
        g_project_dir .. "**.lua"
    }

    pchheader "assets_pch.h"
    pchsource ("source/assets_pch.cpp")

    defines
    {
        
    }

    includedirs
    {
        "source",
        "include",
        "vendor",
        g_dir_core_include,
        g_dir_root .. "/vendor/include/assimp/"
    }

    links
    {
        
    }

    filter "files:**/lodepng/**.cpp"
        flags {"NoPCH"}

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
