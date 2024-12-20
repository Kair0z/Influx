function new_influx_project(_name, _kind)
    project(_name)
        kind(_kind)
        language("C++")
        cppdialect(g_common_cpp_dialect)
       
        targetdir(g_dir_binaries .. "/%{prj.name}/")
        objdir(g_dir_int .. "/%{prj.name}/")
        files
        {
            project_dir .. "**.h",
            project_dir .. "**.cpp",
            project_dir .. "**.hpp",
            project_dir .. "**.lua"
        }

        -- common includes for each project
        includedirs
        {
            "source",
            "include",
            "vendor",

            -- global third party
            iif(g_use_pix ~= true, "", g_dir_vendor .. "/include/pix/")
        }

        

        -- common defines for each project
        defines
        {
            iif(g_use_pix ~= true, "INFLUX_USE_WINPIX=0", "INFLUX_USE_WINPIX=1")
        }

        filter "system:windows"
            if g_use_pix then links("WinPixEventRuntime") end
            common_windows_config()

        filter "configurations:debug"
            common_debug_config()
    
        filter "configurations:release"
           common_release_config()

        filter "configurations:profile"
           common_profile_config()

        -- ending all filters
        filter {}
end

function new_influx_app(name)
    project_dir = g_dir_projects_apps .. "/%{prj.name}/"
    new_influx_project(name, "ConsoleApp")
        fastuptodate(false)
end

function new_influx_game(name)
    project_dir = g_dir_projects_apps .. "/%{prj.name}/"
    new_influx_project(name, "SharedLib")
end

function new_influx_dll(name)
    project_dir = g_dir_projects_engine .. "/%{prj.name}/"
    new_influx_project(name, "SharedLib")
end

function new_influx_statlib(name)
    project_dir = g_dir_projects_engine .. "/%{prj.name}/"
    new_influx_project(name, "StaticLib")
end

function new_influx_library(name)
    if g_compile_mono_engine then
        new_influx_statlib(name)
    else
        new_influx_dll(name)
    end
end

-- helpers
function set_influx_app_dependencies(...)

    local copy_strings = ...
    if g_compile_mono_engine then
        -- mono engine only makes 2 dependencies
        copy_strings = {"influx_core", "influx_engine"}
    end
    
    -- add include dependency includes
    set_influx_includes(copy_strings)

    -- add static lib links
    set_influx_links(copy_strings)

    -- make a string listing all dependencies (except core)
    local copylist = table.concat(copy_strings, " ")
    copylist = string.gsub(copylist, "influx_core", "")

    -- copy dependencies script setup
    postbuildmessage "Copying dependencies..."
    postbuildcommands
    {
        {"cd " .. g_dir_root .. "/scripts/"},
        {
            "python.exe stage.py " 
                .. " --config=" .. g_config_string 
                .. " --game=" .. "%{prj.name}"
                .. " --deps " .. copylist
        }
    }
end

function set_influx_includes(...)
    for i, str in ipairs(...) do
        includedirs( g_dir_projects_engine .. "/" .. str .. "/include/" )
    end
end

function set_influx_links(...)
    for i, str in ipairs(...) do
        if str ~= "influx_core" then
            links(str)
        end
    end
end

function common_debug_config()
    defines "INFLUX_DEBUG"
    runtime "Debug"
    optimize "off"
    symbols "on"
end

function common_release_config()
    defines "INFLUX_RELEASE"
    runtime "Release"
    optimize "on"
    symbols "off"
end

function common_profile_config()
    defines "INFLUX_PROFILE"
    runtime "Release"
    optimize "on"
    symbols "off"
end

function common_windows_config()
    systemversion "latest"
    defines "INFLUX_PLATFORM_WINDOWS"
end