new_influx_app("influx_game")
    staticruntime "on"
    local dependencies =
    {
        "influx_core", 
        "influx_application", 
        "influx_input", 
        "influx_events",
        "influx_renderer",
        "influx_async",
        "influx_assets",
        "influx_graphics",
        "influx_shader"
    }
    set_influx_app_dependencies(dependencies)
    staticruntime "on"
    

    