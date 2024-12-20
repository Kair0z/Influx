workspace "influx"
    architecture "x64"
    configurations {"debug", "release", "profile"}
    flags {"MultiProcessorCompile"}
    language "C++"
    startproject "influx_game"
    location "../generated/%{_ACTION}/"
    
    g_compile_mono_engine = false
    g_use_pix = true;
    
    -- /influx/
    g_dir_root      = "%{wks.location}/../../"
    g_config_string = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    -- /influx/projects/...
    g_dir_projects      = g_dir_root .. "/projects/"
    g_dir_projects_apps = g_dir_projects .. "/apps/"
    g_dir_projects_engine = g_dir_projects .. "/influx/"
    g_dir_projects_misc = g_dir_projects .. "/misc/"
    g_dir_binaries      = g_dir_root .. "/bin/" .. g_config_string .. "/"
    g_dir_int           = g_dir_root .. "/int/" .. g_config_string .. "/"
    g_dir_scripts       = g_dir_root .. "/scripts/"
    g_dir_resources     = g_dir_root .. "/resources/"
    g_dir_assets        = g_dir_root .. "/assets/"
    g_dir_vendor        = g_dir_root .. "/vendor/"

    -- /influx/projects/influx/<projectname>/include/...
    g_dir_core_include      = g_dir_projects_engine .. "/influx_core/include/"
    g_dir_platform_include  = g_dir_projects_engine .. "/influx_platform/include/"
    g_dir_app_include       = g_dir_projects_engine .. "/influx_application/include/"
    g_dir_input_include     = g_dir_projects_engine .. "/influx_input/include/"
    g_dir_async_include     = g_dir_projects_engine .. "/influx_async/include/"
    g_dir_assets_include    = g_dir_projects_engine .. "/influx_assets/include/"
    g_dir_render_include    = g_dir_projects_engine .. "/influx_renderer/include/"
    g_dir_graphics_include  = g_dir_projects_engine .. "/influx_graphics/include/"
    g_dir_rhi_include       = g_dir_projects_engine .. "/influx_rhi/include/"
    g_dir_shader_include    = g_dir_projects_engine .. "/influx_shader/include/"
    g_dir_imgui_include     = g_dir_projects_engine .. "/influx_imgui/include/"
    g_dir_vulkan_include    = g_dir_projects_engine .. "/influx_graphics/foreign/vulkan/"
    g_dir_rendergraph_include = g_dir_projects_engine .. "/influx_rendergraph/include/"
    
    -- vendor libraries
    g_dir_vendor_libraries = g_dir_vendor .. "/lib/x64/"
    libdirs{ g_dir_vendor_libraries .. "%{cfg.buildcfg}" }
    
    -- common
    g_common_cpp_dialect = "C++20"
    
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

    include "premake_common.lua"
    
    -- projects
    printf(".. influx engine")
    group "influx engine"
    for _, dir in ipairs(os.matchdirs("../projects/influx/" .. "/*")) do
        include(dir)
    end
    group ""

    printf(".. apps ")
    group "apps"
    for _, dir in ipairs(os.matchdirs("../projects/apps/" .. "/*")) do
        include(dir)
    end
    group ""

    printf(".. misc ")
    group "misc"
    for _, dir in ipairs(os.matchdirs("../projects/misc/" .. "/*")) do
        include(dir)
    end
    group ""