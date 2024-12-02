new_influx_game("influx_mod")

    local dependencies =
    {
        "influx_core",
        "influx_engine",
        "influx_platform",
        "influx_input",
        "influx_async",
        "influx_import",
        "influx_renderer",
        "influx_shader",
        "influx_graphics",
        "influx_script"
    }

    set_influx_includes(dependencies)
    set_influx_links(dependencies)