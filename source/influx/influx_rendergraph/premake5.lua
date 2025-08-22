-- influx rendergraph
new_influx_library("influx_rendergraph")

    pchheader "rendergraph_pch.h"
    pchsource "source/rendergraph_pch.cpp"

    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_graphics",
        "influx_rhi"
    }
    set_influx_includes(dependencies)
    set_influx_links(dependencies)