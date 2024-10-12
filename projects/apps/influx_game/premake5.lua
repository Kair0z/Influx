new_influx_app("influx_game")

    local dependencies =
    {
        "influx_core",
        "influx_engine",
        "influx_platform"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"