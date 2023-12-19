project "influx_application"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    g_project_dir = g_dir_projects .. "/influx_application/"

    targetdir(g_dir_binaries .. "/%{prj.name}")
    objdir(g_dir_int .. "/%{prj.name}")

    files
    {
        g_project_dir .. "include/influx_application.h",
        g_project_dir .. "source/application/**.h",
        g_project_dir .. "source/application/**.cpp",
    }

    defines
    {
        
    }

    includedirs
    {
        "source",
        "include",
        g_dir_core_include,
        g_dir_async_include,
        g_dir_render_include
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
