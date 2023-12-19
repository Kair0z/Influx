
workspace "influx"
    architecture "x64"
    configurations {"debug", "release", "profile"}
    flags {"MultiProcessorCompile"}
    language "C++"
    startproject "sandbox"
    location "../generated"
    
    g_dir_root = "../"
    g_dir_projects = g_dir_root .. "projects/"
    g_dir_binaries = g_dir_root .. "bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    g_dir_int = g_dir_root .. "int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    g_dir_core_include = g_dir_root .. "/influx_core/include/"
    g_dir_async_include = g_dir_root .. "/influx_async/include/"
    g_dir_render_include = g_dir_root .. "/influx_render/include/"
    -- projects
    group "applications"
    include "../projects/sandbox"
    include "../projects/influx_game"
    group ""

    group "libraries"
    include "../projects/influx_application"
    include "../projects/influx_async"
    include "../projects/influx_core"
    include "../projects/influx_renderer"
    group ""

    group "misc"
    include "../projects/flux_renderer"
    include "../projects/flux_raytracing"
    group ""