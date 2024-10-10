-- influx renderer
new_influx_dll("influx_renderer")

    pchheader "renderer_pch.h"
    pchsource "source/renderer_pch.cpp"

    local dependencies =
    {
        "influx_core",
        "influx_graphics",
        "influx_imgui",
        "influx_shader"
    }
    set_influx_includes(dependencies)
    set_influx_links(dependencies)

    includedirs
    {
        "vendor/imgui/",
    }

    filter "files:**/imgui/**.cpp"
        flags {"NoPCH"}
