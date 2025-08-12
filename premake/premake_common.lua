function new_influx_project(_name, _kind, _language)
    project(_name)
        kind(_kind)
        language(_language)
        cppdialect(g_cpp_dialect)
        targetdir(g_dir_binaries .. "/%{prj.name}/")
        objdir(g_dir_int .. "/%{prj.name}/")
        files
        {
            project_dir .. "**.h",
            project_dir .. "**.cpp",
            project_dir .. "**.hpp",
            project_dir .. "**.lua",
            project_dir .. "**.hlsli",
            project_dir .. "**.flx"
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

function new_influx_cpp_project(_name, _kind)
    new_influx_project(_name, _kind, "C++")
end

function new_influx_cs_project(_name, _kind)
    new_influx_project(_name, _kind, "C#")
end

-- declares an 'app' (console) project into /source/apps/...
function new_influx_app(name)
    project_dir = g_dir_source_apps .. "/%{prj.name}/"
    new_influx_project(name, "ConsoleApp")
    fastuptodate(false)
end

-- declares a 'game' (library) project into /source/apps/...
function new_influx_game(name)
    project_dir = g_dir_source_apps .. "/%{prj.name}/"
    new_influx_project(name, "SharedLib")
end

-- declares a 'test' project into /source/test/...
function new_influx_test(name)
    project_dir = g_dir_source_test .. "/%{prj.name}/"
    new_influx_project(name, "ConsoleApp")
    fastuptodate(false)
end

function new_influx_misc(name)
    project_dir = g_dir_source_misc .. "/%{prj.name}/"
    new_influx_project(name, "ConsoleApp")
    fastuptodate(false)
end

-- declares a third party header only library as a project
function new_thirdparty_headeronly(name)
    project_dir = g_dir_source_thirdparty .. "/%{prj.name}/"
    new_influx_project(name, "None")
end

-- declares a 'tool' project into /source/tools/...
function new_influx_tool(name)
    project_dir = g_dir_source_tools .. "/%{prj.name}/"
    new_influx_project(name, "ConsoleApp")
    fastuptodate(false)
end

-- declares a 'dll' project into /source/influx/...
function new_influx_dll(name)
    project_dir = g_dir_source_engine .. "/%{prj.name}/"
    new_influx_project(name, "SharedLib")
end

-- declares a 'lib' project into /source/influx/...
function new_influx_statlib(name)
    project_dir = g_dir_source_engine .. "/%{prj.name}/"
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

    local base = path.getabsolute("../..")

    -- check in /source/influx/...
    for i, dep in ipairs(...) do
        local incdir = base .. "/influx/" .. dep .. "/include/"
        if os.isdir(incdir) then
            -- print(incdir)
            includedirs(incdir)
        end
    end
    -- check in /source/thirdparty/...
    for i, dep in ipairs(...) do
        local incdir = base .. "/thirdparty/" .. dep .. "/include/"
        if os.isdir(incdir) then
            print(incdir)
            includedirs(incdir)
        end
    end
end

function set_influx_links(...)
    for i, str in ipairs(...) do
        if str ~= "influx_core" then
            links(str)
        end
    end
end

function common_cpp_config()
    cppdialect "C++17"
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

function common_linux_config()
    toolset "gcc"   -- Use GCC (default on Arch)
end