-- influx assets
new_influx_library("influx_async")

    pchheader "async_pch.h"
    pchsource ("source/async_pch.cpp")

    local dependencies =
    {
        "influx_core"
    }

    set_influx_includes(dependencies)
    set_influx_links(dependencies)
