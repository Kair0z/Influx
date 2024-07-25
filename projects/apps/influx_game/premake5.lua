project "influx_game"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    g_project_dir = g_dir_projects_apps .. "/influx_game/"
    g_target_dir = g_dir_binaries .. "/%{prj.name}"
    
    targetdir(g_target_dir)
    debugdir(g_target_dir)
    objdir(g_dir_int .. "/%{prj.name}")

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
        g_dir_app_include,
        g_dir_core_include
    }

    links
    {
        "influx_application"
    }

    postbuildmessage "Copying dependencies..."
    postbuildcommands
    {
        --{"{COPYFILE} " .. g_dir_root .. "/vendor/bin/x64/debug/dxil.dll %{cfg.buildtarget.directory}"},
        --{"{COPYFILE} " .. g_dir_root .. "/vendor/bin/x64/debug/dxcompiler.dll %{cfg.buildtarget.directory}"},

        {"cd " .. g_dir_root .. "/scripts/"},
        {
            "python.exe stage.py " 
                .. " --config=" .. g_config_string 
                .. " --game=" .. "influx_game"
                .. " --deps " .. "influx_application influx_renderer influx_async influx_assets influx_input influx_events"
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
        postbuildcommands
        {
            {"{COPYFILE} " .. g_dir_root .. "vendor/bin/x64/Debug/assimp-vc142-mtd.dll %{cfg.buildtarget.directory}"}
        }
    
    filter "configurations:release"
        defines "INFLUX_RELEASE"
        runtime "Release"
        optimize "on"
        postbuildcommands
        {
            {"{COPYFILE} " .. g_dir_root .. "vendor/bin/x64/Release/assimp-vc142-mt.dll %{cfg.buildtarget.directory}"}
        }
