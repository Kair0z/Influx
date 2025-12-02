-- influx renderer
new_influx_library("influx_renderer")

    pchheader "renderer_pch.h"
    pchsource "source/renderer_pch.cpp"

    g_userenderjobs = true
    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_graphics",
        "influx_rhi",
        "influx_rendergraph",
        "influx_imgui",
        "influx_shader",
        iif(g_userenderjobs ~= true, "", "influx_async")
    }
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
