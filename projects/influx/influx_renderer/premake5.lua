-- influx renderer
new_influx_library("influx_renderer")

    pchheader "renderer_pch.h"
    pchsource "source/renderer_pch.cpp"

    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_graphics",
        "influx_rendergraph",
        "influx_imgui",
        "influx_shader"
    }
    set_influx_includes(dependencies)
    set_influx_links(dependencies)

    includedirs
    {
        "vendor/imgui/",
        g_dir_shaders_engine
    }

    filter "files:**/imgui/**.cpp"
        flags {"NoPCH"}
