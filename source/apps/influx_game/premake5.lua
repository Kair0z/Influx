new_influx_game("influx_game")

    local dependencies =
    {
        "influx_core",
        "influx_script",
        "influx_engine"
    }

    set_influx_includes(dependencies)
    set_influx_links(dependencies)