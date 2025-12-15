-- influx renderer
new_influx_library("influx_renderer")

    pchheader "renderer_pch.h"
    pchsource "source/renderer_pch.cpp"

    g_userenderjobs = false
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

    if g_userenderjobs then
        dependencies {
            "influx_async"
        }
    end

    set_influx_includes(dependencies)
    set_influx_links(dependencies)

    includedirs
    {
        g_dir_shaders_engine,
        "/shaders/"
    }

    defines
    {
        iif(g_userenderjobs, "WITH_RENDERJOBS=1", "WITH_RENDERJOBS=0")
    }

    filter "files:**/imgui/**.cpp"
        flags {"NoPCH"}

    -- precompile + embed shaders
    --dir_shaders = project_dir .. "/shaders/"
    --dir_shaders_precompile_output_dir = dir_shaders .. "/compiled/"
    --prebuildmessage "Embedding Shaders..."
    --prebuildcommands
    --{
    --    -- compile each shader
    --    {"cd " .. g_dir_root .. "/scripts/"},
    --    {
    --        "python.exe compile_shaders.py "
    --            --.. " --config=" .. g_config_string 
    --            --.. " --game=" .. "%{prj.name}"
    --            --.. " --deps " .. copylist
    --    }
    --
    --    -- convert each shader.cso -> shader.h
    --    -- renderer will include these headers
    --    {
    --
    --    }
    --}
