project "influx_rendy"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    g_project_dir = g_dir_projects_apps .. "/%{prj.name}/"
    g_target_dir = g_dir_binaries .. "/%{prj.name}"
    
    targetdir(g_target_dir)
    debugdir(g_target_dir)
    objdir(g_dir_int .. "/%{prj.name}")

    fastuptodate (false)

    files
    {
        g_project_dir .. "**.h",
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
        g_dir_graphics_include,
        g_dir_rendergraph_include
    }

    links
    {
        "influx_rendergraph"
    }

    dependencies = "influx_rendergraph influx_graphics"

    postbuildmessage "Copying dependencies..."
    postbuildcommands
    {
        {"cd " .. g_dir_root .. "/scripts/"},
        {
            "python.exe stage.py " 
                .. " --config=" .. g_config_string 
                .. " --game=" .. "influx_rendy"
                .. " --deps " .. dependencies
        }
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
