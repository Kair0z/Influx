new_influx_app("influx_mod")

    local dependencies =
    {
        "influx_core",
        "influx_engine",

        -- non-mono-engine dependencies
        "influx_platform",
        "influx_input",
        "influx_async",
        "influx_import",
        "influx_renderer",
        "influx_shader",
        "influx_graphics",
        "influx_script"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "off"