project "influx_vendor"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    g_project_dir = g_dir_projects .. "/%{prj.name}/"
    
    files
    {
        g_project_dir .. "**.h",
        g_project_dir .. "**.cpp"
    }

    defines
    {
        
    }

    includedirs
    {
        "include/vendor/ImGui/",
        "source/vendor/ImGui/"
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
