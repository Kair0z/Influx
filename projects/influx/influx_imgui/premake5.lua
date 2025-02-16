-- influx imgui
new_influx_library("influx_imgui")

    pchheader "imgui_pch.h"
    pchsource "source/imgui_pch.cpp"

    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_graphics",
        "influx_shader"
    }

    set_influx_includes(dependencies)
    set_influx_links(dependencies)

    filter "files:**/imgui/**.cpp"
        flags {"NoPCH"}
