new_influx_app("influx_editor")

    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_engine"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"