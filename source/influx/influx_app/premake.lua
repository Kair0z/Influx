-- influx app
new_influx_dll("influx_app")
    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_graphics"
    }
    set_influx_includes(dependencies)
    set_influx_links(dependencies)