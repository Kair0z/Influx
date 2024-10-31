-- influx platform
new_influx_library("influx_platform")
    local dependencies =
    {
        "influx_core"
    }
    set_influx_includes(dependencies)
    set_influx_links(dependencies)