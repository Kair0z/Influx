-- influx renderer
new_influx_library("influx_renderer")

    pchheader "renderer_pch.h"
    pchsource "source/renderer_pch.cpp"

    use_renderjobs = false
    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_graphics",
        "influx_rhi",
        "influx_rendergraph",
        "influx_imgui",
        "influx_shader"
    }

    if use_renderjobs then
        dependencies {
            "influx_async"
        }
    end
    set_influx_includes(dependencies)
    set_influx_links(dependencies)

    local tp_sources = 
    {
        "thirdparty/imgui",
        "thirdparty/D3DX12"
    }
    add_thirdparty_source(tp_sources)

    includedirs
    {
        g_dir_shaders_engine,
        "/shaders/"
    }

    defines
    {
        iif(g_userenderjobs, "WITH_RENDERJOBS=1", "WITH_RENDERJOBS=0")
    }

    -- precompile + embed shaders
    dir_shaders = project_dir .. "/shaders/"
    dir_shaders_precompile_output_dir = dir_shaders .. "/compiled/"
    prebuildmessage "prebuild: Embedding Renderer Shaders..."
    prebuildcommands
    {
        ---- compile each shader
        {"cd " .. g_dir_root .. "/scripts/"},
        {
            "python.exe compile_shaders_renderer.py"
        }
    }

    filter "files:**/imgui/**.cpp"
        flags {"NoPCH"}
