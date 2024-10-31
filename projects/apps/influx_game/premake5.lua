new_influx_app("influx_game")

    local dependencies =
    {
        "influx_core",
        "influx_engine"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"