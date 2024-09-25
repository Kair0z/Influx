project "influx_platform"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    g_project_dir = g_dir_projects_engine .. "/%{prj.name}/"
    
    files
    {
        g_project_dir .. "**.h",
        g_project_dir .. "**.cpp",
        g_project_dir .. "**.lua"
    }

    includedirs
    {
        "include",
        "source",
        g_dir_core_include
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
