-- influx assets
new_influx_dll("influx_assets")

    pchheader "assets_pch.h"
    pchsource ("source/assets_pch.cpp")

    local dependencies =
    {
        "influx_core"
    }

    set_influx_includes(dependencies)
    set_influx_links(dependencies)
