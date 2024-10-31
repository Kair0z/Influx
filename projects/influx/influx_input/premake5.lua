-- influx input
new_influx_library("influx_input")

    pchheader "input_pch.h"
    pchsource ("source/input_pch.cpp")

    local dependencies =
    {
        "influx_core",
        "influx_platform",
    }
    set_influx_includes(dependencies)
    set_influx_links(dependencies)