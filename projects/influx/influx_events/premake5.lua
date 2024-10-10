-- influx events
new_influx_dll("influx_events")

    pchheader "events_pch.h"
    pchsource ("source/events_pch.cpp")

    local dependencies =
    {
        "influx_core"
    }
    set_influx_includes(dependencies)
    set_influx_links(dependencies)

    disablewarnings 
    {
        "4244",
        "4267"
    }