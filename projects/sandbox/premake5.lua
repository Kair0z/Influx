project "sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    g_project_dir = g_dir_projects .. "/%{prj.name}/"

    targetdir(g_dir_binaries .. "/%{prj.name}")
    objdir(g_dir_int .. "/%{prj.name}")

    files
    {
        g_project_dir .. "**.cpp",
        g_project_dir .. "**.lua"
    }

    defines
    {
        
    }

    includedirs
    {
        "source",
        g_dir_core_include,
        g_dir_render_include
    }

    links
    {
        "influx_renderer"
    }

    postbuildmessage "Copying dependencies..."
    postbuildcommands
    {
        {"{COPYFILE} %{cfg.buildtarget.directory}../influx_renderer/influx_renderer.dll %{cfg.buildtarget.directory}"}
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
