-- influx imgui
new_influx_library("influx_imgui")

    pchheader "imgui_pch.h"
    pchsource "source/imgui_pch.cpp"

    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_graphics",
        "influx_shader",

        "thirdparty/imgui"
    }

    set_influx_includes(dependencies)
    set_influx_links(dependencies)

    local tp_source = 
    {
        "thirdparty/imgui"
    }
    add_thirdparty_source(tp_source)

    filter "files:**/imgui/**.cpp"
        flags {"NoPCH"}
