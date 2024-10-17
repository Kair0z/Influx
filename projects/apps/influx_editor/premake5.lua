new_influx_app("influx_editor")

    local dependencies =
    {
        "influx_core",
        "influx_platform",
        "influx_engine",
        "influx_renderer",
        "influx_graphics",
        "influx_import",
        "influx_input",
        "influx_async"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"