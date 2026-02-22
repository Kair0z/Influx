-- influx renderer
new_influx_library("influx_renderer")
    pchheader "renderer_pch.h"
    pchsource "source/renderer_pch.cpp"

    -- dependencies
    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_graphics",
        "influx_rhi",
        "influx_rendergraph",
        "influx_shader",

        "thirdparty/sol"
    }
    use_renderjobs = false
    if use_renderjobs then
        dependencies { "influx_async" }
    end
    add_compile_dependencies(dependencies)

    -- TP sources
    local tp_sources = 
    {
        "thirdparty/imgui",
        "thirdparty/D3DX12",
        "thirdparty/lua"
    }
    add_thirdparty_source(tp_sources)

    -- shader frontend includes
    includedirs
    {
        g_dir_shaders_engine,
        "/shaders/"
    }

    -- DEFINES
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

    -- TP sources no PCH
    filter "files:**/imgui/**.cpp"
        flags {"NoPCH"}
    filter "files:**/lua/**.c"
        flags {"NoPCH"}