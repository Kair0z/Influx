new_influx_module("influx_mod")
    local dependencies =
    {
        "influx_core",
        "influx_engine",
        "influx_input",
    }

    set_influx_includes(dependencies)
    set_influx_links(dependencies)