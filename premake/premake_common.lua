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
            "vendor"
        }

        -- common defines for each project
        defines
        {
            iif(g_use_pix ~= true, "INFLUX_USE_WINPIX=0", "INFLUX_USE_WINPIX=1")
        }

        filter "system:windows"
            if g_use_pix then 
                set_influx_includes("thirdparty/WinPixEventRuntime")
                set_influx_links("thirdparty/WinPixEventRuntime")
            end
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

    local dependency_list = ...
    if g_compile_mono_engine then
        -- mono engine only results in 2 dependencies
        dependency_list = {"influx_core", "influx_engine"}
    end
    
    -- add include dependency includes
    set_influx_includes(dependency_list)

    -- add static lib links
    set_influx_links(dependency_list)

    -- make a string listing all dependencies (except core)
    local copylist = table.concat(dependency_list, " ")
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
   
    local influx_base = path.getabsolute("../../..")
    local influx_source = influx_base .. "/source/"
    local thirdparty_prefix = "thirdparty/"
    local thirdparty_includes = influx_base .. "/thirdparty/include/"

    -- first do the non-third-party include files in /source/influx/...
    for i, dep in ipairs(...) do
        local is_third_party = string.find(dep, thirdparty_prefix)
        local found_incdir = influx_source .. "/influx/" .. dep .. "/include/"
        if not is_third_party and os.isdir(found_incdir) then
            -- print(found_incdir)
            includedirs(found_incdir)
        end
    end

    -- then do the "thirdparty/" ones in /thirdparty/include/
    for i, dep in ipairs(...) do
        local prefix_start, prefix_end = string.find(dep, thirdparty_prefix)
        if prefix_end then
            dep = string.sub(dep, prefix_end + 1)
            local found_incdir = thirdparty_includes .. dep .. "/"
            -- print(found_incdir)
            if os.isdir(found_incdir) then
                includedirs(found_incdir)
            end
        end
    end
end

function set_influx_links(...)
    local influx_base = path.getabsolute("../../..")
    local thirdparty_prefix = "thirdparty/"
    local thirdparty_libs = influx_base .. "/thirdparty/lib/"
    local config_path = "/x64/debug/"

    -- non-third party ones (uses link())
    for i, str in ipairs(...) do
        local is_third_party = string.find(str, thirdparty_prefix)
        if not is_third_party and str ~= "influx_core" then
            links(str)
        end
    end

    -- third-party ones
    for i, str in ipairs(...) do
        local prefix_start, prefix_end = string.find(str, thirdparty_prefix)
        if prefix_end then
            -- found the prefix, so it's a thirdparty lib
            str = string.sub(str, prefix_end + 1)
            local thirdparty_libs = thirdparty_libs .. config_path
            if os.isdir(thirdparty_libs) then
                libdirs(thirdparty_libs)
                -- links(str)
            end
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