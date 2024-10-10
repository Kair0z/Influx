new_influx_app("influx_pacman")
    staticruntime "on"
    local dependencies =
    {
        "influx_core", 
        "influx_application", 
        "influx_input", 
        "influx_events",
        "influx_renderer",
        "influx_async",
        "influx_import",
        "influx_graphics",
        "influx_shader"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "on"
    

    