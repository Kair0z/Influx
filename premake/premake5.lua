workspace "influx"
    architecture "x64"
    configurations {"debug", "release", "profile"}
    flags {"MultiProcessorCompile"}
    language "C++"
    startproject "influx_game"
    location "../generated/%{_ACTION}/"
    
    -- /influx/
    g_dir_root = "%{wks.location}/../../"
    g_config_string = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    -- /influx/projects/...
    g_dir_projects = g_dir_root .. "/projects/"
    g_dir_projects_apps = g_dir_projects .. "/apps/"
    g_dir_projects_engine = g_dir_projects .. "/influx/"
    g_dir_projects_misc = g_dir_projects .. "/misc/"
    g_dir_binaries = g_dir_root .. "/bin/" .. g_config_string .. "/"
    g_dir_int = g_dir_root .. "/int/" .. g_config_string .. "/"
    g_dir_scripts = g_dir_root .. "/scripts/"
    g_dir_resources = g_dir_root .. "/resources/"

    -- /influx/projects/influx/<projectname>/include/...
    g_dir_core_include = g_dir_projects_engine .. "/influx_core/include/"
    g_dir_platform_include = g_dir_projects_engine .. "/influx_platform/include/"
    g_dir_app_include = g_dir_projects_engine .. "/influx_application/include/"
    g_dir_input_include = g_dir_projects_engine .. "/influx_input/include/"
    g_dir_async_include = g_dir_projects_engine .. "/influx_async/include/"
    g_dir_assets_include = g_dir_projects_engine .. "/influx_assets/include/"
    g_dir_render_include = g_dir_projects_engine .. "/influx_renderer/include/"
    g_dir_graphics_include = g_dir_projects_engine .. "/influx_graphics/include/"
    g_dir_shader_include = g_dir_projects_engine .. "/influx_shader/include/"
    g_dir_imgui_include = g_dir_projects_engine .. "/influx_imgui/include/"
    g_dir_events_include = g_dir_projects_engine .. "/influx_events/include/"
    g_dir_vulkan_include = g_dir_projects_engine .. "/influx_graphics/foreign/vulkan/"
    g_dir_rendergraph_include = g_dir_projects_engine .. "/influx_rendergraph/include/"
    
    g_dir_vendor_libraries = g_dir_root .. "/vendor/lib/x64/"
    libdirs{ g_dir_vendor_libraries .. "%{cfg.buildcfg}" }
    
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
    
    -- projects
    printf(".. influx engine")
    group "influx engine"
    include "../projects/influx/influx_application"
    include "../projects/influx/influx_events"
    include "../projects/influx/influx_input"
    include "../projects/influx/influx_async"
    include "../projects/influx/influx_core"
    include "../projects/influx/influx_platform"
    include "../projects/influx/influx_renderer"
    include "../projects/influx/influx_rendergraph"
    include "../projects/influx/influx_imgui"
    include "../projects/influx/influx_graphics"
    include "../projects/influx/influx_assets"
    include "../projects/influx/influx_shader"
    include "../projects/influx/shaders"
    group ""

    printf(".. apps ")
    group "apps"
    include "../projects/apps/influx_game"
    include "../projects/apps/influx_rendy"
    group ""

    printf(".. misc ")
    group "misc"
    include "../projects/misc/flux_renderer"
    include "../projects/misc/flux_raytracing"
    group ""