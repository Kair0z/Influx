workspace "influx"
    architecture "x64"
    configurations {"debug", "release", "profile"}
    flags {"MultiProcessorCompile"}
    language "C++"
    startproject "sandbox"
    location "../generated"
    
    g_dir_root = "%{wks.location}/../"

    g_dir_projects = g_dir_root .. "/projects/"
    g_dir_binaries = g_dir_root .. "/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/"
    g_dir_int = g_dir_root .. "/int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/"

    g_dir_core_include = g_dir_projects .. "/influx_core/include/"
    g_dir_app_include = g_dir_projects .. "/influx_application/include/"
    g_dir_async_include = g_dir_projects .. "/influx_async/include/"
    g_dir_render_include = g_dir_projects .. "/influx_renderer/include/"
    g_dir_vendor_include = g_dir_projects .. "/influx_vendor/include/vendor/" -- special include to avoid having to specify vendor

    g_dir_vendor_libraries = g_dir_root .. "/vendor/lib/x64/"
    libdirs{ g_dir_vendor_libraries .. "%{cfg.buildcfg}" }

    g_dir_resources = g_dir_root .. "/resources/"

    -- projects
    printf(".. libraries")
    group "libraries"
    include "../projects/influx_application"
    include "../projects/influx_async"
    include "../projects/influx_core"
    include "../projects/influx_renderer"
    include "../projects/influx_vendor"
    group ""

    printf(".. applications")
    group "applications"
    include "../projects/sandbox"
    include "../projects/influx_game"
    group ""

    printf(".. miscelaneous projects")
    group "misc"
    include "../projects/flux_renderer"
    include "../projects/flux_raytracing"
    group ""