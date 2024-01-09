project "influx_core"
    kind "None"
    language "C++"
    cppdialect "C++20"

    g_project_dir = g_dir_projects .. "/influx_core/"
    
    files
    {
        g_project_dir .. "**.h",
        g_project_dir .. "**.lua"
    }

    defines
    {
        
    }

    includedirs
    {
        "include"
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
